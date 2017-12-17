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
    static const uint16_t READ_ATTEMPT_LIMIT                  = 1000;
    static const uint8_t  DELAY_US_BEFORE_CHIP_SELECT_RELEASE = 50;
    
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
        ClearInterrupts();
        PowerUp();
        DoPecanSteps();
    }
    
    

    
    void SendCmdReceiveAnswer(uint8_t cmdAndReqBufLen, uint8_t /*bufRecvBytes*/, const char *cmdAndReqBuf)
    {
        SendAndWaitAndReceive(cmdAndReqBuf[0],
                              (uint8_t *)&cmdAndReqBuf[1],
                              cmdAndReqBufLen - 1,
                              NULL,
                              0);
    }
    
#define VCXO_FREQ 27000000L
    void setFrequency(unsigned long freq)
    { 
      
      // Set the output divider according to recommended ranges given in Si446x datasheet  
      int outdiv = 4;
      int band = 0;
      if (freq < 705000000UL) { outdiv = 6;  band = 1;};
      if (freq < 525000000UL) { outdiv = 8;  band = 2;};
      if (freq < 353000000UL) { outdiv = 12; band = 3;};
      if (freq < 239000000UL) { outdiv = 16; band = 4;};
      if (freq < 177000000UL) { outdiv = 24; band = 5;};
      
     
      unsigned long f_pfd = 2 * VCXO_FREQ / outdiv;
      
      unsigned int n = ((unsigned int)(freq / f_pfd)) - 1;
      
      float ratio = (float)freq / (float)f_pfd;
      float rest  = ratio - (float)n;
      

      unsigned long m = (unsigned long)(rest * 524288UL); 
     


    // set the band parameter
      unsigned int sy_sel = 8; // 
      char set_band_property_command[] = {0x11, 0x20, 0x01, 0x51, (band + sy_sel)}; //   
      // send parameters
      SendCmdReceiveAnswer(5, 1, set_band_property_command);

    // Set the pll parameters
      unsigned int m2 = m / 0x10000;
      unsigned int m1 = (m - m2 * 0x10000) / 0x100;
      unsigned int m0 = (m - m2 * 0x10000 - m1 * 0x100); 
      // assemble parameter string
      char set_frequency_property_command[] = {0x11, 0x40, 0x04, 0x00, n, m2, m1, m0};
      // send parameters
      SendCmdReceiveAnswer(8, 1, set_frequency_property_command);
      

    }



    void reset()
    {
        Init();
    }
    
void DoPecanSteps()
{
    //  Set all GPIOs LOW; Link NIRQ to CTS; Link SDO to MISO; Max drive strength
    const char gpio_pin_cfg_command[] = {0x13, 0x02, 0x02, 0x02, 0x02, 0x08, /*0x11*/ 11, 0x00}; 
    SendCmdReceiveAnswer(8, 8, gpio_pin_cfg_command);
    // SendAndWaitAndReceive(gpio_pin_cfg_command[0],
                          // (uint8_t *)&gpio_pin_cfg_command[1],
                          // sizeof(gpio_pin_cfg_command) - 1,
                          // NULL,
                          // 0);

    setFrequency(144390000UL);
    setModem(); 
    tune_tx();
}
    
void setModem()
{
  // Set to CW mode
//  Serial.println("Setting modem into CW mode");  
  char set_modem_mod_type_command[] = {0x11, 0x20, 0x01, 0x00, 0x00};
  SendCmdReceiveAnswer(5, 1, set_modem_mod_type_command);
  
}

void tune_tx()
{
  char change_state_command[] = {0x34, 0x05}; //  Change to TX tune state
  SendCmdReceiveAnswer(2, 1, change_state_command);

}

