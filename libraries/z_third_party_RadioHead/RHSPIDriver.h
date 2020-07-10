// RHSPIDriver.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2014 Mike McCauley
// $Id: RHSPIDriver.h,v 1.16 2020/06/15 23:39:39 mikem Exp mikem $

#ifndef RHSPIDriver_h
#define RHSPIDriver_h

#include "RHHardwareSPI.h"

// This is the bit in the SPI address that marks it as a write
#define RH_SPI_WRITE_MASK 0x80



class RHSPIDriver
{
public:
    RHSPIDriver(uint8_t slaveSelectPin)
        : 
        _slaveSelectPin(slaveSelectPin)
    {
    }

    bool init()
    {
        // start the SPI library with the default speeds etc:
        // On Arduino Due this defaults to SPI1 on the central group of 6 SPI pins
        _spi.begin();

        // Initialise the slave select pin
        // On Maple, this must be _after_ spi.begin

        // Sometimes we dont want to work the _slaveSelectPin here
        if (_slaveSelectPin != 0xff)
        pinMode(_slaveSelectPin, OUTPUT);

        deselectSlave();

        // This delay is needed for ATMega and maybe some others, but
        // 100ms is too long for STM32L0, and somehow can cause the USB interface to fail
        // in some versions of the core.
        delay(100);
        
        return true;
    }

    uint8_t spiRead(uint8_t reg)
    {
        uint8_t val;
        ATOMIC_BLOCK_START;
        selectSlave();
        _spi.transfer(reg & ~RH_SPI_WRITE_MASK); // Send the address with the write mask off
        val = _spi.transfer(0); // The written value is ignored, reg value is read
        deselectSlave();
        ATOMIC_BLOCK_END;
        return val;
    }

    uint8_t spiWrite(uint8_t reg, uint8_t val)
    {
        uint8_t status = 0;
        ATOMIC_BLOCK_START;
        _spi.beginTransaction();
        selectSlave();
        status = _spi.transfer(reg | RH_SPI_WRITE_MASK); // Send the address with the write mask on
        _spi.transfer(val); // New value follows
        deselectSlave();
        _spi.endTransaction();
        ATOMIC_BLOCK_END;
        return status;
    }

    uint8_t spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
    {
        uint8_t status = 0;
        ATOMIC_BLOCK_START;
        _spi.beginTransaction();
        selectSlave();
        status = _spi.transfer(reg & ~RH_SPI_WRITE_MASK); // Send the start address with the write mask off
        while (len--)
        *dest++ = _spi.transfer(0);
        deselectSlave();
        _spi.endTransaction();
        ATOMIC_BLOCK_END;
        return status;
    }

    uint8_t spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
    {
        uint8_t status = 0;
        ATOMIC_BLOCK_START;
        _spi.beginTransaction();
        selectSlave();
        status = _spi.transfer(reg | RH_SPI_WRITE_MASK); // Send the start address with the write mask on
        while (len--)
        _spi.transfer(*src++);
        deselectSlave();
        _spi.endTransaction();
        ATOMIC_BLOCK_END;
        return status;
    }

    void setSlaveSelectPin(uint8_t slaveSelectPin)
    {
        _slaveSelectPin = slaveSelectPin;
    }

    void spiUsingInterrupt(uint8_t interruptNumber)
    {
        _spi.usingInterrupt(interruptNumber);
    }

    void selectSlave()
    {
        digitalWrite(_slaveSelectPin, LOW);
    }
        
    void deselectSlave()
    {
        digitalWrite(_slaveSelectPin, HIGH);
    }

    /// Reference to the RHGenericSPI instance to use to transfer data with the SPI device
    RHHardwareSPI _spi;

    /// The pin number of the Slave Select pin that is used to select the desired device.
    uint8_t             _slaveSelectPin;
};

#endif
