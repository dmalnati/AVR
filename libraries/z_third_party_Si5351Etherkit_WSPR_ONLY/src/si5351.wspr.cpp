/*
 * si5351.cpp - Si5351 library for Arduino
 *
 * Copyright (C) 2015 - 2016 Jason Milldrum <milldrum@gmail.com>
 *                           Dana H. Myers <k6jq@comcast.net>
 *
 * Some tuning algorithms derived from clk-si5351.c in the Linux kernel.
 * Sebastian Hesselbarth <sebastian.hesselbarth@gmail.com>
 * Rabeeh Khoury <rabeeh@solid-run.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "Arduino.h"
#include "ThinWire.h"
#include "si5351.wspr.h"


/********************/
/* Public functions */
/********************/

Si5351::Si5351(uint8_t i2c_addr):
	i2c_bus_addr(i2c_addr)
{
	xtal_freq[0] = SI5351_XTAL_FREQ;

	// Start by using XO ref osc as default for each PLL
	plla_ref_osc = SI5351_PLL_INPUT_XO;
	pllb_ref_osc = SI5351_PLL_INPUT_XO;
	clkin_div = SI5351_CLKIN_DIV_1;
}

/*
 * init(uint8_t xtal_load_c, uint32_t ref_osc_freq, int32_t corr)
 *
 * Setup communications to the Si5351 and set the crystal
 * load capacitance.
 *
 * xtal_load_c - Crystal load capacitance. Use the SI5351_CRYSTAL_LOAD_*PF
 * defines in the header file
 * xo_freq - Crystal/reference oscillator frequency in 1 Hz increments.
 * Defaults to 25000000 if a 0 is used here.
 * corr - Frequency correction constant in parts-per-billion
 *
 * Returns a boolean that indicates whether a device was found on the desired
 * I2C address.
 *
 */
bool Si5351::init(uint8_t xtal_load_c, uint32_t /*xo_freq*/, int32_t corr)
{
	// Start I2C comms
	ThinWire.begin();

	// Check for a device on the bus, bail out if it is not there
	ThinWire.beginTransmission(i2c_bus_addr);
	uint8_t reg_val;
  reg_val = ThinWire.endTransmission();

	if(reg_val == 0)
	{
		// Wait for SYS_INIT flag to be clear, indicating that device is ready
		uint8_t status_reg = 0;
		do
		{
			status_reg = si5351_read(SI5351_DEVICE_STATUS);
		} while (status_reg >> 7 == 1);

		// Set crystal load capacitance
		si5351_write(SI5351_CRYSTAL_LOAD, (xtal_load_c & SI5351_CRYSTAL_LOAD_MASK) | 0b00010010);

		// Set up the XO reference frequency for 25MHz
        set_ref_freq(SI5351_XTAL_FREQ, SI5351_PLL_INPUT_XO);

		// Set the frequency calibration for the XO
		set_correction(corr, SI5351_PLL_INPUT_XO);

		reset();

		return true;
	}
	else
	{
		return false;
	}
}

/*
 * reset(void)
 *
 * Call to reset the Si5351 to the state initialized by the library.
 *
 */
void Si5351::reset(void)
{
	// Initialize the CLK outputs according to flowchart in datasheet
	// First, turn them off
	si5351_write(16, 0x80);

	// Turn the clocks back on...
	si5351_write(16, 0x0c);

	// Set PLLA and PLLB to 800 MHz for automatic tuning
	set_pll(SI5351_PLL_FIXED, SI5351_PLLA);

	// Make PLL to CLK assignments for automatic tuning
	pll_assignment[0] = SI5351_PLLA;

	set_ms_source(SI5351_CLK0, SI5351_PLLA);

	// Reset the VCXO param
	si5351_write(SI5351_VXCO_PARAMETERS_LOW, 0);
	si5351_write(SI5351_VXCO_PARAMETERS_MID, 0);
	si5351_write(SI5351_VXCO_PARAMETERS_HIGH, 0);

	// Then reset the PLLs
	pll_reset(SI5351_PLLA);

	// Set initial frequencies
	uint8_t i;
	for(i = 0; i < 1; i++)
	{
		clk_freq[i] = 0;
		output_enable((enum si5351_clock)i, 0);
		clk_first_set[i] = false;
	}
}

