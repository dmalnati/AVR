#include <SPI.h>

#include "PAL.h"


static const uint32_t SPI_SPEED           = 10000;
static const uint8_t  SPI_BIT_ORIENTATION = MSBFIRST;
static const uint8_t  SPI_MODE            = SPI_MODE0;

static const uint8_t PIN_SS = 15;

static const uint8_t PIN_SHUTDOWN = 14;


void PowerOnReset()
{
    PAL.DigitalWrite(PIN_SHUTDOWN, HIGH);
    PAL.PinMode(PIN_SHUTDOWN, OUTPUT);
    PAL.Delay(10);
    PAL.DigitalWrite(PIN_SHUTDOWN, LOW);
    PAL.Delay(10);
}








void SendAndWaitAndReceive(uint8_t req, uint8_t *repBuf, uint8_t repBufLen)
{
    SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
    PAL.DigitalWrite(PIN_SS, LOW);

    // Send req, then send READ_CMD_BUFF command, then clock in CTS
    SPI.transfer(req);
    SPI.transfer(0x44);
    uint8_t rep = SPI.transfer(0x00);

    // Wait for CTS (clear to send) to equal 0xFF
    while (rep != 0xFF)
    {
        // Just send zeros, it's not real data, we're just driving the
        // clock in order to extract a response from the slave
        rep = SPI.transfer(0x00);
    }

    // Consume as many bytes of response as are expected, could be zero
    for (uint16_t i = 0; i < repBufLen; ++i)
    {
        repBuf[i] = SPI.transfer(0x00);
    }
    
    PAL.DigitalWrite(PIN_SS, HIGH);
    SPI.endTransaction();
}


void SendAndWaitAndReceiveNew(uint8_t req, uint8_t *repBuf, uint8_t repBufLen)
{
    SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
    PAL.DigitalWrite(PIN_SS, LOW);

    // Send req, then send READ_CMD_BUFF command, then clock in CTS
    SPI.transfer(req);

    PAL.Delay(1);
    PAL.DigitalWrite(PIN_SS, HIGH);
    SPI.endTransaction();

    
    
    uint8_t rep = 0x00; // temp value to make sure first loop iterates


    // Wait for CTS (clear to send) to equal 0xFF
    while (rep != 0xFF)
    {
        SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
        PAL.DigitalWrite(PIN_SS, LOW);

        SPI.transfer(0x44);
        rep = SPI.transfer(0x00);

        if (rep == 0xFF)
        {
            // Consume as many bytes of response as are expected, could be zero
            for (uint16_t i = 0; i < repBufLen; ++i)
            {
                repBuf[i] = SPI.transfer(0x00);
            }
        }

        PAL.Delay(1);
        PAL.DigitalWrite(PIN_SS, HIGH);
        SPI.endTransaction();
    }
}


void DoReqRep(const char *op, uint8_t req, uint8_t repBufLen)
{
    uint8_t repBuf[repBufLen];
    
    Serial.print(op);
    Serial.print(": ");
    Serial.print(req, HEX);
    Serial.print(" -> [");
    Serial.print(repBufLen);
    Serial.print("] ");

    SendAndWaitAndReceive(req, repBuf, repBufLen);

    for (uint16_t i = 0; i < repBufLen; ++i)
    {
        Serial.print(repBuf[i], HEX);
    }
    
    Serial.println();
}

void DoNOP()
{
    DoReqRep("NOP", 0x00, 0);
}

void DoPART_INFO()
{
    DoReqRep("PART_INFO", 0x01, 8);
}

void DoFUNC_INFO()
{
    DoReqRep("FUNC_INFO", 0x10, 6);
}


void DoGET_PH_STATUS()
{
    DoReqRep("GET_PH_STATUS", 0x21, 2);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    SPI.begin();

    PowerOnReset();

    PAL.PinMode(PIN_SS, OUTPUT);
    PAL.DigitalWrite(PIN_SS, HIGH);

    while (1)
    {
        Serial.println();
        Serial.println("Loop start");

        DoNOP();
        DoPART_INFO();
        DoFUNC_INFO();
        DoGET_PH_STATUS();

        PAL.Delay(1500);
    }
    
}


void loop() {}




