#ifndef __RF_SI4463PRO_H__
#define __RF_SI4463PRO_H__


#include "RF4463.h"
#include <SPI.h>
#include "radio_config_Si4463.h"

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
    //static const uint16_t READ_ATTEMPT_LIMIT                  = 2500;
    static const uint8_t  DELAY_US_BEFORE_CHIP_SELECT_RELEASE = 50;
    
    // Details relating to operating this chip    
    static const uint32_t DURATION_MS_POWER_ON_RESET = 50;
    static const uint32_t EXTERNAL_CRYSTAL_FREQ      = 30000000;
    
    
    
public:

    enum class ModulationType : uint8_t
    {
        MT_OOK = 1,
        MT_2FSK = 2,
    };
    
    
public:
    RFSI4463PRO(uint8_t pinIrq, uint8_t pinChipSelect, uint8_t pinShutdown)
    : pinIrq_(pinIrq)
    , pinChipSelect_(pinChipSelect)
    , pinShutdown_(pinShutdown)
    , deviationFreq_(0)
    , modulationType_(ModulationType::MT_2FSK)
    {
        // Nothing to do
    }



    /*
     * Layout of interface
     * 
     * [public]
     * - ctor, pinSetting
     * - API functions
     *   - common
     *   - packet mode
     *   - raw mode
     * 
     * [private]
     * 
     * - misc
     * 
     * - command convenience functions
     * - command primitives
     * 
     * - property convenience functions
     * - property primitives
     * 
     * - module communication primitives
     * - spi / module electrical primitives
     * 
     * - unclassified
     * - legacy
     * 
     */




    ///////////////////////////////////////////////////////////////////
    //
    // Constructor
    //
    ///////////////////////////////////////////////////////////////////








    ///////////////////////////////////////////////////////////////////
    //
    // API Functions - Common
    //
    ///////////////////////////////////////////////////////////////////



    uint8_t init()
    {
        uint8_t retVal = 0;

        Log(P("init()"));

        // This inits pins and SPI as the lib does
        Init();

        // The lib then does two things in one function call:
        // powerOnReset();
        // - power on reset
        // - send power up command

        PowerOnReset(); // mine is the same as theirs
        Log(P("POR"));

        // Their powerup is the same as mine, except they don't enable
        // the TCXO for some reason (use XTAL)
        //#define RF_POWER_UP 0x02, 0x01, 0x00, 0x01, 0xC9, 0xC3, 0x80
        PowerUp2();
        Log(P("PU2"));


        // Should be able to communicate via SPI at this point, test it
        retVal = CheckDevice();
        if (retVal)
        {
            Log("CheckDevice SUCCESS");
        }
        else
        {
            Log("CheckDevice FAIL");
        }


        // The lib then sets the .h value config
        ReadConfig();
        Log(P("RC"));

        MoreConfig();
        Log(P("MC"));


        return retVal;
    }

    uint8_t SetTxPower(uint8_t power)
    {
        return SetTxPowerInternal(power);
    }

    uint8_t EnterStandbyMode()
    {
        return CmdChangeState_StandbyMode();
    }






    ///////////////////////////////////////////////////////////////////
    //
    // API Functions - Packet Mode
    //
    ///////////////////////////////////////////////////////////////////


// Reset to state where waiting for packet.
// Any interrupt IRQ line is reset at this point.
// Once IRQ fires, you can read the packet.
// Call this again once you've received the packet.  (why, so shitty)
//   (but certainly caller will need to clear IRQ line somehow, so don't
//    forget if you change behavior here)
//
// What happens if you receive multiple packets before clearing IRQ?
// Do you throw away the fifo contents?
bool rxInit()
{		
	uint8_t length;
	length=50;
	SetPacketField2Length(length);

    ResetRxTxFifos();

    SetRxInterrupt();

	ClearInterrupts();
	
    CmdStartRx_UsePktFieldConfigForSizing();

    return true;
}


uint8_t rxPacket(uint8_t *recvbuf)
{
    // TODO -- check if CRC error on reception
    // seems that packet_rx event fires, but CRC can also be wrong
    // at that point as well


	uint8_t rxLen;
	rxLen=ReadRxFifo(recvbuf);			// read data from fifo
	ResetRxTxFifos();

	return rxLen;
}