/*
 * set_freq(uint64_t freq, enum si5351_clock clk)
 *
 * Sets the clock frequency of the specified CLK output.
 * Frequency range of 8 kHz to 150 MHz
 *
 * freq - Output frequency in Hz
 * clk - Clock output
 *   (use the si5351_clock enum)
 */
uint8_t Si5351::set_freq(uint64_t freq, enum si5351_clock clk)
{
	struct Si5351RegSet ms_reg;
	uint8_t int_mode = 0;
	uint8_t div_by_4 = 0;
	uint8_t r_div = 0;

		// MS0 through MS5 logic
		// ---------------------


			clk_freq[(uint8_t)clk] = freq;

			// Enable the output on first set_freq only
			if(clk_first_set[(uint8_t)clk] == false)
			{
				output_enable(clk, 1);
				clk_first_set[(uint8_t)clk] = true;
			}

			// Select the proper R div value
			r_div = select_r_div(&freq);

			// Calculate the synth parameters
				multisynth_calc(freq, plla_freq, &ms_reg);

			// Set multisynth registers
			set_ms(clk, ms_reg, int_mode, r_div, div_by_4);

			// Reset the PLL
			//pll_reset(pll_assignment[clk]);

		return 0;
}




/*
 * set_pll(uint64_t pll_freq, enum si5351_pll target_pll)
 *
 * Set the specified PLL to a specific oscillation frequency
 *
 * pll_freq - Desired PLL frequency in Hz * 100
 * target_pll - Which PLL to set
 *     (use the si5351_pll enum)
 */
void Si5351::set_pll(uint64_t pll_freq, enum si5351_pll /*target_pll*/)
{
  struct Si5351RegSet pll_reg;

    pll_calc(SI5351_PLLA, pll_freq, &pll_reg, ref_correction[plla_ref_osc], 0);

  // Derive the register values to write

  // Prepare an array for parameters to be written to
  //uint8_t *params = new uint8_t[20];
  uint8_t params[20];
  uint8_t i = 0;
  uint8_t temp;

  // Registers 26-27
  temp = ((pll_reg.p3 >> 8) & 0xFF);
  params[i++] = temp;

  temp = (uint8_t)(pll_reg.p3  & 0xFF);
  params[i++] = temp;

  // Register 28
  temp = (uint8_t)((pll_reg.p1 >> 16) & 0x03);
  params[i++] = temp;

  // Registers 29-30
  temp = (uint8_t)((pll_reg.p1 >> 8) & 0xFF);
  params[i++] = temp;

  temp = (uint8_t)(pll_reg.p1  & 0xFF);
  params[i++] = temp;

  // Register 31
  temp = (uint8_t)((pll_reg.p3 >> 12) & 0xF0);
  temp += (uint8_t)((pll_reg.p2 >> 16) & 0x0F);
  params[i++] = temp;

  // Registers 32-33
  temp = (uint8_t)((pll_reg.p2 >> 8) & 0xFF);
  params[i++] = temp;

  temp = (uint8_t)(pll_reg.p2  & 0xFF);
  params[i++] = temp;

  // Write the parameters
    si5351_write_bulk(SI5351_PLLA_PARAMETERS, i, params);
		plla_freq = pll_freq;

  //delete params;
}

/*
 * set_ms(enum si5351_clock clk, struct Si5351RegSet ms_reg, uint8_t int_mode, uint8_t r_div, uint8_t div_by_4)
 *
 * Set the specified multisynth parameters. Not normally needed, but public for advanced users.
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * int_mode - Set integer mode
 *  Set to 1 to enable, 0 to disable
 * r_div - Desired r_div ratio
 * div_by_4 - Set Divide By 4 mode
 *   Set to 1 to enable, 0 to disable
 */
