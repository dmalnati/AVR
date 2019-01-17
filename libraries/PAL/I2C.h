#ifndef __TWI_CLASS_H__
#define __TWI_CLASS_H__


#include <inttypes.h>

#include <util/twi.h>

#include "PAL.h"


// Works in Master Transmitter and Master Receiver modes
class TWIClass
{
    // Master Transmitter Status Codes
    static const uint8_t MT_STATUS_START_TRANSMITTED              = 0x08;
    static const uint8_t MT_STATUS_REPEATED_START_TRANSMITTED     = 0x10;
    static const uint8_t MT_STATUS_SLA_W_TRASMITTED_ACK_RECEIVED  = 0x18;
    static const uint8_t MT_STATUS_SLA_W_TRASMITTED_NACK_RECEIVED = 0x20;
    static const uint8_t MT_STATUS_DATA_TRASMITTED_ACK_RECEIVED   = 0x28;
    static const uint8_t MT_STATUS_DATA_TRASMITTED_NACK_RECEIVED  = 0x30;
    static const uint8_t MT_STATUS_ARBITRATION_LOST               = 0x30;
    
    // Master Receiver Status Codes
    static const uint8_t MR_STATUS_START_TRANSMITTED               = 0x08;
    static const uint8_t MR_STATUS_REPEATED_START_TRANSMITTED      = 0x10;
    static const uint8_t MR_STATUS_ARBITRATION_LOST_OR_NACK        = 0x38;
    static const uint8_t MR_STATUS_SLA_R_TRANSMITTED_ACK_RECEIVED  = 0x40;
    static const uint8_t MR_STATUS_SLA_R_TRANSMITTED_NACK_RECEIVED = 0x48;
    static const uint8_t MR_STATUS_DATA_RECEIVED_ACK_RETURNED      = 0x50;
    static const uint8_t MR_STATUS_DATA_RECEIVED_NACK_RETURNED     = 0x58;
    
    // Misc Status Codes
    static const uint8_t MISC_STATUS_NO_STATE_AVAILABLE = 0xF8;
    static const uint8_t MISC_STATUS_BUS_ERROR          = 0x00;
    
    // Other
    static const uint32_t DEFAULT_CLOCK_FREQ = 100000;
    static const uint8_t  DEFAULT_PRESCALER  = 1;
    
public:

