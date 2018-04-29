#ifndef __RF_SI4463PRO_H__
#define __RF_SI4463PRO_H__


#include <SPI.h>

#include "PAL.h"
#include "BufferFieldExtractor.h"


// Some notes regarding implementation were taken from here:
// https://github.com/Yveaux/RadioHead/blob/master/RadioHead/RH_RF24.cpp
// and
// https://github.com/tkrahn/pecanpico4/blob/master/radio_si446x.cpp
//
// Such as:
// - Waiting briefly to let SS pin go high after transfer
// - Using two transactions for command and reading of response
// - Power-on-Reset functionality being necessary above tying Shutdown to GND
//
// These details are scattered in specs and emails as well.
//


class RFSI4463PRO
{
    // SPI values relating to pure SPI interfacing
    static const uint32_t SPI_SPEED           = 1000000;
    static const uint8_t  SPI_BIT_ORIENTATION = MSBFIRST;
    static const uint8_t  SPI_MODE            = SPI_MODE0;
    
    // SPI values relating to particulars established for this chip
    static const uint16_t READ_ATTEMPT_LIMIT                  = 1000;
    static const uint8_t  DELAY_US_BEFORE_CHIP_SELECT_RELEASE = 50;
    
    // Details relating to operating this chip    
    static const uint32_t DURATION_MS_POWER_ON_RESET = 50;
    static const uint32_t EXTERNAL_CRYSTAL_FREQ      = 30000000;
    
    
public:
    RFSI4463PRO(uint8_t pinChipSelect, uint8_t pinShutdown)
    : pinChipSelect_(pinChipSelect)
    , pinShutdown_(pinShutdown)
    , deviationFreq_(0)
    {
        // Nothing to do
    }
    
    void Init()
    {
        PAL.PinMode(pinShutdown_, OUTPUT);
        PAL.DigitalWrite(pinShutdown_, HIGH);
        
        PAL.PinMode(pinChipSelect_, OUTPUT);
        PAL.DigitalWrite(pinChipSelect_, HIGH);
        
        SPI.begin();
    }
    
    void SetFrequency(uint32_t frequency, uint8_t updateNow = 0)
    {
        frequency_ = frequency;
        
        if (updateNow)
        {
            SetFrequencyInternal(frequency_);
            ChangeStateToTx();
        }
    }
    
    void SetDeviation(uint32_t deviationFreq, uint8_t updateNow = 0)
    {
        deviationFreq_ = deviationFreq;
        
        if (updateNow)
        {
            SetDeviationInternal(deviationFreq_);
            ChangeStateToTx();
        }
    }
    
    void Start()
    {
        PowerOnReset();
        PowerUp();
        ClearInterrupts();
        
        SetFrequencyInternal(frequency_);
        
        if (deviationFreq_ != 0)
        {
            SetDeviationInternal(deviationFreq_);
        }

        SetModemTransmitOnDirectInput();
        
        ChangeStateToTx();
    }
    
    void Stop()
    {
        PAL.DigitalWrite(pinShutdown_, HIGH);
    }
    
    uint8_t SetProperty(uint8_t propGroup, uint8_t propIdx, uint8_t value)
    {
        RFSI4463PRO::SET_PROPERTY_REQ req;
        
        req.GROUP.GROUP           = propGroup;
        req.NUM_PROPS.NUM_PROPS   = 1;
        req.START_PROP.START_PROP = propIdx;
        req.DATA0.DATA0           = value;
        
        uint8_t retVal = Command_SET_PROPERTY(req);
        
        return retVal;
    }
    