void Si5351::set_ms(enum si5351_clock clk, struct Si5351RegSet ms_reg, uint8_t int_mode, uint8_t r_div, uint8_t div_by_4)
{
	//uint8_t *params = new uint8_t[20];
    uint8_t params[20];
	uint8_t i = 0;
 	uint8_t temp;
 	uint8_t reg_val;


		// Registers 42-43 for CLK0
		temp = (uint8_t)((ms_reg.p3 >> 8) & 0xFF);
		params[i++] = temp;

		temp = (uint8_t)(ms_reg.p3  & 0xFF);
		params[i++] = temp;

		// Register 44 for CLK0
		reg_val = si5351_read((SI5351_CLK0_PARAMETERS + 2) + (clk * 8));
		reg_val &= ~(0x03);
		temp = reg_val | ((uint8_t)((ms_reg.p1 >> 16) & 0x03));
		params[i++] = temp;

		// Registers 45-46 for CLK0
		temp = (uint8_t)((ms_reg.p1 >> 8) & 0xFF);
		params[i++] = temp;

		temp = (uint8_t)(ms_reg.p1  & 0xFF);
		params[i++] = temp;

		// Register 47 for CLK0
		temp = (uint8_t)((ms_reg.p3 >> 12) & 0xF0);
		temp += (uint8_t)((ms_reg.p2 >> 16) & 0x0F);
		params[i++] = temp;

		// Registers 48-49 for CLK0
		temp = (uint8_t)((ms_reg.p2 >> 8) & 0xFF);
		params[i++] = temp;

		temp = (uint8_t)(ms_reg.p2  & 0xFF);
		params[i++] = temp;

			si5351_write_bulk(SI5351_CLK0_PARAMETERS, i, params);
			set_int(clk, int_mode);
			ms_div(clk, r_div, div_by_4);

	//delete params;
}

/*
 * output_enable(enum si5351_clock clk, uint8_t enable)
 *
 * Enable or disable a chosen output
 * clk - Clock output
 *   (use the si5351_clock enum)
 * enable - Set to 1 to enable, 0 to disable
 */
void Si5351::output_enable(enum si5351_clock clk, uint8_t enable)
{
  uint8_t reg_val;

  reg_val = si5351_read(SI5351_OUTPUT_ENABLE_CTRL);

  if(enable == 1)
  {
    reg_val &= ~(1<<(uint8_t)clk);
  }
  else
  {
    reg_val |= (1<<(uint8_t)clk);
  }

  si5351_write(SI5351_OUTPUT_ENABLE_CTRL, reg_val);
}

/*
 * drive_strength(enum si5351_clock clk, enum si5351_drive drive)
 *
 * Sets the drive strength of the specified clock output
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * drive - Desired drive level
 *   (use the si5351_drive enum)
 */
void Si5351::drive_strength(enum si5351_clock clk, enum si5351_drive /*drive*/)
{
  uint8_t reg_val;
  const uint8_t mask = 0x03;

  reg_val = si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk);
  reg_val &= ~(mask);


    reg_val |= 0x03;

  si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);
}

/*
 * update_status(void)
 *
 * Call this to update the status structs, then access them
 * via the dev_status and dev_int_status global members.
 *
 * See the header file for the struct definitions. These
 * correspond to the flag names for registers 0 and 1 in
 * the Si5351 datasheet.
 */
void Si5351::update_status(void)
{
	update_sys_status(&dev_status);
	update_int_status(&dev_int_status);
}

/*
 * set_correction(int32_t corr, enum si5351_pll_input ref_osc)
 *
 * corr - Correction factor in ppb
 * ref_osc - Desired reference oscillator
 *     (use the si5351_pll_input enum)
 *
 * Use this to set the oscillator correction factor.
 * This value is a signed 32-bit integer of the
 * parts-per-billion value that the actual oscillation
 * frequency deviates from the specified frequency.
 *
 * The frequency calibration is done as a one-time procedure.
 * Any desired test frequency within the normal range of the
 * Si5351 should be set, then the actual output frequency
 * should be measured as accurately as possible. The
 * difference between the measured and specified frequencies
 * should be calculated in Hertz, then multiplied by 10 in
 * order to get the parts-per-billion value.
 *
 * Since the Si5351 itself has an intrinsic 0 PPM error, this
 * correction factor is good across the entire tuning range of
 * the Si5351. Once this calibration is done accurately, it
 * should not have to be done again for the same Si5351 and
 * crystal.
 */