bool txPacket(uint8_t* sendbuf,uint8_t sendLen, uint8_t syncSend)
{
	uint8_t retVal = 0;

	uint16_t txTimer;

	ResetRxTxFifos();
	writeTxFifo(sendbuf,sendLen);		// load data to fifo	
	SetTxInterrupt();
	ClearInterrupts();
	enterTxMode();						// enter TX mode

	if (!syncSend)
	{
		retVal = true;
	}
	else
	{
		txTimer=RF4463_TX_TIMEOUT;
		while(txTimer--)
		{
			if(waitnIRQ())					// wait INT
			{
				retVal = 1;
			}
			else
			{
				delay(1);
			}
		}

		if (!retVal)
		{
			init();								// reset RF4463 if tx time out
		}
	}

	return retVal;
}









    ///////////////////////////////////////////////////////////////////
    //
    // API Functions - Raw Mode
    //
    ///////////////////////////////////////////////////////////////////












































    ///////////////////////////////////////////////////////////////////
    //
    // Misc
    //
    ///////////////////////////////////////////////////////////////////


    uint8_t PowerUp2()
    {
        RFSI4463PRO::POWER_UP_REQ req;
        
        req.BOOT_OPTIONS.FUNC = 1;
        req.XTAL_OPTIONS.TCXO = 1;
        req.XO_FREQ.XO_FREQ   = EXTERNAL_CRYSTAL_FREQ;
        
        uint8_t retVal = Command_POWER_UP(req);
        
        return retVal;
    }

    uint8_t CheckDevice()
    {
        uint8_t retVal = 0;

        const uint8_t RESPONSE_LEN = 9;
        uint8_t buf[RESPONSE_LEN] = { 0 };

        const uint8_t COMMAND_PART_INFO = 0x01;
        SendCommand(COMMAND_PART_INFO, nullptr, 0, buf, RESPONSE_LEN);

        uint16_t partInfo = (buf[1] << 8) | buf[2];

        if (partInfo == 0x4463)
        {
            retVal = 1;
        }

        return retVal;
    }

    void ReadConfig()
    {
        // setConfig(RF4463_CONFIGURATION_DATA,sizeof(RF4463_CONFIGURATION_DATA));
        // Configuration parameters from "radio_config_Si4463.h"
        static uint8_t RF4463_CONFIGURATION_DATA[] = RADIO_CONFIGURATION_DATA_ARRAY;

        const uint8_t* parameters = RF4463_CONFIGURATION_DATA;
        uint16_t paraLen = sizeof(RF4463_CONFIGURATION_DATA);

        // command buf starts with length of command in RADIO_CONFIGURATION_DATA_ARRAY
        uint8_t cmdLen;
        uint8_t command;
        uint16_t pos;
        uint8_t buf[30];

        // power up command had already send
        paraLen=paraLen-1;
        cmdLen=parameters[0];
        pos=cmdLen+1;
        
        while(pos<paraLen)
        {
            cmdLen=parameters[pos++]-1;		// get command lend
            command=parameters[pos++];		// get command
            memcpy(buf,parameters+pos,cmdLen);		// get parameters
            
            //setCommand(cmdLen,command,buf);
            {
                uint8_t  cmd           = command;
                uint8_t *cmdArgList    = buf;
                uint8_t  cmdArgListLen = cmdLen;

                SendCommand(cmd, cmdArgList, cmdArgListLen);
            }

            pos=pos+cmdLen;
        }
    }

    void MoreConfig()
    {
        SetSequencerModeGuaranteed();

        // TODO -- we do want CRCs, but 2 or 4 byte.
        // Want to stop reading in the radio config.
        // Then figure out how to tune this.
        SetPacketCRC();
        
        // TODO -- stop using this as-is and work out a specific packet format
        // that you want.
        SetPacketRxVariableLenConfiguration();
        
        // TODO -- stop using this as-is and work out a specific packet format
        // that you want.
        SetPacketField2Configuration();

        SetPacketField4And5Configuration();

        // set max tx power
        SetTxPower(127);


        SetFrequencyInternal((uint32_t)433500000);


        // To get rid of RF_MODEM_MOD_TYPE_12 from radio_config, put these two in place.

        // Set data rate
        /* MODEM_DATA_RATE (0x0F, 0x42, 0x40 default of 100kbps, not sure below value without calculations) */ \
        {
            uint8_t group = 0x20;
            uint8_t propIdx = 0x03;
            uint8_t buf[] = {
                0x00,
                0x2E,
                0xE0,
            };
            uint8_t bufLen = sizeof(buf);

            SetProperty(group, propIdx, buf, bufLen);
        }

        // Set frequency deviation
        /* MODEM_FREQ_DEV (0x00,0x06,0xd3 default, not sure below without calculation) */ \
        {
            uint8_t group = 0x20;
            uint8_t propIdx = 0x0A;
            uint8_t buf[] = {
                0x00,
                0x00,
                0xD2,
            };
            uint8_t bufLen = sizeof(buf);

            SetProperty(group, propIdx, buf, bufLen);
        }









        // To get rid of RF_MODEM_TX_RAMP_DELAY_8 from radio_config, put these two in place.



        // Set frequency deviation
        /* MODEM_IF_FREQ (0x03,0xC0,0x00 default) */ \
        /*   affects intermedite frequency in some way requiring calculation */ \
        {
            uint8_t group = 0x20;
            uint8_t propIdx = 0x1B;
            uint8_t buf[] = {
                0x03,
                0x80,
                0x00,
            };
            uint8_t bufLen = sizeof(buf);

            SetProperty(group, propIdx, buf, bufLen);
        }

        /* MODEM_DECIMATION_CFG1 (0x10 default) */ \
        /* controls CIC (cascaded integrator comb) filter in some way */ \
        {
            uint8_t group = 0x20;
            uint8_t propIdx = 0x1E;
            uint8_t buf[] = {
                0x70,
            };
            uint8_t bufLen = sizeof(buf);

            SetProperty(group, propIdx, buf, bufLen);
        }






    }



    ///////////////////////////////////////////////////////////////////
    //
    // Command Convenience Functions
    //
    ///////////////////////////////////////////////////////////////////
    
    uint8_t ClearInterrupts()
    {
        return CmdGetIntStatus_ClearInterrupts();
    }

    uint8_t ResetRxTxFifos()
    {
        return CmdFifoInfo_ResetRxTxFifos();
    }


    uint8_t SetTxPowerInternal(uint8_t power)
    {
        uint8_t retVal = 0;

        if (power <= 127)
        {
            uint8_t  group   = 0x22;
            uint8_t  propIdx = 0x00;
            uint8_t  buf[]   = {
                0x08,   // PA_MODE
                power,  // PA_PWR_LVL
                0x00,   // PA_BIAS_CLKDUTY
                0x3D,   // PA_TC
            };
            uint8_t  bufLen  = sizeof(buf);

            retVal = SetProperty(group, propIdx, buf, bufLen);
        }

        return retVal;
    }




