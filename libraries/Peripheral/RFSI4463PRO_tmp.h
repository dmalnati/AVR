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



    ///////////////////////////////////////////////////////////////////
    //
    // Shims
    //
    ///////////////////////////////////////////////////////////////////

bool setCommand(uint8_t length,uint8_t command,uint8_t* paraBuf)
{
	uint8_t retVal = 0;

	if (setCommandStart())
	{
		retVal = 1;

		setCommandWriteCommand(command);
		setCommandWriteBuffer(length, paraBuf);
		setCommandEnd();
	}

	return retVal;
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



bool setProperties(uint16_t startProperty, uint8_t length ,uint8_t* paraBuf)
{
	uint8_t buf[4];

	if(!checkCTS())
		return false;

	buf[0] = RF4463_CMD_SET_PROPERTY;
	buf[1] = startProperty >> 8;   		// GROUP
	buf[2] = length;                	// NUM_PROPS
	buf[3] = startProperty & 0xff; 		// START_PROP

	PAL.DigitalWrite(pinChipSelect_, LOW);
	spiWriteBuf(4,buf);					// set start property and read length
	spiWriteBuf(length,paraBuf);		// set parameters
	PAL.DigitalWrite(pinChipSelect_, HIGH);
	
	return true;
}
    bool setSyncWords(uint8_t * syncWords,uint8_t len)
    {
        if((len==0)||(len>3))
            return false;
        uint8_t buf[5];
        buf[0]=len-1;
        memcpy(buf+1,syncWords,len);
        return setProperties(RF4463_PROPERTY_SYNC_CONFIG,sizeof(buf),buf);
    }

    bool setTxPower(uint8_t power)
    {
        if(power>127)	// max is 127
            return false;

        uint8_t buf[4]={0x08,0x00,0x00,0x3d};
        buf[1]=power;

        return setProperties(RF4463_PROPERTY_PA_MODE,sizeof(buf),buf);

    }


void spiReadBuf(uint8_t readLen,uint8_t* readBuf)
{
    	while (readLen--)
		*readBuf++=spiByte(0);
}


bool getCommand(uint8_t length,uint8_t command,uint8_t* paraBuf)
{
	if(!checkCTS())
		return false;

    PAL.DigitalWrite(pinChipSelect_, LOW);
	spiByte(command);				// set command to read 
    PAL.DigitalWrite(pinChipSelect_, HIGH);
	
	if(!checkCTS())					// check if RF4463 is ready
		return false;

    PAL.DigitalWrite(pinChipSelect_, LOW);
	spiByte(RF4463_CMD_READ_BUF);	// turn to read command mode
	spiReadBuf(length,paraBuf);		// read parameters
    PAL.DigitalWrite(pinChipSelect_, HIGH);

	return true;	
}








    ///////////////////////////////////////////////////////////////////
    //
    // Duplicating RF4463 code
    //
    ///////////////////////////////////////////////////////////////////


void spiInit()
{
	SPI.begin();
	//init slave select pin
    PAL.PinMode(pinChipSelect_, OUTPUT);
    PAL.DigitalWrite(pinChipSelect_, HIGH);

	// depends on RF4463 spi timing
	SPI.setBitOrder(MSBFIRST);
	// 4463 top speed 10MHz, so even at 16MHz AVR clock, we come in at 8MHz.
	// Cannot div1, doesn't exist.
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI.setDataMode(SPI_MODE0);
}
void pinInit()
{
    PAL.PinMode(pinShutdown_, OUTPUT);
    PAL.DigitalWrite(pinShutdown_, HIGH);
    PAL.PinMode(pinIrq_, INPUT);
}


bool init()
{
	pinInit();
	spiInit();


	uint8_t buf[20];

	// reset RF4463
	powerOnReset();
	// Set RF parameter,like frequency,data rate etc
    static const uint8_t RF4463_CONFIGURATION_DATA[] = RADIO_CONFIGURATION_DATA_ARRAY;

	setConfig(RF4463_CONFIGURATION_DATA,sizeof(RF4463_CONFIGURATION_DATA));
	
	// set antenna switch,in RF4463 is GPIO2 and GPIO3
	// don't change setting of GPIO2,GPIO3,NIRQ,SDO
	buf[0]  = RF4463_GPIO_NO_CHANGE; 
	buf[1]  = RF4463_GPIO_NO_CHANGE;
	buf[2]  = RF4463_GPIO_RX_STATE;
	buf[3]  = RF4463_GPIO_TX_STATE;
	buf[4]  = RF4463_NIRQ_INTERRUPT_SIGNAL; 
	buf[5]  = RF4463_GPIO_SPI_DATA_OUT; 
	setCommand(6,RF4463_CMD_GPIO_PIN_CFG,buf);
	
	// frequency adjust
	// frequency will inaccurate if change this parameter
	buf[0]  = 98;
	setProperties(RF4463_PROPERTY_GLOBAL_XO_TUNE,1,buf);

	// tx = rx = 64 byte,PH mode ,high performance mode
	buf[0] = 0x40;
	setProperties(RF4463_PROPERTY_GLOBAL_CONFIG,1,buf);

	// set preamble
	buf[0]  = 0x08;		//  8 bytes Preamble			
	buf[1]  = 0x14;		//  detect 20 bits
	buf[2]  = 0x00;						
	buf[3]  = 0x0f;
	buf[4]  = RF4463_PREAMBLE_FIRST_1|RF4463_PREAMBLE_LENGTH_BYTES|RF4463_PREAMBLE_STANDARD_1010;
	buf[5]  = 0x00;
	buf[6]  = 0x00;
	buf[7]  = 0x00;
	buf[8]  = 0x00;
	setProperties(RF4463_PROPERTY_PREAMBLE_TX_LENGTH,9,buf);

	// set sync words
	buf[0] = 0x2d;
	buf[1] = 0xd4;
	setSyncWords(buf,2);
   
    // set CRC
	buf[0] = RF4463_CRC_SEED_ALL_1S|RF4463_CRC_ITU_T ;			
	setProperties(RF4463_PROPERTY_PKT_CRC_CONFIG,1,buf);
	
	buf[0]=RF4463_CRC_ENDIAN;
	setProperties(RF4463_PROPERTY_PKT_CONFIG1,1,buf);

	buf[0]=RF4463_IN_FIFO|RF4463_DST_FIELD_ENUM_2;
	buf[1]=RF4463_SRC_FIELD_ENUM_1;
	buf[2]=0x00;
	setProperties(RF4463_PROPERTY_PKT_LEN,3,buf);
	
	// set length of Field 1 -- 4
	// variable len,field as length field,field 2 as data field
	// didn't use field 3 -- 4
	buf[0] = 0x00;	// PKT_FIELD_1_LENGTH_2
	buf[1] = 0x01;	// PKT_FIELD_1_LENGTH_1
	
	// PKT_FIELD_1_CONFIG
	buf[2] = RF4463_FIELD_CONFIG_PN_START;

	//PKT_FIELD_1_CRC_CONFIG
	buf[3] = RF4463_FIELD_CONFIG_CRC_START|RF4463_FIELD_CONFIG_SEND_CRC|RF4463_FIELD_CONFIG_CHECK_CRC|RF4463_FIELD_CONFIG_CRC_ENABLE;
	
	// PKT_FIELD_2_LENGTH
	buf[4] = 0x00;
	buf[5] = 50;	// -- bingo, that's the 50 char limit there

	// PKT_FIELD_2_CONFIG -- this doesn't even make sense for field 2, PN doesn't apply here
	buf[6] = RF4463_FIELD_CONFIG_PN_START;
	
	// PKT_FIELD_2_CRC_CONFIG
	buf[7] = RF4463_FIELD_CONFIG_CRC_START|RF4463_FIELD_CONFIG_SEND_CRC|RF4463_FIELD_CONFIG_CHECK_CRC|RF4463_FIELD_CONFIG_CRC_ENABLE;;
	
	// 
	buf[8]  = 0x00; 
	buf[9] = 0x00;
	buf[10] = 0x00;
	buf[11] = 0x00;
	setProperties(RF4463_PROPERTY_PKT_FIELD_1_LENGTH_12_8 ,12,buf);
  
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x00;
	buf[6] = 0x00;
	buf[7] = 0x00;
	setProperties(RF4463_PROPERTY_PKT_FIELD_4_LENGTH_12_8,8,buf);

	// set max tx power
    setTxPower(127);

	// check if RF4463 works
	if(!checkDevice())
	{
		return false;
	}
	
    return true;
}
void powerOnReset()
{
    uint8_t buf[]={RF_POWER_UP};

    PAL.DigitalWrite(pinShutdown_, HIGH);
    delay(100);
    PAL.DigitalWrite(pinShutdown_, LOW);
    delay(20);						// wait for RF4463 stable

	// send power up command
    PAL.DigitalWrite(pinChipSelect_, LOW);
	spiWriteBuf(sizeof(buf),buf);
	PAL.DigitalWrite(pinChipSelect_, HIGH);
	
	delay(200);
}
void setConfig(const uint8_t* parameters,uint16_t paraLen)
{
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
		
		setCommand(cmdLen,command,buf);
		pos=pos+cmdLen;
	}
}
bool checkDevice()
{
	uint8_t buf[9];
	uint16_t partInfo;
	if(!getCommand(9,RF4463_CMD_PART_INFO,buf))		// read part info to check if 4463 works
		return false;
		
	partInfo=buf[2]<<8|buf[3];
	if(partInfo!=0x4463)
	{
		return false;
	}

	return true;
}








    uint8_t initOld()
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

    uint8_t PowerUp2()
    {
        RFSI4463PRO::POWER_UP_REQ req;
        
        req.BOOT_OPTIONS.FUNC = 1;
        //req.XTAL_OPTIONS.TCXO = 1;
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
    	uint8_t buf[20];

        // frequency adjust
        // frequency will inaccurate if change this parameter
        buf[0]  = 98;
        setProperties(RF4463_PROPERTY_GLOBAL_XO_TUNE,1,buf);

        // tx = rx = 64 byte,PH mode ,high performance mode
        buf[0] = 0x40;
        setProperties(RF4463_PROPERTY_GLOBAL_CONFIG,1,buf);

        // set preamble
        buf[0]  = 0x08;		//  8 bytes Preamble			
        buf[1]  = 0x14;		//  detect 20 bits
        buf[2]  = 0x00;						
        buf[3]  = 0x0f;
        buf[4]  = RF4463_PREAMBLE_FIRST_1|RF4463_PREAMBLE_LENGTH_BYTES|RF4463_PREAMBLE_STANDARD_1010;
        buf[5]  = 0x00;
        buf[6]  = 0x00;
        buf[7]  = 0x00;
        buf[8]  = 0x00;
        setProperties(RF4463_PROPERTY_PREAMBLE_TX_LENGTH,9,buf);

        // set sync words
        buf[0] = 0x2d;
        buf[1] = 0xd4;
        setSyncWords(buf,2);

        // set CRC
        buf[0] = RF4463_CRC_SEED_ALL_1S|RF4463_CRC_ITU_T ;			
        setProperties(RF4463_PROPERTY_PKT_CRC_CONFIG,1,buf);
        
        buf[0]=RF4463_CRC_ENDIAN;
        setProperties(RF4463_PROPERTY_PKT_CONFIG1,1,buf);

        buf[0]=RF4463_IN_FIFO|RF4463_DST_FIELD_ENUM_2;
        buf[1]=RF4463_SRC_FIELD_ENUM_1;
        buf[2]=0x00;
        setProperties(RF4463_PROPERTY_PKT_LEN,3,buf);
        
        // set length of Field 1 -- 4
        // variable len,field as length field,field 2 as data field
        // didn't use field 3 -- 4
        buf[0] = 0x00;	// PKT_FIELD_1_LENGTH_2
        buf[1] = 0x01;	// PKT_FIELD_1_LENGTH_1
        
        // PKT_FIELD_1_CONFIG
        buf[2] = RF4463_FIELD_CONFIG_PN_START;

        //PKT_FIELD_1_CRC_CONFIG
        buf[3] = RF4463_FIELD_CONFIG_CRC_START|RF4463_FIELD_CONFIG_SEND_CRC|RF4463_FIELD_CONFIG_CHECK_CRC|RF4463_FIELD_CONFIG_CRC_ENABLE;
        
        // PKT_FIELD_2_LENGTH
        buf[4] = 0x00;
        buf[5] = 50;	// -- bingo, that's the 50 char limit there

        // PKT_FIELD_2_CONFIG -- this doesn't even make sense for field 2, PN doesn't apply here
        buf[6] = RF4463_FIELD_CONFIG_PN_START;
        
        // PKT_FIELD_2_CRC_CONFIG
        buf[7] = RF4463_FIELD_CONFIG_CRC_START|RF4463_FIELD_CONFIG_SEND_CRC|RF4463_FIELD_CONFIG_CHECK_CRC|RF4463_FIELD_CONFIG_CRC_ENABLE;;
        
        // 
        buf[8]  = 0x00; 
        buf[9] = 0x00;
        buf[10] = 0x00;
        buf[11] = 0x00;
        setProperties(RF4463_PROPERTY_PKT_FIELD_1_LENGTH_12_8 ,12,buf);

        buf[0] = 0x00;
        buf[1] = 0x00;
        buf[2] = 0x00;
        buf[3] = 0x00;
        buf[4] = 0x00;
        buf[5] = 0x00;
        buf[6] = 0x00;
        buf[7] = 0x00;
        setProperties(RF4463_PROPERTY_PKT_FIELD_4_LENGTH_12_8,8,buf);

        // set max tx power
        setTxPower(127);
    }




    bool enterStandbyMode()
    {
        uint8_t data=0x01;
        return setCommand(1,RF4463_CMD_CHANGE_STATE,&data);
    }