void Si5351::set_correction(int32_t corr, enum si5351_pll_input ref_osc)
{
	ref_correction[(uint8_t)ref_osc] = corr;

	// Recalculate and set PLL freqs based on correction value
	set_pll(plla_freq, SI5351_PLLA);
}

/*
 * set_phase(enum si5351_clock clk, uint8_t phase)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * phase - 7-bit phase word
 *   (in units of VCO/4 period)
 *
 * Write the 7-bit phase register. This must be used
 * with a user-set PLL frequency so that the user can
 * calculate the proper tuning word based on the PLL period.
 */
void Si5351::set_phase(enum si5351_clock clk, uint8_t phase)
{
	// Mask off the upper bit since it is reserved
	phase = phase & 0b01111111;

	si5351_write(SI5351_CLK0_PHASE_OFFSET + (uint8_t)clk, phase);
}

/*
 * get_correction(enum si5351_pll_input ref_osc)
 *
 * ref_osc - Desired reference oscillator
 *     0: crystal oscillator (XO)
 *     1: external clock input (CLKIN)
 *
 * Returns the oscillator correction factor stored
 * in RAM.
 */
int32_t Si5351::get_correction(enum si5351_pll_input ref_osc)
{
	return ref_correction[(uint8_t)ref_osc];
}

/*
 * pll_reset(enum si5351_pll target_pll)
 *
 * target_pll - Which PLL to reset
 *     (use the si5351_pll enum)
 *
 * Apply a reset to the indicated PLL.
 */
void Si5351::pll_reset(enum si5351_pll /*target_pll*/)
{
    si5351_write(SI5351_PLL_RESET, SI5351_PLL_RESET_A);
}

/*
 * set_ms_source(enum si5351_clock clk, enum si5351_pll pll)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * pll - Which PLL to use as the source
 *     (use the si5351_pll enum)
 *
 * Set the desired PLL source for a multisynth.
 */
void Si5351::set_ms_source(enum si5351_clock clk, enum si5351_pll pll)
{
	uint8_t reg_val;

	reg_val = si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk);

    reg_val &= ~(SI5351_CLK_PLL_SELECT);

	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);

	pll_assignment[(uint8_t)clk] = pll;
}

/*
 * set_int(enum si5351_clock clk, uint8_t int_mode)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * enable - Set to 1 to enable, 0 to disable
 *
 * Set the indicated multisynth into integer mode.
 */
void Si5351::set_int(enum si5351_clock clk, uint8_t enable)
{
	uint8_t reg_val;
	reg_val = si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk);

	if(enable == 1)
	{
		reg_val |= (SI5351_CLK_INTEGER_MODE);
	}
	else
	{
		reg_val &= ~(SI5351_CLK_INTEGER_MODE);
	}

	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);

	// Integer mode indication
	/*
	switch(clk)
	{
	case SI5351_CLK0:
		clk0_int_mode = enable;
		break;
	case SI5351_CLK1:
		clk1_int_mode = enable;
		break;
	case SI5351_CLK2:
		clk2_int_mode = enable;
		break;
	default:
		break;
	}
	*/
}

/*
 * set_clock_pwr(enum si5351_clock clk, uint8_t pwr)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * pwr - Set to 1 to enable, 0 to disable
 *
 * Enable or disable power to a clock output (a power
 * saving feature).
 */
void Si5351::set_clock_pwr(enum si5351_clock clk, uint8_t pwr)
{
	uint8_t reg_val; //, reg;
	reg_val = si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk);

	if(pwr == 1)
	{
		reg_val &= 0b01111111;
	}
	else
	{
		reg_val |= 0b10000000;
	}

	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);
}

/*
 * set_clock_invert(enum si5351_clock clk, uint8_t inv)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * inv - Set to 1 to enable, 0 to disable
 *
 * Enable to invert the clock output waveform.
 */