    uint8_t GetProperty(uint8_t propGroup, uint8_t propIdx, uint8_t &value)
    {
        RFSI4463PRO::GET_PROPERTY_REQ req;
        RFSI4463PRO::GET_PROPERTY_REP rep;
        
        req.GROUP.GROUP           = propGroup;
        req.NUM_PROPS.NUM_PROPS   = 1;
        req.START_PROP.START_PROP = propIdx;
        
        uint8_t retVal = Command_GET_PROPERTY(req, rep);
        
        if (retVal)
        {
            value = rep.DATA0.DATA0;
        }
        
        return retVal;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Generated code
    //
    ///////////////////////////////////////////////////////////////////////////

    #include "RFSI4463PRO_Generated.h"
    
    
private:

    void PowerOnReset()
    {
        PAL.DigitalWrite(pinShutdown_, LOW);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
        
        PAL.DigitalWrite(pinShutdown_, HIGH);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
        
        PAL.DigitalWrite(pinShutdown_, LOW);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
    }
    
    uint8_t PowerUp()
    {
        RFSI4463PRO::POWER_UP_REQ req;
        
        /*
         * Full functionality, not bootloader
         * External crystal, not internal
         */
        req.BOOT_OPTIONS.FUNC = 1;
        req.XTAL_OPTIONS.TCXO = 1;
        req.XO_FREQ.XO_FREQ   = EXTERNAL_CRYSTAL_FREQ;
        
        uint8_t retVal = Command_POWER_UP(req);
        
        return retVal;
    }
    
    uint8_t ClearInterrupts()
    {
        RFSI4463PRO::GET_INT_STATUS_REQ req;
        RFSI4463PRO::GET_INT_STATUS_REP rep;
        
        uint8_t retVal = Command_GET_INT_STATUS(req, rep);
        
        return retVal;
    }
    
    void ChangeStateToTx()
    {
        // Send START_TX command with no optional arguments specified.
        // This simplified form avoids the need to define auto-generated message
        // structure that you don't really want to send all of.
        SendAndWaitAndReceive(0x31, NULL, 0, NULL, 0);
    }
    
    void ChangeStateToSleep()
    {
        CHANGE_STATE_REQ req;
        
        req.NEXT_STATE.NEW_STATE = 1;
        
        Command_CHANGE_STATE(req);
    }
    
    void ChangeStateToReady()
    {
        CHANGE_STATE_REQ req;
        
        req.NEXT_STATE.NEW_STATE = 3;
        
        Command_CHANGE_STATE(req);
    }
    
    void SetModemTransmitOnDirectInput()
    {
        // Configure GPIO0 as input to be used as source of data to TX
        ConfigureGpio0AsInput();
        
        // Drive modem as async direct
        SetModemAsyncDirect();
    }
    
    uint8_t ConfigureGpio0AsInput()
    {
        RFSI4463PRO::GPIO_PIN_CFG_REQ req;
        RFSI4463PRO::GPIO_PIN_CFG_REP rep;

        req.GPIO0.GPIO_MODE = 4;
        req.GPIO1.GPIO_MODE = 16;
        
        uint8_t retVal = Command_GPIO_PIN_CFG(req, rep);
        
        return retVal;
    }
    
    uint8_t SetModemAsyncDirect()
    {
        MODEM_MOD_TYPE_PROP prop;

        // async
        prop.BYTE0.TX_DIRECT_MODE_TYPE = 1;
        
        // GPIO0
        prop.BYTE0.TX_DIRECT_MODE_GPIO = 0;
        
        // Real-time sourcing
        prop.BYTE0.MOD_SOURCE = 1;
        
        // 2FSK
        prop.BYTE0.MOD_TYPE = 2;
        
        return SetProperty(prop);
    }
    
    void SetFrequencyInternal(uint32_t freq)
    {
        uint8_t  band;
        uint8_t  fcInt;
        uint32_t fcFrac;
        uint8_t  outdiv;
        
        GetFrequencyParameters(freq, band, fcInt, fcFrac, outdiv);
        
        SetBand(band);
        SetFreqControlInt(fcInt);
        SetFreqControlFrac(fcFrac);
    }
    
    uint8_t SetBand(uint8_t band)
    {
        MODEM_CLKGEN_BAND_PROP prop;
        
        // force recalibration
        // high-performance mode (mandated to work with equations in spec)
        // band value passed-in
        prop.BYTE0.SY_SEL = 1;
        prop.BYTE0.BAND   = band;
        
        return SetProperty(prop);
    }
    
    uint8_t SetFreqControlInt(uint8_t fcInt)
    {
        FREQ_CONTROL_INTE_PROP prop;
        
        prop.BYTE0.INTE = fcInt;
        
        return SetProperty(prop);
    }
    
    void SetFreqControlFrac(uint32_t fcFrac)
    {
        // FREQ_CONTROL_FRAC
        const uint8_t PROP_GROUP = 0x40;
        const uint8_t PROP_IDX0  = 0x01;
        const uint8_t PROP_IDX1  = 0x02;
        const uint8_t PROP_IDX2  = 0x03;
        
        uint32_t bigEndian = PAL.htonl(fcFrac);
        uint8_t *p = (uint8_t *)&bigEndian;
        
        SetProperty(PROP_GROUP, PROP_IDX0, (0b00001111 & p[1]));
        SetProperty(PROP_GROUP, PROP_IDX1, (0b11111111 & p[2]));
        SetProperty(PROP_GROUP, PROP_IDX2, (0b11111111 & p[3]));
    }
    
    void SetDeviationInternal(uint32_t deviationFreq)
    {
        uint8_t  band;
        uint8_t  fcInt;
        uint32_t fcFrac;
        uint8_t  outdiv;

        // Get parameters which are needed in subsequent calculation
        GetFrequencyParameters(frequency_, band, fcInt, fcFrac, outdiv);
        
        // This is after hunting around in the datasheet.
        // Basically, when in high-performance mode, which we're in,
        // this value is 2.
        uint8_t nPresc = 2;
        
        // Do the calculation as seen in the datasheet
        uint32_t modemFreqDev =
            ((double)((uint32_t)2 << 18) * outdiv * deviationFreq) /
            (nPresc * EXTERNAL_CRYSTAL_FREQ);
        
        // Populate the property structure.
        // Unfortunately this is 3 bytes, encompassing 17 bits, and since
        // the byte layout is Big Endian, that means the first byte is really
        // just 1 bit.
        // Bit gymnastics required.
        
        // Default: 0x0006D3 = 1747, which amounts to deviationFreq = 8330
        MODEM_FREQ_DEV_PROP prop;
        
        prop.BYTE0.FREQDEV = (modemFreqDev & 0x00010000) >> 16;
        prop.BYTE1.FREQDEV = (modemFreqDev & 0x0000FF00) >>  8;
        prop.BYTE2.FREQDEV = (modemFreqDev & 0x000000FF) >>  0;
        
        /*
        @fix@Serial.print(F("outdiv: ")); @fix@Serial.println(outdiv);
        @fix@Serial.print(F("EXTERNAL_CRYSTAL_FREQ: ")); @fix@Serial.println(EXTERNAL_CRYSTAL_FREQ);
        @fix@Serial.print(F("deviationFreq: ")); @fix@Serial.println(deviationFreq);
        @fix@Serial.print(F("modemFreqDev: ")); @fix@Serial.println(modemFreqDev);
        @fix@Serial.print(F("prop.BYTE0.FREQDEV: ")); @fix@Serial.println(prop.BYTE0.FREQDEV);
        @fix@Serial.print(F("prop.BYTE1.FREQDEV: ")); @fix@Serial.println(prop.BYTE1.FREQDEV);
        @fix@Serial.print(F("prop.BYTE2.FREQDEV: ")); @fix@Serial.println(prop.BYTE2.FREQDEV);
        */
        
        SetProperty(prop);
    }
    
    
    /*
     *
     * The datasheet says the module will operate at a given frequency given
     * a number of factors.
     * Those are:
     * - freq_xo (crystal frequency -- fixed)
     * - outdiv (determined by lookup table in spec for a desired frequency)
     * - fc_inte, fc_frac -- whole and "fractional" settable values
     *
     * So in short, fc_inte and fc_frac are numbers you need to calculate,
     * everything else is basically fixed.
     *
     * The specific formula is:
     *
     *              (           fc_frac  )   ( 2 * freq_xo )
     * RF_channel = ( fc_inte + -------- ) * ( ----------- )
     *              (             2^19   )   (    outdiv   )
     *
     * 
     * The spec notes that the (fc_frac / 2^19) value is in the range of 1-2.
     *
     *
     * A good solution for solving this was seen on the pecan pico implementation.
     * https://github.com/tkrahn/pecanpico4/blob/master/radio_si446x.cpp
     * This is reformatted for clarity below.
     *
     * It is basically:
     * - calculate the known ((2*freq_xo)/outdiv) value and divide RF_channel by it.
     * - you now know the right-hand of the equation is what you need to solve.
     * - that is done by finding what is left over after removing 1+x from the left
     *   hand side of the equation.
     * 
     */
    void GetFrequencyParameters(uint32_t  freq,
                                uint8_t  &band,
                                uint8_t  &fcInt,
                                uint32_t &fcFrac,
                                uint8_t  &outdiv)
    {
        // Start with default assumption that highest-frequency chosen, fall down
        // ladder from there.
        //
        // Notably this is incorporates ranges from both the 4461/2/3 and 4464.
        outdiv = 4;
        band           = 0;
        if (freq < 705000000UL) { outdiv =  6; band = 1; };
        if (freq < 525000000UL) { outdiv =  8; band = 2; };
        if (freq < 353000000UL) { outdiv = 12; band = 3; };
        if (freq < 239000000UL) { outdiv = 16; band = 4; };
        if (freq < 177000000UL) { outdiv = 24; band = 5; };

        // Calculate the known part of the equation
        double secondParenValue =
            (2.0 * (double)EXTERNAL_CRYSTAL_FREQ) / (double)outdiv;

        // Calculate what the right hand side must equal once the known part of
        // the equation is moved to the left
        double leftHandSide = (double)freq / secondParenValue;

        // The fc_inte value is:
        // - the left hand side
        //   - shifted by 1 to account for the fc_frac/2^19 being at least 1
        //   - truncated to eliminate any fractional part above 1
        fcInt = (uint8_t)(leftHandSide - 1);

        // The difference between the leftHandSide and fcInte must be equal to the
        // fc_frac / 2^19.
        // Calculate it in decimal first.
        // This will be in the range of 1-2.
        double remainingValue = leftHandSide - fcInt;

        // Now scale this value to find fc_frac.
        fcFrac = (uint32_t)(remainingValue * (uint32_t)((uint32_t)2 << 18));
        
        /*
        @fix@Serial.println("SetFrequency");
        @fix@Serial.print("freq  : "); @fix@Serial.println(freq);
        @fix@Serial.print("band  : "); @fix@Serial.println(band);
        @fix@Serial.print("fcInt : "); @fix@Serial.println(fcInt);
        @fix@Serial.print("rem   : "); @fix@Serial.println(remainingValue);
        @fix@Serial.print("fcFrac: "); @fix@Serial.println(fcFrac);
        @fix@Serial.println();
        */
    }
    
    uint8_t SendAndWaitAndReceive(uint8_t  cmd,
                                  uint8_t *reqBuf,
                                  uint8_t  reqBufLen,
                                  uint8_t *repBuf,
                                  uint8_t  repBufLen)
    {
        const uint8_t CMD_READ_CMD_BUFF = 0x44;
    
        const uint8_t VAL_TO_SEND_WHEN_READING = 0x00;
        const uint8_t VAL_CLEAR_TO_SEND        = 0xFF;
        
        uint8_t retVal = 0;
        
        //
        // Step 1 -- Issue command
        //
        SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
        PAL.DigitalWrite(pinChipSelect_, LOW);

        SPI.transfer(cmd);
        
        // Transfer as many command supplemental parameters as indicated
        // (could be zero based on input, which is valid)
        for (uint8_t i = 0; i < reqBufLen; ++i)
        {
            SPI.transfer(reqBuf[i]);
        }

        PAL.DelayMicroseconds(DELAY_US_BEFORE_CHIP_SELECT_RELEASE);
        
        PAL.DigitalWrite(pinChipSelect_, HIGH);
        SPI.endTransaction();
        
        //
        // Step 2 -- Wait for chip to indicate that the result of the above
        // command is complete and any data ready to read.
        //
        // This is done by issuing another command where:
        // - issue the second command
        // - wait for a CTS (clear to send) indication to be returned
        //   (despite the fact that we're actually waiting to read, not send)
        // - read however many bytes are expected
        //
        // If the chip indicates the results aren't ready to be read, start
        // another complete transactional attempt to read again.
        //
        uint16_t readAttempts = 0;
        uint8_t  rep          = !VAL_CLEAR_TO_SEND;
        while (rep != VAL_CLEAR_TO_SEND && readAttempts < READ_ATTEMPT_LIMIT)
        {
            ++readAttempts;
            
            SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
            PAL.DigitalWrite(pinChipSelect_, LOW);

            // Send command indicating we want to see the result of the previous
            // command.
            SPI.transfer(CMD_READ_CMD_BUFF);
            
            // Now wait for an indication that the result of the first command is
            // actually ready to read.
            rep = SPI.transfer(VAL_TO_SEND_WHEN_READING);

            if (rep == VAL_CLEAR_TO_SEND)
            {
                retVal = 1;
                
                // Command result ready to read.
                // Consume as many bytes of response as are expected
                // (could be zero based on input, which is valid)
                for (uint8_t i = 0; i < repBufLen; ++i)
                {
                    repBuf[i] = SPI.transfer(VAL_TO_SEND_WHEN_READING);
                }
            }

            PAL.DelayMicroseconds(DELAY_US_BEFORE_CHIP_SELECT_RELEASE);
            
            PAL.DigitalWrite(pinChipSelect_, HIGH);
            SPI.endTransaction();
        }
        
        return retVal;
    }

private:

    uint8_t  pinChipSelect_;
    uint8_t  pinShutdown_;
    uint32_t frequency_;
    uint32_t deviationFreq_;
};



#endif  // __RF_SI4463PRO_H__