bool rxInit()
{		
	uint8_t length;
	length=50;
	setProperties(RF4463_PROPERTY_PKT_FIELD_2_LENGTH_7_0, sizeof(length),&length);	// reload rx fifo size
	fifoReset();				// clr fifo
	setRxInterrupt();
	clrInterrupts();			// clr int factor	
	enterRxMode();				// enter RX mode
	return true;
}

bool setRxInterrupt()
{
	uint8_t buf[3]={0x03,0x18,0x00};			// enable PACKET_RX interrupt
	return setProperties(RF4463_PROPERTY_INT_CTL_ENABLE, 3,buf);
}

void enterRxMode()
{
	uint8_t buf[]={0x00,0x00,0x00,0x00,0x00,0x08,0x08};
	buf[0]=RF4463_FREQ_CHANNEL;
	setCommand(7,RF4463_CMD_START_RX ,buf);
}

bool txPacket(uint8_t* sendbuf,uint8_t sendLen, uint8_t syncSend)
{
	uint8_t retVal = 0;

	uint16_t txTimer;

	fifoReset();		 				// clr fifo
	writeTxFifo(sendbuf,sendLen);		// load data to fifo	
	setTxInterrupt();
	clrInterrupts();					// clr int factor	
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


void fifoReset()
{
	uint8_t data=0x03;
	setCommand(sizeof(data),RF4463_CMD_FIFO_INFO,&data);
}

void writeTxFifoOld(uint8_t* databuf,uint8_t length)
{
	setProperties(RF4463_PROPERTY_PKT_FIELD_2_LENGTH_7_0, sizeof(length),&length);

	setCommandStart();
	setCommandWriteCommand(RF4463_CMD_TX_FIFO_WRITE);
	setCommandWriteBuffer(1, &length);
	setCommandWriteBuffer(length, databuf);
	setCommandEnd();
}

void writeTxFifo(uint8_t* databuf,uint8_t length)
{
	setProperties(RF4463_PROPERTY_PKT_FIELD_2_LENGTH_7_0, sizeof(length),&length);

    uint8_t buf[length + 1];
    buf[0] = length;
    memcpy(&buf[1], databuf, length);

    setCommand(length + 1, RF4463_CMD_TX_FIFO_WRITE, buf);
}

bool setTxInterrupt()
{
	uint8_t buf[3]={0x01,0x20,0x00};			// enable PACKET_SENT interrupt
	return setProperties(RF4463_PROPERTY_INT_CTL_ENABLE, 3,buf);
}
bool clrInterrupts()
{
    uint8_t buf[] = { 0x00, 0x00, 0x00 };  
    return setCommand(sizeof(buf),RF4463_CMD_GET_INT_STATUS,buf);
}

void enterTxMode()
{
	uint8_t buf[]={0x00,0x30,0x00,0x00};
	buf[0]=RF4463_FREQ_CHANNEL;
	setCommand(4,RF4463_CMD_START_TX ,buf);
}


bool waitnIRQ()
{
	//return !digitalRead(_nIRQPin);		// inquire interrupt

    return !PAL.DigitalRead(pinIrq_);
}




uint8_t rxPacket(uint8_t *recvbuf)
{
	uint8_t rxLen;
	rxLen=ReadRxFifo(recvbuf);			// read data from fifo
	fifoReset();						// clr fifo

	return rxLen;
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

    void StartPacketHandler()
    {
        // # Start
        // RESET
                PowerOnReset();

        // 'POWER_UP' 01 00 01 C9 C3 80
                PowerUp();

        // 'PART_INFO'
        // 'FUNC_INFO'

        // # Adjust Crystal Osc cap bank to center oscillator frequency
        // 'SET_PROPERTY' 'GLOBAL_XO_TUNE' 4B
        // DM -- no, default is fine

        // # Set interrupts = Packet Sent
        // 'SET_PROPERTY' 'INT_CTL_ENABLE' 05

        {
            uint8_t buf[] = { 0x05 };

            const uint8_t GROUP = 0x01;
            const uint8_t INDEX = 0x00;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }



        // 'SET_PROPERTY' 'INT_CTL_PH_ENABLE' 20

        // equal, enable packet sent interrupt
        {
            uint8_t buf[] = { 0x20 };

            const uint8_t GROUP = 0x01;
            const uint8_t INDEX = 0x01;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }


        // # Read and clear any existing interrupts
        // 'GET_INT_STATUS' 00 00 00

        ClearInterrupts();



        // # General parameters, Mod Type = 2GFSK, Packet FIFO
        // 'SET_PROPERTY' 'MODEM_MOD_TYPE' 03

        // equal, set to 2GFSK, packet FIFO is default
        {
            MODEM_MOD_TYPE_PROP prop;

            prop.BYTE0.MOD_TYPE = 3;

            SetProperty(prop);
        }


        // 'SET_PROPERTY' 'MODEM_MAP_CONTROL' 00

        // equal, disable manchester encoding (defaulted to on)
        {
            MODEM_MAP_CONTROL_PROP prop;

            SetProperty(prop);
        }



        // 'SET_PROPERTY' 'MODEM_CLKGEN_BAND' 08
        // DM -- this is the default already



        // # Freq control group = 915.0 MHz
        // 'SET_PROPERTY' 'FREQ_CONTROL_INTE' 3C
        // 'SET_PROPERTY' 'FREQ_CONTROL_FRAC_2' 08
        // 'SET_PROPERTY' 'FREQ_CONTROL_FRAC_1' 00
        // 'SET_PROPERTY' 'FREQ_CONTROL_FRAC_0' 00

        {
            uint8_t buf[] = { 0x3C, 0x08, 0x00, 0x00 };

            const uint8_t GROUP = 0x40;
            const uint8_t INDEX = 0x00;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }


        // 'SET_PROPERTY' 'FREQ_CONTROL_W_SIZE' 20
        // DM -- this is the default already


        // # PA control group = Class-E, +20 dBm
        // AN626
        // Rev. 0.2 27


        // 'SET_PROPERTY' 'PA_MODE' 08
        // DM -- this is the default already


        // 'SET_PROPERTY' 'PA_PWR_LVL' 7F
        // DM -- this is the default already


        // 'SET_PROPERTY' 'PA_BIAS_CLKDUTY' 00
        // DM -- this is the default already


        // 'SET_PROPERTY' 'PA_TC' 3D

        // 14us (default) to 6us delay of rising edge of PA ramp for (G)FSK
        // not sure why changing this...
        {
            PA_TC_PROP prop;

            prop.BYTE0.FSK_MOD_DLY = 3;
            prop.BYTE0.TC          = 13;    // 0x0D

            SetProperty(prop);
        }


        // # Tx parameters, DR=5kbps, Dev=1.625kHz, TXOSR=40
        // 'SET_PROPERTY' 'MODEM_DATA_RATE_2' 00
        // 'SET_PROPERTY' 'MODEM_DATA_RATE_1' 4E
        // 'SET_PROPERTY' 'MODEM_DATA_RATE_0' 20

        {
            MODEM_DATA_RATE_PROP prop;

            prop.BYTE0.DATA_RATE = 0x00;
            prop.BYTE1.DATA_RATE = 0x4E;
            prop.BYTE2.DATA_RATE = 0x20;

            SetProperty(prop);
        }


        // 'SET_PROPERTY' 'MODEM_TX_NCO_MODE_3' 04
        // 'SET_PROPERTY' 'MODEM_TX_NCO_MODE_2' 2D
        // 'SET_PROPERTY' 'MODEM_TX_NCO_MODE_1' C6
        // 'SET_PROPERTY' 'MODEM_TX_NCO_MODE_0' C0

        {
            uint8_t buf[] = { 0x04, 0x2D, 0xC6, 0xC0 };

            const uint8_t GROUP = 0x20;
            const uint8_t INDEX = 0x06;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }


        // 'SET_PROPERTY' 'MODEM_FREQ_DEV_2' 00
        // 'SET_PROPERTY' 'MODEM_FREQ_DEV_1' 00
        // 'SET_PROPERTY' 'MODEM_FREQ_DEV_0' 39

        {
            uint8_t buf[] = { 0x00, 0x00, 0x39 };

            const uint8_t GROUP = 0x20;
            const uint8_t INDEX = 0x0A;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }

        // 'SET_PROPERTY' 'MODEM_TX_RAMP_DELAY' 01
        // DM -- this is the default already



        // # Set Preamble = Std '0101, Length = 5 bytes
        // 'SET_PROPERTY' 'PREAMBLE_TX_LENGTH' 05

        // Not sure why changing 8 (default) to 5
        {
            uint8_t buf[] = { 0x05 };

            const uint8_t GROUP = 0x10;
            const uint8_t INDEX = 0x00;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }

        // 'SET_PROPERTY' 'PREAMBLE_CONFIG' 12
        // changing default 0x21 to 0x12
        {
            uint8_t buf[] = { 0x21 };

            const uint8_t GROUP = 0x10;
            const uint8_t INDEX = 0x04;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }


        // # Set Sync word = 2 bytes = 0xB42B (sent little-endian = 0x2DD4 over air interface)
        // 'SET_PROPERTY' 'SYNC_CONFIG' 01
        // DM -- this is the default already

        // 'SET_PROPERTY' 'SYNC_BITS_31_24' B4
        // 'SET_PROPERTY' 'SYNC_BITS_23_16' 2B
        {
            uint8_t buf[] = { 0xB4, 0x2B };

            const uint8_t GROUP = 0x11;
            const uint8_t INDEX = 0x01;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }


        // # Enable CRC, set the X15+X12+X5+1 16-bit polynomial, seed value = 1's
        // 'SET_PROPERTY' 'PKT_CRC_CONFIG' 85
        {
            uint8_t buf[] = { 0x85 };

            const uint8_t GROUP = 0x12;
            const uint8_t INDEX = 0x00;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }

        // # Set fixed packet length, Field-1 = 10 bytes, CRC across Field-1, load seed value
        // 'SET_PROPERTY' 'PKT_LEN' 00
        // DM -- this is the default already

        // 'SET_PROPERTY' 'PKT_FIELD_1_LENGTH_12_8' 00
        // 'SET_PROPERTY' 'PKT_FIELD_1_LENGTH_7_0' 0A

        {
            uint8_t buf[] = { 0x00, 0x0A };

            const uint8_t GROUP = 0x12;
            const uint8_t INDEX = 0x0D;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }


        // 'SET_PROPERTY' 'PKT_FIELD_1_CONFIG' 00
        // DM -- this is the default already

        // 'SET_PROPERTY' 'PKT_FIELD_1_CRC_CONFIG' A2
        {
            uint8_t buf[] = { 0xA2 };

            const uint8_t GROUP = 0x12;
            const uint8_t INDEX = 0x10;
            SetProperty(GROUP, INDEX, buf, sizeof(buf));
        }
    }

    uint8_t SendPH()
    {
        uint8_t retVal = 0;

        // # Load 10-bytes of arbitrary data into TX FIFO
        // 'WRITE_TX_FIFO' 01 02 03 04 05 06 07 08 09 0A

        {
            uint8_t buf[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A };

            const uint8_t CMD_ID = 0x66;
            retVal = SendCommand(CMD_ID, buf, sizeof(buf));
        }

        // # Start transmitting, return to READY after packet
        // AN626
        // 28 Rev. 0.2
        // 'START_TX' 00 30 00 00

        {
            ChangeStateToTx();
        }

        return retVal;
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

    uint8_t SetProperty(uint8_t propGroup, uint8_t propIdx, uint8_t *buf, uint8_t bufLen)
    {
        uint8_t bufReqLen = bufLen + 3;
        uint8_t bufReq[bufReqLen];

        bufReq[0] = propGroup;
        bufReq[1] = bufLen;
        bufReq[2] = propIdx;

        memcpy(&bufReq[3], buf, bufLen);

        //uint8_t ok = SendAndWaitAndReceive(0x11, bufReq, bufReqLen, nullptr, 0);
        uint8_t ok = SendCommand(0x11, bufReq, bufReqLen);

        return ok;
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



    
    // uint8_t SetProperty(uint8_t propGroup, uint8_t propIdx, uint8_t value)
    // {
    //     RFSI4463PRO::SET_PROPERTY_REQ req;
        
    //     req.GROUP.GROUP           = propGroup;
    //     req.NUM_PROPS.NUM_PROPS   = 1;
    //     req.START_PROP.START_PROP = propIdx;
    //     req.DATA0.DATA0           = value;
        
    //     uint8_t retVal = Command_SET_PROPERTY(req);
        
    //     return retVal;
    // }
    
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

    
    uint8_t ClearInterrupts()
    {
        RFSI4463PRO::GET_INT_STATUS_REQ req;
        RFSI4463PRO::GET_INT_STATUS_REP rep;

        RFSI4463PRODebug::Print(rep);
        
        uint8_t retVal = Command_GET_INT_STATUS(req, rep);
        
        return retVal;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Generated code
    //
    ///////////////////////////////////////////////////////////////////////////

    #include "RFSI4463PRO_Generated.h"
    #include "RFSI4463PRODebug.h"
    
    
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

private:

    uint8_t  pinIrq_;
    uint8_t  pinChipSelect_;
    uint8_t  pinShutdown_;
    uint32_t frequency_;
    uint32_t deviationFreq_;
    
    ModulationType modulationType_;
};



#endif  // __RF_SI4463PRO_H__