void Si5351::set_clock_invert(enum si5351_clock clk, uint8_t inv)
{
	uint8_t reg_val;
	reg_val = si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk);

	if(inv == 1)
	{
		reg_val |= (SI5351_CLK_INVERT);
	}
	else
	{
		reg_val &= ~(SI5351_CLK_INVERT);
	}

	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);
}

/*
 * set_clock_source(enum si5351_clock clk, enum si5351_clock_source src)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * src - Which clock source to use for the multisynth
 *   (use the si5351_clock_source enum)
 *
 * Set the clock source for a multisynth (based on the options
 * presented for Registers 16-23 in the Silicon Labs AN619 document).
 * Choices are XTAL, CLKIN, MS0, or the multisynth associated with
 * the clock output.
 */
void Si5351::set_clock_source(enum si5351_clock clk, enum si5351_clock_source src)
{
	uint8_t reg_val;
	reg_val = si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk);

	// Clear the bits first
	reg_val &= ~(SI5351_CLK_INPUT_MASK);

	switch(src)
	{
	case SI5351_CLK_SRC_XTAL:
		reg_val |= (SI5351_CLK_INPUT_XTAL);
		break;
	case SI5351_CLK_SRC_CLKIN:
		reg_val |= (SI5351_CLK_INPUT_CLKIN);
		break;
	case SI5351_CLK_SRC_MS0:
		if(clk == SI5351_CLK0)
		{
			return;
		}

		reg_val |= (SI5351_CLK_INPUT_MULTISYNTH_0_4);
		break;
	case SI5351_CLK_SRC_MS:
		reg_val |= (SI5351_CLK_INPUT_MULTISYNTH_N);
		break;
	default:
		return;
	}

	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);
}

/*
 * set_clock_disable(enum si5351_clock clk, enum si5351_clock_disable dis_state)
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * dis_state - Desired state of the output upon disable
 *   (use the si5351_clock_disable enum)
 *
 * Set the state of the clock output when it is disabled. Per page 27
 * of AN619 (Registers 24 and 25), there are four possible values: low,
 * high, high impedance, and never disabled.
 */
void Si5351::set_clock_disable(enum si5351_clock clk, enum si5351_clock_disable dis_state)
{
	uint8_t reg_val, reg;

	if (clk >= SI5351_CLK0 && clk <= SI5351_CLK3)
	{
		reg = SI5351_CLK3_0_DISABLE_STATE;
	}
	else if(clk >= SI5351_CLK4 && clk <= SI5351_CLK7)
	{
		reg = SI5351_CLK7_4_DISABLE_STATE;
	}

	reg_val = si5351_read(reg);

	if (clk >= SI5351_CLK0 && clk <= SI5351_CLK3)
	{
		reg_val &= ~(0b11 << (clk * 2));
		reg_val |= dis_state << (clk * 2);
	}
	else if(clk >= SI5351_CLK4 && clk <= SI5351_CLK7)
	{
		reg_val &= ~(0b11 << ((clk - 4) * 2));
		reg_val |= dis_state << ((clk - 4) * 2);
	}

	si5351_write(reg, reg_val);
}

/*
 * set_clock_fanout(enum si5351_clock_fanout fanout, uint8_t enable)
 *
 * fanout - Desired clock fanout
 *   (use the si5351_clock_fanout enum)
 * enable - Set to 1 to enable, 0 to disable
 *
 * Use this function to enable or disable the clock fanout options
 * for individual clock outputs. If you intend to output the XO or
 * CLKIN on the clock outputs, enable this first.
 *
 * By default, only the Multisynth fanout is enabled at startup.
 */
