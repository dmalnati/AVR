#ifndef __RF_SI4463PRO_H__
#define __RF_SI4463PRO_H__


#include <SPI.h>

#include "PAL.h"
#include "BufferFieldExtractor.h"


// Some notes regarding implementation were taken from here:
// https://github.com/Yveaux/RadioHead/blob/master/RadioHead/RH_RF24.cpp
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
    static const uint8_t READ_ATTEMPT_LIMIT                  = 100;
    static const uint8_t DELAY_US_BEFORE_CHIP_SELECT_RELEASE = 50;
    
    // Details relating to operating this chip    
    static const uint32_t DURATION_MS_POWER_ON_RESET = 50;
    
    // Commands
    enum
    {
        CMD_READ_CMD_BUFF = 0x44,
    };
    
    // Various relevant values
    enum
    {
        VAL_TO_SEND_WHEN_READING = 0xFF,
        VAL_CLEAR_TO_SEND = 0xFF,
    };

    
public:
    RFSI4463PRO(uint8_t pinChipSelect, uint8_t pinShutdown)
    : pinChipSelect_(pinChipSelect)
    , pinShutdown_(pinShutdown)
    {
        // Nothing to do
    }
    
    void Init()
    {
        SPI.begin();
        
        PAL.PinMode(pinChipSelect_, OUTPUT);
        PAL.DigitalWrite(pinChipSelect_, HIGH);
        
        PowerOnReset();
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
        PAL.PinMode(pinShutdown_, OUTPUT);

        PAL.DigitalWrite(pinShutdown_, LOW);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
        
        PAL.DigitalWrite(pinShutdown_, HIGH);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
        
        PAL.DigitalWrite(pinShutdown_, LOW);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
    }
    
    uint8_t SendAndWaitAndReceive(uint8_t req, uint8_t *repBuf, uint8_t repBufLen)
    {
        uint8_t retVal = 0;
        
        //
        // Step 1 -- Issue command
        //
        SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
        PAL.DigitalWrite(pinChipSelect_, LOW);

        SPI.transfer(req);

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
        uint8_t readAttempts = 0;
        uint8_t rep          = !VAL_CLEAR_TO_SEND;
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
                for (uint16_t i = 0; i < repBufLen; ++i)
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

    uint8_t pinChipSelect_;
    uint8_t pinShutdown_;

};



#endif  // __RF_SI4463PRO_H__