void ptt_on()
{
  
  // digitalWrite(VCXO_ENABLE_PIN, HIGH);
  reset();
  // turn on the blue LED (GPIO2) to indicate TX
  char gpio_pin_cfg_command2[] = {0x13, 0x02, 0x02, 0x03, 0x02, 0x08, /*0x11*/11, 0x00}; //  Set GPIO2 HIGH; Link NIRQ to CTS; Link SDO to MISO; Max drive strength
  SendCmdReceiveAnswer(8, 1, gpio_pin_cfg_command2);

  start_tx();
  // si446x_powerlevel = 1023;
}

void ptt_off()
{
  stop_tx();
  // si446x_powerlevel = 0;
  // turn off the blue LED (GPIO2)
  char gpio_pin_cfg_command0[] = {0x13, 0x02, 0x02, 0x02, 0x02, 0x08, /*0x11*/11, 0x00}; //  Set all GPIOs LOW; Link NIRQ to CTS; Link SDO to MISO; Max drive strength
  SendCmdReceiveAnswer(8, 1, gpio_pin_cfg_command0);

  // digitalWrite(RADIO_SDN_PIN, HIGH);  // active high = shutdown
  //digitalWrite(VCXO_ENABLE_PIN, LOW); //keep enabled for now

}

void start_tx()
{
  char change_state_command[] = {0x34, 0x07}; //  Change to TX state
  SendCmdReceiveAnswer(2, 1, change_state_command);

}

void stop_tx()
{
  char change_state_command[] = {0x34, 0x03}; //  Change to Ready state
  SendCmdReceiveAnswer(2, 1, change_state_command);

}

    
    uint8_t SetProperty(uint16_t property, uint8_t value)
    {
        RFSI4463PRO::SET_PROPERTY_REQ req;
        
        req.GROUP.GROUP           = (uint8_t)((property & 0xFF00) >> 8);
        req.NUM_PROPS.NUM_PROPS   = 1;
        req.START_PROP.START_PROP = (uint8_t)(property & 0x00FF);
        req.DATA0.DATA0           = value;
        
        uint8_t retVal = Command_SET_PROPERTY(req);
        
        return retVal;
    }
    
    uint8_t GetProperty(uint16_t property, uint8_t &value)
    {
        RFSI4463PRO::GET_PROPERTY_REQ req;
        RFSI4463PRO::GET_PROPERTY_REP rep;
        
        req.GROUP.GROUP           = (uint8_t)((property & 0xFF00) >> 8);
        req.NUM_PROPS.NUM_PROPS   = 1;
        req.START_PROP.START_PROP = (uint8_t)(property & 0x00FF);
        
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
        PAL.PinMode(pinShutdown_, OUTPUT);

        PAL.DigitalWrite(pinShutdown_, LOW);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
        
        PAL.DigitalWrite(pinShutdown_, HIGH);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
        
        PAL.DigitalWrite(pinShutdown_, LOW);
        PAL.Delay(DURATION_MS_POWER_ON_RESET);
    }
    
    uint8_t ClearInterrupts()
    {
        RFSI4463PRO::GET_INT_STATUS_REQ req;
        RFSI4463PRO::GET_INT_STATUS_REP rep;
        
        uint8_t retVal = Command_GET_INT_STATUS(req, rep);
        
        return retVal;
    }
    
    uint8_t PowerUp()
    {
        RFSI4463PRO::POWER_UP_REQ req;
        
        req.BOOT_OPTIONS.FUNC = 1;
        req.XTAL_OPTIONS.TCXO = 1;
        req.XO_FREQ.XO_FREQ   = 27000000L;
        
        uint8_t retVal = Command_POWER_UP(req);
        
        return retVal;
    }
    
    uint8_t SendAndWaitAndReceive(uint8_t  cmd,
                                  uint8_t *reqBuf,
                                  uint8_t  reqBufLen,
                                  uint8_t *repBuf,
                                  uint8_t  repBufLen)
    {
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

    uint8_t pinChipSelect_;
    uint8_t pinShutdown_;

};



#endif  // __RF_SI4463PRO_H__