void Si5351::set_clock_fanout(enum si5351_clock_fanout fanout, uint8_t enable)
{
	uint8_t reg_val;
	reg_val = si5351_read(SI5351_FANOUT_ENABLE);

	switch(fanout)
	{
	case SI5351_FANOUT_CLKIN:
		if(enable)
		{
			reg_val |= SI5351_CLKIN_ENABLE;
		}
		else
		{
			reg_val &= ~(SI5351_CLKIN_ENABLE);
		}
		break;
	case SI5351_FANOUT_XO:
		if(enable)
		{
			reg_val |= SI5351_XTAL_ENABLE;
		}
		else
		{
			reg_val &= ~(SI5351_XTAL_ENABLE);
		}
		break;
	case SI5351_FANOUT_MS:
		if(enable)
		{
			reg_val |= SI5351_MULTISYNTH_ENABLE;
		}
		else
		{
			reg_val &= ~(SI5351_MULTISYNTH_ENABLE);
		}
		break;
	}

	si5351_write(SI5351_FANOUT_ENABLE, reg_val);
}

/*
 * set_pll_input(enum si5351_pll pll, enum si5351_pll_input input)
 *
 * pll - Which PLL to use as the source
 *     (use the si5351_pll enum)
 * input - Which reference oscillator to use as PLL input
 *     (use the si5351_pll_input enum)
 *
 * Set the desired reference oscillator source for the given PLL.
 */
void Si5351::set_pll_input(enum si5351_pll pll, enum si5351_pll_input input)
{
	uint8_t reg_val;
	reg_val = si5351_read(SI5351_PLL_INPUT_SOURCE);

	// Clear the bits first
	//reg_val &= ~(SI5351_CLKIN_DIV_MASK);

	switch(pll)
	{
	case SI5351_PLLA:
		if(input == SI5351_PLL_INPUT_CLKIN)
		{
			reg_val |= SI5351_PLLA_SOURCE;
			reg_val |= clkin_div;
			plla_ref_osc = SI5351_PLL_INPUT_CLKIN;
		}
		else
		{
			reg_val &= ~(SI5351_PLLA_SOURCE);
			plla_ref_osc = SI5351_PLL_INPUT_XO;
		}
		break;
	case SI5351_PLLB:
		if(input == SI5351_PLL_INPUT_CLKIN)
		{
			reg_val |= SI5351_PLLB_SOURCE;
			reg_val |= clkin_div;
			pllb_ref_osc = SI5351_PLL_INPUT_CLKIN;
		}
		else
		{
			reg_val &= ~(SI5351_PLLB_SOURCE);
			pllb_ref_osc = SI5351_PLL_INPUT_XO;
		}
		break;
	default:
		return;
	}

	si5351_write(SI5351_PLL_INPUT_SOURCE, reg_val);

	set_pll(plla_freq, SI5351_PLLA);
	set_pll(pllb_freq, SI5351_PLLB);
}



/*
 * set_ref_freq(uint32_t ref_freq, enum si5351_pll_input ref_osc)
 *
 * ref_freq - Reference oscillator frequency in Hz
 * ref_osc - Which reference oscillator frequency to set
 *    (use the si5351_pll_input enum)
 *
 * Set the reference frequency value for the desired reference oscillator
 */
void Si5351::set_ref_freq(uint32_t ref_freq, enum si5351_pll_input ref_osc)
{
    xtal_freq[(uint8_t)ref_osc] = ref_freq;
}

uint8_t Si5351::si5351_write_bulk(uint8_t addr, uint8_t bytes, uint8_t *data)
{
	ThinWire.beginTransmission(i2c_bus_addr);
	ThinWire.write(addr);
	for(int i = 0; i < bytes; i++)
	{
		ThinWire.write(data[i]);
	}
	return ThinWire.endTransmission();

}

uint8_t Si5351::si5351_write(uint8_t addr, uint8_t data)
{
	ThinWire.beginTransmission(i2c_bus_addr);
	ThinWire.write(addr);
	ThinWire.write(data);
	return ThinWire.endTransmission();
}

uint8_t Si5351::si5351_read(uint8_t addr)
{
	uint8_t reg_val = 0;

	ThinWire.beginTransmission(i2c_bus_addr);
	ThinWire.write(addr);
	ThinWire.endTransmission();

	ThinWire.requestFrom(i2c_bus_addr, (uint8_t)1, (uint8_t)false);

	while(ThinWire.available())
	{
		reg_val = ThinWire.read();
	}

	return reg_val;
}