uint8_t ReadRxFifo(uint8_t* databuf)
{
	if(!checkCTS())
		return 0;
	uint8_t readLen;
    PAL.DigitalWrite(pinChipSelect_, LOW);
	spiByte(RF4463_CMD_RX_FIFO_READ);
	spiReadBuf(1,&readLen);
	spiReadBuf(readLen,databuf);
    PAL.DigitalWrite(pinChipSelect_, HIGH);
	return readLen;
}


void writeTxFifo(uint8_t* databuf,uint8_t length)
{
    SetPacketField2Length(length);

	setCommandStart();
	setCommandWriteCommand(RF4463_CMD_TX_FIFO_WRITE);
	setCommandWriteBuffer(1, &length);
	setCommandWriteBuffer(length, databuf);
	setCommandEnd();
}







void enterTxMode()
{
	uint8_t buf[]={0x00,0x30,0x00,0x00};
	buf[0]=RF4463_FREQ_CHANNEL;
	setCommand(4,RF4463_CMD_START_TX ,buf);
}




    ///////////////////////////////////////////////////////////////////
    //
    // Command Primitives
    //
    ///////////////////////////////////////////////////////////////////

    uint8_t CmdStartRx_UsePktFieldConfigForSizing()
    {
        uint8_t retVal = 0;

        // Cmd START_RX
        uint8_t cmd = 0x32;
        uint8_t buf[] = {
            // CHANNEL -- just leave at 0 to avoid questions around what channel to use
            0x00,

            // CONDITION
            //   UPDATE - apply now, not for the future
            //   START  - start rx immediately
            0x00,

            // RX_LEN -- use the PKT_FIELD_X_LENGTH properties
            0x00,
            0x00,
            
            // NEXT_STATE1
            //   RXTIMEOUT_STATE[3:0] --
            //     what to do if preamble detection times out.  0 = stay in RX.
            0x00,

            // NEXT_STATE2
            //   RXVALID_STATE[3:0] --
            //     go to what state when a valid packet recieved?
            //     8 = re-arm to receive next packet
            0x08,

            // NEXT_STATE3
            //   RXINVALID_STATE[3:0] --
            //     go to what state when an invalid packet received?
            //     8 = re-arm to receive next packet
            0x08,
        };
        uint8_t bufLen = sizeof(buf);

        retVal = SendCommand(cmd, buf, bufLen);

        return retVal;
    }


    uint16_t CmdGetPhStatus_Debug()
    {
        // Cmd GET_PH_STATUS
        uint8_t cmd = 0x21;
        uint16_t bufRep;
        uint8_t bufRepLen = sizeof(bufRep);

        SendAndWaitAndReceive(cmd, nullptr, 0, (uint8_t *)&bufRep, bufRepLen);

        return bufRep;
    }

    uint8_t CmdGetIntStatus_ClearInterrupts()
    {
        uint8_t retVal = 0;

        // Cmd GET_INT_STATUS
        // Basically set all interrupt indications to zero to clear them out
        uint8_t cmd = 0x20;
        uint8_t buf[] = {
            // PH_CLR_PEND
            0x00,
            // MODEM_CLR_PEND
            0x00,
            // CHIP_CLR_PEND
            0x00,
        };
        uint8_t bufLen = sizeof(buf);

        retVal = SendCommand(cmd, buf, bufLen);

        return retVal;
    }

    uint8_t CmdFifoInfo_GetCountsDebug()
    {
        uint8_t retVal = 0;

        // Cmd FIFO_INFO
        uint8_t cmd = 0x15;
        uint8_t bufRep[2] = { 0 };
        uint8_t bufRepLen = sizeof(bufRep);

        retVal = SendAndWaitAndReceive(cmd, nullptr, 0, bufRep, bufRepLen);

        Log("RX_FIFO_COUNT: ", bufRep[0]);
        Log("TX_FIFO_SPACE: ", bufRep[1]);

        return retVal;
    }

    uint8_t CmdFifoInfo_ResetRxTxFifos()
    {
        uint8_t retVal = 0;

        // Cmd FIFO_INFO
        uint8_t cmd = 0x15;
        uint8_t buf[] = {
            // FIFO
            //       1 = Reset TX FIFO
            //      1  = Reset RX FIFO
            0b00000011,
        };
        uint8_t bufLen = sizeof(buf);

        retVal = SendCommand(cmd, buf, bufLen);

        return retVal;
    }

    uint8_t CmdChangeState_StandbyMode()
    {
        uint8_t retVal = 0;

        // Cmd CHANGE_STATE
        uint8_t cmd = 0x34;
        uint8_t buf[] = {
            // NEW_STATE
            //   1 = SLEEP
            0x01,
        };
        uint8_t bufLen = sizeof(buf);

        retVal = SendCommand(cmd, buf, bufLen);

        return retVal;
    }


    ///////////////////////////////////////////////////////////////////
    //
    // Property Convenience Functions
    //
    ///////////////////////////////////////////////////////////////////

    uint8_t SetRxInterrupt()
    {
        uint8_t retVal = 0;

        uint8_t  group   = 0x01;
        uint8_t  propIdx = 0x00;
        uint8_t  buf[]   = {
            // INT_CTL_ENABLE (0x04 default)
            //   CHIP_INT_STATUS_EN (1 = default)
            //     0 = disable
            //   MODEM_INT_STATUS_EN (0 = default)
            //     0 = disable
            //   PH_INT_STATUS_EN (0 = default)
            //     1 = TX mode can fire IRQ on interrupts
            0x01,

            // INT_CTL_PH_ENABLE (0x00 default) -- select which PH events trigger interrupts
            //   PACKET_RX = 1
            0x10,

            // INT_CTL_MODEM_ENABLE (0x00 default) -- select which modem events trigger interrupts
            //   disables all events
            0x00,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetTxInterrupt()
    {
        uint8_t retVal = 0;

        uint8_t  group   = 0x01;
        uint8_t  propIdx = 0x00;
        uint8_t  buf[]   = {
            // INT_CTL_ENABLE (0x04 default)
            //   CHIP_INT_STATUS_EN (1 = default)
            //     0 = disable
            //   MODEM_INT_STATUS_EN (0 = default)
            //     0 = disable
            //   PH_INT_STATUS_EN (0 = default)
            //     1 = TX mode can fire IRQ on interrupts
            0x01,

            // INT_CTL_PH_ENABLE (0x00 default) -- select which PH events trigger interrupts
            //   PACKET_SENT_EN = 1
            0x20,

            // INT_CTL_MODEM_ENABLE (0x00 default) -- select which modem events trigger interrupts
            //   disables all events
            0x00,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetSequencerModeGuaranteed()
    {
        uint8_t retVal = 0;

        // Set GLOBAL_CONFIG.
        // Carried over from other library, but that used to set to 0x40.
        //
        // The stated default is 0x20.  We don't like that default.
        //
        // The observed default (after reading in the radio_config) was 0x60.
        // That shows RESERVED=1 and SEQUENCER_MODE=1(FAST).
        // Not sure what RESERVED is about.
        // We don't want the module going "FAST," it can mess up TX.
        //
        // There are other configurations in the GLOBAL_CONFIG, but for the
        // time being it's no conflict to set all of these together.
        //
        // It is not necessary to set the RESERVED=1 as far as I can tell in
        // my test cases.
        uint8_t  group   = 0x00;
        uint8_t  propIdx = 0x03;
        uint8_t  buf[]   = {
            0x00,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetPreamble()
    {
        uint8_t retVal = 0;


        // All told, the below, copied from other lib:
        // - keeps all but 1 defaults
        // - including 8 PREAMBLE_TX_LENGTH
        // - except PREAMBLE_TX_LENGTH means bytes not nibbles (2x more transitions)
        // 
        // I suspect this can just be ignored, and use default values across the board.
        //
        // Will check if the read-in radio_config tries to mess with this before deleting.
        //     Aaaaand it does.
        // Will check if compatible with other radio configured for bytes while this
        // radio is configured for nibbles.  I suspect yes.
        //     And it is.  And a bit faster to transmit also (to be expected).
        //
        // In summary, I'm commenting out this code to save for the future one
        // day if I feel like tweaking the preamble.
        // I don't feel like it today.  I want to use the defaults.
        // Ceasing to read in the radio_config won't affect this because the
        // radio config just wants to over-do it a bit.  This is fine.

        uint8_t  group   = 0x10;
        uint8_t  propIdx = 0x00;
        uint8_t  buf[]   = {
            0x08,   // PREAMBLE_TX_LENGTH    (0x08 default)
            0x14,   // PREAMBLE_CONFIG_STD_1 (0x14 default)
            0x00,   // PREAMBLE_CONFIG_NSTD  (0x00 default)
            0x0F,   // PREAMBLE_CONFIG_STD_2 (0x0F default)


            // override the below to:
            // RF4463_PREAMBLE_FIRST_1 (0x20) |         (this is default)
            // RF4463_PREAMBLE_LENGTH_BYTES (0x10) |    (this is changed -- use bytes not nibbles of PREAMBLE_TX_LENGTH)
            // RF4463_PREAMBLE_STANDARD_1010 (0x01)     (this is default)
            // 
            // this nets out to 0x31 (0011 0001)
            //0x21,   // PREAMBLE_CONFIG       (0x21 default)
            0x31,   // PREAMBLE_CONFIG       (0x21 default)



            0x00,   // PREAMBLE_PATTERN_4    (0x00 default)
            0x00,   // PREAMBLE_PATTERN_3    (0x00 default)
            0x00,   // PREAMBLE_PATTERN_2    (0x00 default)
            0x00,   // PREAMBLE_PATTERN_1    (0x00 default)
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = ConfirmSetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetPacketCRC()
    {
        uint8_t retVal = 0;

        // Set PKT_CRC_CONFIG
        // Can configure
        // - CRC_SEED, whether use all 1s or all 0s when CRCing itself
        //   - default to 0s
        // - ALT_CRC_POLYNOMIAL
        //   - default to none
        // - CRC_POLYNOMIAL
        //   - default to none
        uint8_t  group   = 0x12;
        uint8_t  propIdx = 0x00;
        uint8_t  buf[]   = {
            // Default 0x00
            // RF4463_CRC_SEED_ALL_1S (0x80) |
            // RF4463_CRC_ITU_T (0x01)
            // 
            // Nets out to 0x81
            0x81,   // override to use all 1s and ITU_T_CRC8 (1-byte CRC)
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetPacketConfig()
    {
        uint8_t retVal = 0;

        // Set PKT_CONFIG1
        //
        // Appears to be pointless to override defaults here for CRC network
        // byte order because:
        // - the CRC elsewhere is configured to be 1 byte (so no odering)
        // - why not use the defaults?
        //
        // I suspect I can simply not tune this and let the single-byte CRC
        // be unaffected.
        //
        // That is unless the radio config overrides it and application code
        // is tuning it back?
        //
        // So, radio config does set this, but to the same value the app is
        // using.  So still should be able to remove completely.
        //
        // Confirmed, no effect.  Keep code for later if you want to revisit, or
        // just delete.
        //
        uint8_t  group   = 0x12;
        uint8_t  propIdx = 0x06;
        uint8_t  buf[]   = {
            // Default 0x00
            // RF4463_CRC_ENDIAN (0x02) -- change to be network byte order away from default LSB
            // 
            // Nets out to 0x02
            0x02,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetPacketRxVariableLenConfiguration()
    {
        uint8_t retVal = 0;

        // Ultimately sets:
        // - Field 2 is variable sized
        // - Field 1 value is the size of field 2
        // No idea why this is being done.
        // Why not just send frames and extract the value yourself?
        //
        // In fact, I believe that sending code already has to craft this
        // the first-byte-as-length-value itself.
        //
        // Hmm but wait, isn't field 1 CRC'd?
        // Is Field 2 CRC'd?
        //
        // Is the radio config overriding any of this?  (no)
        //
        // Testing shows can still receive from existing transmitters when all
        // values explicitly set to defaults.
        //
        // Need to understand better the specific packet configuration in play
        // and how CRCs fit into that.

        // Set:
        // PKT_LEN              - Configuration bits for reception of a variable length packet
        // PKT_LEN_FIELD_SOURCE - Field number containing the received packet length byte(s).
        // PKT_LEN_ADJUST       - Provides for adjustment/offset of the received packet length value (in order to accommodate a variety of methods of defining total packet length).
        uint8_t  group   = 0x12;
        uint8_t  propIdx = 0x08;
        uint8_t  buf[]   = {
            // PKT_LEN (0x00 default)
            // override to
            //   RF4463_IN_FIFO (0x08) |    -- LEAVE_IN the field length data in RX FIFO (as opposed to stripping out)
            //   RF4463_DST_FIELD_ENUM_2 (0x02) -- Field 2 is the variable length field
            //   Nets out to 0x0A
            // 
            0x0A,
            //0x00,

            // PKT_LEN_FIELD_SOURCE (0x00 default)
            // override to
            //   RF4463_SRC_FIELD_ENUM_1 (0x01) -- Use Field 1 as the size of Field 2
            //   Nets out to 0x01
            0x01,
            //0x00,

            // PKT_LEN_ADJUST (0x00 default)
            0x00,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    // Only supporting 8-bit values
    uint8_t SetPacketField2Length(uint8_t length)
    {
        uint8_t retVal = 0;

        uint8_t  group   = 0x12;
        uint8_t  propIdx = 0x11;
        uint8_t  buf[]   = {
            0x00,   // FIELD_2_LENGTH[12:8]
            length, // FIELD_2_LENGTH[7:0]
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetPacketField2Configuration()
    {
        uint8_t retVal = 0;

        uint8_t  group   = 0x12;
        uint8_t  propIdx = 0x0D;
        uint8_t  buf[]   = {
            // PKT_FIELD_1_LENGTH[12:8] (0x00 default)
            0x00,
            // PKT_FIELD_1_LENGTH[7:0] (0x00 default)
            // override to 1 byte
            0x01,

            // PKT_FIELD_1_CONFIG (0x00 default)
            // override to
            // RF4463_FIELD_CONFIG_PN_START (0x04) --
            //   load pseudo-random num (PN) seed value at start, don't carry
            //   over from prior packet.
            //
            0x04,
            
            // PKT_FIELD_1_CRC_CONFIG (0x00 default)
            // override to
            //   RF4463_FIELD_CONFIG_CRC_START  (0x80) --
            //     TX - load CRC with seed value at start, don't carry over from prior
            //   RF4463_FIELD_CONFIG_SEND_CRC   (0x20) --
            //     TX - send a CRC after this field
            //   RF4463_FIELD_CONFIG_CHECK_CRC  (0x08) --
            //     RX - check the CRC upon reception
            //   RF4463_FIELD_CONFIG_CRC_ENABLE (0x02) --
            //     RX/TX -- enable calculation (not necessarily send)
            //
            // Nets out to 0xAA
            0xAA,

            // PKT_FIELD_2_LENGTH (0x00, 0x00 default)
            // override to 50 (decimal) bytes, representing the maximum expected length
            // for when this field is variable (which it is)
            0x00,
            50,

            // PKT_FIELD_2_CONFIG (0x00 default)
            // override to
            //   RF4463_FIELD_CONFIG_PN_START (0x04) --
            //     Unclear what this does... setting a field with no defined meaning
            //     and was previously reserved.
            //     TODO -- I suspect this can be reset to default.
            0x04,
            
            // PKT_FIELD_2_CRC_CONFIG (0x00 default)
            // override to
            //   RF4463_FIELD_CONFIG_CRC_START  (0x80) --
            //     TODO -- this has no meaning here.  Reserved or 0 bitfield.
            //   RF4463_FIELD_CONFIG_SEND_CRC   (0x20) --
            //     TX - send a CRC after this field
            //   RF4463_FIELD_CONFIG_CHECK_CRC  (0x08) --
            //     RX - check the CRC upon reception
            //   RF4463_FIELD_CONFIG_CRC_ENABLE (0x02) --
            //     RX/TX -- enable calculation (not necessarily send)
            //
            // Nets out to 0xAA
            0xAA,

            // PKT_FIELD_3_LENGTH (0x00, 0x00 default)
            0x00,
            0x00,

            // PKT_FIELD_3_CRC_CONFIG (0x00 default)
            0x00,

            // PKT_FIELD_4_LENGTH (0x00 default)
            0x00,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }

    uint8_t SetPacketField4And5Configuration()
    {
        uint8_t retVal = 0;

        uint8_t  group   = 0x12;
        uint8_t  propIdx = 0x19;
        uint8_t  buf[]   = {
            // PKT_FIELD_4_LENGTH (0x00, 0x00 default)
            0x00,
            0x00,
            
            // PKT_FIELD_4_CONFIG (0x00 default)
            0x00,
            
            // PKT_FIELD_4_CRC_CONFIG (0x00 default)
            0x00,
            
            // PKT_FIELD_5_LENGTH (0x00, 0x00 default)
            0x00,
            0x00,
            
            // PKT_FIELD_5_CONFIG (0x00 default)
            0x00,
            
            // PKT_FIELD_5_CRC_CONFIG (0x00 default)
            0x00,
        };
        uint8_t  bufLen  = sizeof(buf);

        retVal = SetProperty(group, propIdx, buf, bufLen);

        return retVal;
    }



    ///////////////////////////////////////////////////////////////////
    //
    // Property Primitives
    //
    ///////////////////////////////////////////////////////////////////










    ///////////////////////////////////////////////////////////////////
    //
    // Module Communication Primitives - Commands
    //
    ///////////////////////////////////////////////////////////////////


bool setCommand(uint8_t length,uint8_t command,uint8_t* paraBuf)
{
	uint8_t retVal = 0;

    uint8_t  cmd           = command;
    uint8_t *cmdArgList    = paraBuf;
    uint8_t  cmdArgListLen = length;

    retVal = SendCommand(cmd, cmdArgList, cmdArgListLen);

	return retVal;
}



    uint8_t SendCommand(uint8_t  cmdId,
                        uint8_t *buf,
                        uint8_t  bufLen,
                        uint8_t *bufRep = nullptr,
                        uint8_t  bufRepLen = 0)
    {
        uint8_t ok = SendAndWaitAndReceive(cmdId, buf, bufLen, bufRep, bufRepLen);

        return ok;
    }



bool setCommandStart()
{
	if(!checkCTS())
		return false;

	PAL.DigitalWrite(pinChipSelect_, LOW);

	return true;
}
void setCommandWriteCommand(uint8_t command)
{
	spiByte(command);				// send command
}
void setCommandWriteBuffer(uint8_t length, uint8_t* paraBuf)
{
	spiWriteBuf(length,paraBuf);	// send parameters
}
void setCommandEnd()
{
	PAL.DigitalWrite(pinChipSelect_, HIGH);
}






    ///////////////////////////////////////////////////////////////////
    //
    // Module Communication Primitives - Properties
    //
    ///////////////////////////////////////////////////////////////////


    uint8_t GetProperty(uint8_t propGroup, uint8_t propIdx, uint8_t *buf, uint8_t bufLen)
    {
        uint8_t retVal = 0;

        uint8_t bufReq[] = {
            propGroup,
            bufLen,
            propIdx,
        };

        // Use the GET_PROPERTY command
        retVal = SendCommand(0x12, bufReq, sizeof(bufReq), buf, bufLen);

        return retVal;
    }

    uint8_t SetProperty(uint8_t propGroup, uint8_t propIdx, uint8_t *buf, uint8_t bufLen)
    {
        uint8_t bufReqLen = bufLen + 3;
        uint8_t bufReq[bufReqLen];

        bufReq[0] = propGroup;
        bufReq[1] = bufLen;
        bufReq[2] = propIdx;

        memcpy(&bufReq[3], buf, bufLen);

        uint8_t ok = SendCommand(0x11, bufReq, bufReqLen);

        return ok;
    }

    uint8_t ConfirmSetProperty(uint8_t group, uint8_t propIdx, uint8_t *buf, uint8_t bufLen)
    {
        uint8_t retVal = 1;

        uint8_t beforeList[bufLen] = { 0 };
        uint8_t afterList[bufLen] = { 0 };

        GetProperty(group, propIdx, beforeList, bufLen);
        SetProperty(group, propIdx, buf, bufLen);
        GetProperty(group, propIdx, afterList, bufLen);

        Log(P("group  : "), LogHEX(group));
        Log(P("propIdx: "), LogHEX(propIdx));
        Log(P("count  : "), bufLen);
        for (uint8_t i = 0; i < bufLen; ++i)
        {
            Log(LogHEX(beforeList[i]), P(" -> "), LogHEX(buf[i]), P(" -> "), LogHEX(afterList[i]));

            if (buf[i] != afterList[i])
            {
                retVal = 0;
            }
        }
        Log(P("OK: "), retVal);

        return retVal;
    }







    ///////////////////////////////////////////////////////////////////
    //
    // SPI / Module Electrical Primitives
    //
    ///////////////////////////////////////////////////////////////////


bool waitnIRQ()
{
	//return !digitalRead(_nIRQPin);		// inquire interrupt

    return !PAL.DigitalRead(pinIrq_);
}


uint8_t spiByte(uint8_t writeData)
{
	uint8_t readData;
	readData=SPI.transfer(writeData);
	return readData;
}
void spiWriteBuf(uint8_t writeLen,uint8_t* writeBuf)
{
    	while (writeLen--)
		spiByte(*writeBuf++);
}

void spiReadBuf(uint8_t readLen,uint8_t* readBuf)
{
    	while (readLen--)
		*readBuf++=spiByte(0);
}


bool checkCTS()
{
    uint16_t timeOutCnt;
	timeOutCnt=RF4463_CTS_TIMEOUT;
	while(timeOutCnt--)				// cts counter
	{
		PAL.DigitalWrite(pinChipSelect_, LOW);
		spiByte(RF4463_CMD_READ_BUF);	// send READ_CMD_BUFF command
		if (spiByte(0) == RF4463_CTS_REPLY)	// read CTS
		{
			PAL.DigitalWrite(pinChipSelect_, HIGH);
			return true;
		}
		PAL.DigitalWrite(pinChipSelect_, HIGH);
	}
	return	false;
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
        //SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
        PAL.DigitalWrite(pinChipSelect_, LOW);

        SPI.transfer(cmd);
        
        // Transfer as many command supplemental parameters as indicated
        // (could be zero based on input, which is valid)
        for (uint8_t i = 0; i < reqBufLen; ++i)
        {
            SPI.transfer(reqBuf[i]);
        }

        // PAL.DelayMicroseconds(DELAY_US_BEFORE_CHIP_SELECT_RELEASE);
        
        PAL.DigitalWrite(pinChipSelect_, HIGH);
        //SPI.endTransaction();
        
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
            
            //SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
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

            // PAL.DelayMicroseconds(DELAY_US_BEFORE_CHIP_SELECT_RELEASE);
            
            PAL.DigitalWrite(pinChipSelect_, HIGH);
            //SPI.endTransaction();
        }
        
        return retVal;
    }



    ///////////////////////////////////////////////////////////////////
    //
    // Unclassified
    //
    ///////////////////////////////////////////////////////////////////



    ///////////////////////////////////////////////////////////////////
    //
    // Legacy
    //
    ///////////////////////////////////////////////////////////////////
    
    void Init()
    {
        PAL.PinMode(pinIrq_, INPUT);

        PAL.PinMode(pinShutdown_, OUTPUT);
        PAL.DigitalWrite(pinShutdown_, HIGH);
        
        PAL.PinMode(pinChipSelect_, OUTPUT);
        PAL.DigitalWrite(pinChipSelect_, HIGH);
        
        SPI.begin();

        // depends on RF4463 spi timing
        SPI.setBitOrder(MSBFIRST);
        // 4463 top speed 10MHz, so even at 16MHz AVR clock, we come in at 8MHz.
        // Cannot div1, doesn't exist.
        SPI.setClockDivider(SPI_CLOCK_DIV2);
        SPI.setDataMode(SPI_MODE0);
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
    
    void SetModulationType(ModulationType modulationType)
    {
        modulationType_ = modulationType;
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
    }



    void StartDirectInput()
    {
        Start();

        SetModemTransmitOnDirectInput();
        
        ChangeStateToTx();
    }
    
    void Stop()
    {
        PAL.DigitalWrite(pinShutdown_, HIGH);
    }






    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Generated code
    //
    ///////////////////////////////////////////////////////////////////////////

    #include "RFSI4463PRO_Generated.h"
    
    

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
        
        // Modulation Type
        prop.BYTE0.MOD_TYPE = (uint8_t)modulationType_;
        
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
        // const uint8_t PROP_IDX1  = 0x02;
        // const uint8_t PROP_IDX2  = 0x03;
        
        uint32_t bigEndian = PAL.htonl(fcFrac);
        uint8_t *p = (uint8_t *)&bigEndian;

        uint8_t buf[] = {
            (uint8_t)(0b00001111 & p[1]),
            (uint8_t)(0b11111111 & p[2]),
            (uint8_t)(0b11111111 & p[3]),
        };

        SetProperty(PROP_GROUP, PROP_IDX0, buf, sizeof(buf));
        
        // SetProperty(PROP_GROUP, PROP_IDX0, (0b00001111 & p[1]));
        // SetProperty(PROP_GROUP, PROP_IDX1, (0b11111111 & p[2]));
        // SetProperty(PROP_GROUP, PROP_IDX2, (0b11111111 & p[3]));
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



private:

    uint8_t  pinIrq_;
    uint8_t  pinChipSelect_;
    uint8_t  pinShutdown_;
    uint32_t frequency_;
    uint32_t deviationFreq_;
    
    ModulationType modulationType_;
};



#endif  // __RF_SI4463PRO_H__