    TWIClass()
    : health_(1)
    , unexpectedTWIStatus_(0)
    {
        SetSCLPeriod();
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    // User Code
    //
    //////////////////////////////////////////////////////////////////////
    
    // Address 0 is a write-only, no-acknowledge address (broadcast).
    // We use the fact that 0 cannot be the address of the first slave and
    // instead use 0 to indicate that there is no first slave.
    // Any other value is the first address which responded.
    uint8_t GetFirstSlaveAddrPresent()
    {
        uint8_t retVal = 0;
        uint8_t found  = 0;
        
        for (uint8_t i = 0; i <= 127 && !found; ++i)
        {
            if (SlaveAddrPresent(i))
            {
                retVal = i;
                
                found = 1;
            }
        }
        
        return retVal;
    }
    
    uint8_t SlaveAddrPresent(uint8_t slaveAddr)
    {
        uint8_t retVal = Send(slaveAddr, NULL, 0);
        
        PAL.DelayMicroseconds(10);
        
        return retVal;
    }
    
    uint8_t SendAndReceive(uint8_t  slaveAddr,
                           uint8_t *bufTx,
                           uint8_t  bufTxLen,
                           uint8_t *bufRx,
                           uint8_t  bufRxLen)
    {
        SetHealthOk();

        SendStart();
        
        SendSlaveAddressWrite(slaveAddr);
        for (uint8_t i = 0; i < bufTxLen; ++i)
        {
            SendByte(bufTx[i]);
        }

        uint8_t repeated = 1;
        SendStart(repeated);
        
        SendSlaveAddressRead(slaveAddr);
        for (uint8_t i = 0; i < bufRxLen; ++i)
        {
            uint8_t ackOrNack = !(i == (bufRxLen - 1));
            
            bufRx[i] = ReceiveByte(ackOrNack);
        }
        
        SendStop();
        
        DealWithErrors();

        return GetHealth();
    }
    
    uint8_t Send(uint8_t slaveAddr, uint8_t *bufTx, uint8_t bufTxLen)
    {
        SetHealthOk();
        
        SendStart();
        SendSlaveAddressWrite(slaveAddr);
        for (uint8_t i = 0; i < bufTxLen; ++i)
        {
            SendByte(bufTx[i]);
        }
        SendStop();
        
        DealWithErrors();

        return GetHealth();
    }
    
    uint8_t WriteRegister(uint8_t slaveAddr, uint8_t reg, uint8_t val)
    {
        uint8_t buf[2] = { reg, val };
        uint8_t bufLen = 2;
        
        return Send(slaveAddr, buf, bufLen);
    }
    
    uint8_t ReadRegister(uint8_t slaveAddr, uint8_t reg, uint8_t &val)
    {
        return SendAndReceive(slaveAddr, &reg, 1, &val, 1);
    }
    
    
private:

    //////////////////////////////////////////////////////////////////////
    //
    // Init
    //
    //////////////////////////////////////////////////////////////////////

    void SetSCLPeriod()
    {
        TWBR = (((PAL.GetCpuFreq() / DEFAULT_CLOCK_FREQ) - 16 ) / 2);
    }

    
    //////////////////////////////////////////////////////////////////////
    //
    // Health and Error Condition Handling
    //
    //////////////////////////////////////////////////////////////////////

    void SetHealthOk()
    {
        health_ = 1;
    }
    
    uint8_t GetHealth()
    {
        return health_;
    }
    
    void SetHealth(uint8_t health)
    {
        health_ = health;
    }
    
    uint8_t GetTWIStatus()
    {
        return (TWSR & 0b11111000);
    }
    
    void ExpectedTWIStatus(uint8_t status)
    {
        SetHealth(GetTWIStatus() == status);
        
        if (!GetHealth())
        {
            SetUnexpectedTWIStatus(GetTWIStatus());
        }
    }
    
    void SetUnexpectedTWIStatus(uint8_t status)
    {
        unexpectedTWIStatus_ = status;
    }
    
    uint8_t GetUnexpectedTWIStatus()
    {
        return unexpectedTWIStatus_;
    }
    
    void DealWithErrors()
    {
        if (!GetHealth())
        {
            uint8_t status = GetUnexpectedTWIStatus();
            
            if (status == MT_STATUS_SLA_W_TRASMITTED_NACK_RECEIVED ||
                status == MT_STATUS_DATA_TRASMITTED_NACK_RECEIVED  ||
                status == MR_STATUS_SLA_R_TRANSMITTED_NACK_RECEIVED)
            {
                SendStopUnconditional();
            }
        }
    }


    //////////////////////////////////////////////////////////////////////
    //
    // Protocol Operation
    //
    //////////////////////////////////////////////////////////////////////
    
    void SendStart(uint8_t repeated = 0)
    {
        if (GetHealth())
        {
            // 1.
            // Application writes to TWCR to initiate transmission of START
            TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
            
            // 2.
            // TWINT set.  Status code indicates START condition sent
            while (!(TWCR & (1 << TWINT)));
            
            if (repeated)
            {
                ExpectedTWIStatus(MT_STATUS_REPEATED_START_TRANSMITTED);
            }
            else
            {
                ExpectedTWIStatus(MT_STATUS_START_TRANSMITTED);
            }
        }
    }
    
    void SendSlaveAddressWrite(uint8_t slaveAddr)
    {
        if (GetHealth())
        {
            SendSlaveAddressWithAlreadySetDirection(slaveAddr << 1 | 0x00);
            
            ExpectedTWIStatus(MT_STATUS_SLA_W_TRASMITTED_ACK_RECEIVED);
        }
    }
    
    void SendSlaveAddressRead(uint8_t slaveAddr)
    {
        if (GetHealth())
        {
            SendSlaveAddressWithAlreadySetDirection(slaveAddr << 1 | 0x01);
            
            ExpectedTWIStatus(MR_STATUS_SLA_R_TRANSMITTED_ACK_RECEIVED);
        }
    }
    
    void SendSlaveAddressWithAlreadySetDirection(uint8_t slaveAddrPlus)
    {
        if (GetHealth())
        {
            // 3.
            // Check TWSR to see if START was sent. Application loads SLA+W into
            // TWDR, and loads appropriate control signals into TWCR, making sure
            // that TWINT is written to one, and TWSTA is written to zero.
            TWDR = slaveAddrPlus;
            TWCR = (1 << TWINT) | (1 << TWEN);
            
            // 4.
            // TWINT set.  Status code indicates SLA+W sent, ACK received
            while (!(TWCR & (1 << TWINT)));
        }
    }
    
    void SendByte(uint8_t val)
    {
        if (GetHealth())
        {
            // 5.
            // Check TWSR to see if SLA+W was sent and ACK received. Application
            // loads data into TWDR, and loads appropriate control signals into
            // TWCR, making sure that TWINT is written to one
            TWDR = val;
            TWCR = (1 << TWINT) | (1 << TWEN);
            
            // 6.
            // TWINT set. Status code indicates data sent, ACK received
            while (!(TWCR & (1<<TWINT)));
            
            ExpectedTWIStatus(MT_STATUS_DATA_TRASMITTED_ACK_RECEIVED);
        }
    }
    
    uint8_t ReceiveByte(uint8_t ackOrNack)
    {
        uint8_t retVal = 0;
        
        if (GetHealth())
        {
            if (ackOrNack)
            {
                // ACK
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
            }
            else
            {
                // NACK
                TWCR = (1 << TWINT) | (1 << TWEN);
            }
            
            while (!(TWCR & (1 << TWINT)));
            
            if (ackOrNack)
            {
                // ACK
                ExpectedTWIStatus(MR_STATUS_DATA_RECEIVED_ACK_RETURNED);
            }
            else
            {
                // NACK
                ExpectedTWIStatus(MR_STATUS_DATA_RECEIVED_NACK_RETURNED);
            }
            
            if (GetHealth())
            {
                retVal = TWDR;
            }
        }
        
        return retVal;
    }
    
    void SendStop()
    {
        if (GetHealth())
        {
            SendStopUnconditional();
        }
    }
    
    void SendStopUnconditional()
    {
        TWCR = (1 << TWINT)|(1 << TWEN) | (1 << TWSTO);
        
        // Wait for stop condition to complete
        while (!(TWCR & (1 << TWSTO)));
    }
    
    
private:

    uint8_t health_;
    uint8_t unexpectedTWIStatus_;
};


// Singleton
extern TWIClass I2C;









/*

    void SetSCLPeriod()
    {
        // TWI Bit Rate Register
        // TWBR
        
        // p. 213
        // 
        //                           CPU Freq
        // SCL Frequency = -------------------------------
        //                 16 + ((2 * TWBR) * (Prescaler))
        // 
        // 
        
        // Note -- Slave frequency must be at least 16x SCL Frequency
    }


    // START
    void SendStart()
    {
        // 1.
        // Application writes to TWCR to initiate transmission of START
        TWCR = (1<<TWINT)|(1<<TWSTA|(1<<TWEN);
        
        // 2.
        // TWINT set.  Status code indicates START condition sent
        while (!(TWCR &(1<<TWINT)));
    }
    
    // SLA+W -- (SLA = Slave Adddress)
    void SendSlaveAddress()
    {
        // 3.
        // Check TWSR to see if START was sent. Application loads SLA+W into
        // TWDR, and loads appropriate control signals into TWCR, making sure
        // that TWINT is written to one, and TWSTA is written to zero.
        if ((TWSR & 0xF8) != START) ERROR();
        
        TWDR = SLA_W;
        TWCR = (1<<TWINT) | (1<<TWEN);
        
        // 4.
        // TWINT set.  Status code indicates SLA+W sent, ACK received
        while (!(TWCR & (1<<TWINT)));
    }
    
    // Data
    void SendData()
    {
        // 5.
        // Check TWSR to see if SLA+W was sent and ACK received. Application
        // loads data into TWDR, and loads appropriate control signals into
        // TWCR, making sure that TWINT is written to one
        if ((TWSR & 0xF8) != MT_SLA_ACK) ERROR();
        
        TWDR = DATA;
        TWCR = (1<<TWINT) | (1<<TWEN);
        
        
        // 6.
        // TWINT set. Status code indicates data sent, ACK received
        while (!(TWCR & (1<<TWINT)));
    }
    
    void SendStop()
    {
        // 7.
        // Check TWSR to see if data was sent and ACK received.
        // Application loads appropriate control signals to send STOP into TWCR,
        // making sure that TWINT is written to one
        
        if ((TWSR & 0xF8) != MT_DATA_ACK) ERROR();
        
        TWCR = (1<<TWINT)|(1<<TWEN | (1<<TWSTO);
    }
    
    
    //////////////////////////////////////////////////////////////////////
    //
    // Master Receiver Logic
    //
    //////////////////////////////////////////////////////////////////////
    
    // Same Start
    
    // Send SLA_R
    
    // Receive as many bytes as you want, then NACK

    // Now Stop, or repeated start
    
    

// SCL - clock pin (28)
// SDA - data pin  (27)


// TWI Bit Rate Register
// TWBR

// TWI Status Register
// TWSR
// - has status code of why interrupt flag set, or indication that nothing
//   is going on

// Data and Address Shift Register
// TWDR
// - data, or address of data, to send, or to have received
// - also START/STOP Controller and Arbitration

// TWI Control Register
// TWCR
// - ACK/NACK, when transmitting can see response here, when receiving can
//   set or clear using this
// - has TWEN bit which enables TWI
// - has TWINT bit which is an interrupt flag
//   - can tell what by looking at the TWSR status code
//   - as long as this is set, the SCL line is held low
//   - when transmitting, must clear this flag after each operation, as the
//     system will not proceed with this flag set
// - has TWIE bit which is interrupt enable flag
//   - if not using interrupts, simply poll for the TWINT:TWSR flag

// TWI Address Register
// TWAR
// - Give self address, also control wakeup/status when addressed on the bus

// Repeated start
// - don't let go of the bus
// - either continue to address the same slave, or switch slaves
// - must be done when instructing slave what to send, then reading
//   - otherwise another master could ask for other data that you then read


 */





#endif  // __TWI_CLASS_H__