/*********************/
/* Private functions */
/*********************/

uint64_t Si5351::pll_calc(enum si5351_pll /*pll*/, uint64_t freq, struct Si5351RegSet *reg, int32_t correction, uint8_t /*vcxo*/)
{
	uint64_t ref_freq;
    ref_freq = xtal_freq[(uint8_t)plla_ref_osc] * SI5351_FREQ_MULT;
	//ref_freq = 15974400ULL * SI5351_FREQ_MULT;
	uint32_t a, b, c, p1, p2, p3;
	uint64_t lltmp; //, denom;

	// Factor calibration value into nominal crystal frequency
	// Measured in parts-per-billion

	ref_freq = ref_freq + (int32_t)((((((int64_t)correction) << 31) / 1000000000LL) * ref_freq) >> 31);

	// PLL bounds checking

	// Determine integer part of feedback equation
	a = freq / ref_freq;


	// Find best approximation for b/c = fVCO mod fIN
	// denom = 1000ULL * 1000ULL;
	// lltmp = freq % ref_freq;
	// lltmp *= denom;
	// do_div(lltmp, ref_freq);

	//b = (((uint64_t)(freq % ref_freq)) * RFRAC_DENOM) / ref_freq;
    b = (((uint64_t)(freq % ref_freq)) * RFRAC_DENOM) / ref_freq;
    c = b ? RFRAC_DENOM : 1;

	// Calculate parameters
  p1 = 128 * a + ((128 * b) / c) - 512;
  p2 = 128 * b - c * ((128 * b) / c);
  p3 = c;

	// Recalculate frequency as fIN * (a + b/c)
	lltmp = ref_freq;
	lltmp *= b;
	do_div(lltmp, c);
	freq = lltmp;
	freq += ref_freq * a;

	reg->p1 = p1;
	reg->p2 = p2;
	reg->p3 = p3;

    return freq;

}

uint64_t Si5351::multisynth_calc(uint64_t freq, uint64_t pll_freq, struct Si5351RegSet *reg)
{
	uint32_t a, b, c, p1, p2, p3;

	// Multisynth bounds checking

		// Preset PLL, so return the actual freq for these params instead of PLL freq

		// Determine integer part of feedback equation
		a = pll_freq / freq;

		b = (pll_freq % freq * RFRAC_DENOM) / freq;
		c = b ? RFRAC_DENOM : 1;

    p1 = 128 * a + ((128 * b) / c) - 512;
    p2 = 128 * b - c * ((128 * b) / c);
    p3 = c;

	reg->p1 = p1;
	reg->p2 = p2;
	reg->p3 = p3;

		return freq;
}

uint64_t Si5351::multisynth67_calc(uint64_t freq, uint64_t pll_freq, struct Si5351RegSet *reg)
{
	//uint8_t p1;
	// uint8_t ret_val = 0;
	uint32_t a;
	uint64_t lltmp;

	// Multisynth bounds checking
	if(freq > SI5351_MULTISYNTH67_MAX_FREQ * SI5351_FREQ_MULT)
	{
		freq = SI5351_MULTISYNTH67_MAX_FREQ * SI5351_FREQ_MULT;
	}
	if(freq < SI5351_MULTISYNTH_MIN_FREQ * SI5351_FREQ_MULT)
	{
		freq = SI5351_MULTISYNTH_MIN_FREQ * SI5351_FREQ_MULT;
	}

	if(pll_freq == 0)
	{
		// Find largest integer divider for max
		// VCO frequency and given target frequency
		lltmp = (SI5351_PLL_VCO_MAX * SI5351_FREQ_MULT) - 100000000UL; // margin needed?
		do_div(lltmp, freq);
		a = (uint32_t)lltmp;

		// Divisor has to be even
		if(a % 2 != 0)
		{
			a++;
		}

		// Divisor bounds check
		if(a < SI5351_MULTISYNTH_A_MIN)
		{
			a = SI5351_MULTISYNTH_A_MIN;
		}
		if(a > SI5351_MULTISYNTH67_A_MAX)
		{
			a = SI5351_MULTISYNTH67_A_MAX;
		}

		pll_freq = a * freq;

		// PLL bounds checking
		if(pll_freq > (SI5351_PLL_VCO_MAX * SI5351_FREQ_MULT))
		{
			a -= 2;
			pll_freq = a * freq;
		}
		else if(pll_freq < (SI5351_PLL_VCO_MIN * SI5351_FREQ_MULT))
		{
			a += 2;
			pll_freq = a * freq;
		}

		reg->p1 = (uint8_t)a;
		reg->p2 = 0;
		reg->p3 = 0;
		return pll_freq;
	}
	else
	{
		// Multisynth frequency must be integer division of PLL
		if(pll_freq % freq)
		{
			// No good
			return 0;
		}
		else
		{
			a = pll_freq / freq;

			// Division ratio bounds check
			if(a < SI5351_MULTISYNTH_A_MIN || a > SI5351_MULTISYNTH67_A_MAX)
			{
				// No bueno
				return 0;
			}
			else
			{
				reg->p1 = (uint8_t)a;
				reg->p2 = 0;
				reg->p3 = 0;
				return 1;
			}
		}
	}
}

void Si5351::update_sys_status(struct Si5351Status *status)
{
  uint8_t reg_val = 0;

  reg_val = si5351_read(SI5351_DEVICE_STATUS);

  // Parse the register
  status->SYS_INIT = (reg_val >> 7) & 0x01;
  status->LOL_B = (reg_val >> 6) & 0x01;
  status->LOL_A = (reg_val >> 5) & 0x01;
  status->LOS = (reg_val >> 4) & 0x01;
  status->REVID = reg_val & 0x03;
}

void Si5351::update_int_status(struct Si5351IntStatus *int_status)
{
  uint8_t reg_val = 0;

  reg_val = si5351_read(SI5351_INTERRUPT_STATUS);

  // Parse the register
  int_status->SYS_INIT_STKY = (reg_val >> 7) & 0x01;
  int_status->LOL_B_STKY = (reg_val >> 6) & 0x01;
  int_status->LOL_A_STKY = (reg_val >> 5) & 0x01;
  int_status->LOS_STKY = (reg_val >> 4) & 0x01;
}

void Si5351::ms_div(enum si5351_clock /*clk*/, uint8_t r_div, uint8_t /*div_by_4*/)
{
	uint8_t reg_val = 0;
    uint8_t reg_addr = 0;

			reg_addr = SI5351_CLK0_PARAMETERS + 2;

	reg_val = si5351_read(reg_addr);

		// Clear the relevant bits
		reg_val &= ~(0x7c);

			reg_val &= ~(SI5351_OUTPUT_CLK_DIVBY4);

		reg_val |= (r_div << SI5351_OUTPUT_CLK_DIV_SHIFT);

	si5351_write(reg_addr, reg_val);
}

uint8_t Si5351::select_r_div(uint64_t */*freq*/)
{
	uint8_t r_div = SI5351_OUTPUT_CLK_DIV_1;

	return r_div;
}

uint8_t Si5351::select_r_div_ms67(uint64_t *freq)
{
	uint8_t r_div = SI5351_OUTPUT_CLK_DIV_1;

	// Choose the correct R divider
	if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 2))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_128;
		*freq *= 128ULL;
	}
	else if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 2) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 4))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_64;
		*freq *= 64ULL;
	}
	else if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 4) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 8))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_32;
		*freq *= 32ULL;
	}
	else if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 8) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 16))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_16;
		*freq *= 16ULL;
	}
	else if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 16) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 32))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_8;
		*freq *= 8ULL;
	}
	else if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 32) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 64))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_4;
		*freq *= 4ULL;
	}
	else if((*freq >= SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 64) && (*freq < SI5351_CLKOUT67_MIN_FREQ * SI5351_FREQ_MULT * 128))
	{
		r_div = SI5351_OUTPUT_CLK_DIV_2;
		*freq *= 2ULL;
	}

	return r_div;
}
