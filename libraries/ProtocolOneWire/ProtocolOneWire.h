#ifndef __PROTOCOL_ONE_WIRE_H__
#define __PROTOCOL_ONE_WIRE_H__


#include "PAL.h"
#include "crc.h"


class ProtocolOneWire
{
private:
    static const uint8_t DURATION_US_TIME_SLOT = 60;
    
    static const uint8_t DURATION_US_LOW_WRITE_0     =  90;
    
    static const uint8_t DURATION_US_LOW_WRITE_1_MIN =  1;
    static const uint8_t DURATION_US_LOW_WRITE_1_MAX = 15;
    static const uint8_t DURATION_US_LOW_WRITE_1     =  6;
    
    static const uint8_t DURATION_US_LOW_READ_MIN =  1;
    static const uint8_t DURATION_US_LOW_READ_MAX = 15;
    static const uint8_t DURATION_US_LOW_READ     =  6;
    
    static const uint8_t DURATION_US_WAIT_SLAVE_PRESENCE_RESPONSE = 90;
    
    static const uint16_t DURATION_US_LOW_RESET_SIGNAL_MIN = 8 * DURATION_US_TIME_SLOT;
    static const uint16_t DURATION_US_LOW_RESET_SIGNAL     = 9 * DURATION_US_TIME_SLOT;
    
    static const uint8_t CODE_CMD_READ_ROM = 0x33;
    static const uint8_t CODE_CMD_SKIP_ROM = 0xCC;
    
private:

    union SlaveId
    {
        SlaveId(uint64_t val) { id = val; }
        
        uint64_t id;
        uint8_t  byteList[8];
    };
    
public:
    ProtocolOneWire(uint8_t pin)
    : pin_(pin)
    {
        // Nothing to do
    }
    
    ~ProtocolOneWire()
    {
        // Nothing to do
    }
    
    void Init()
    {
        // Bus master keeps the line high until triggering communication
        PAL.PinMode(pin_, INPUT_PULLUP);
        
        // Delay ignoring any signal on the bus, as some slaves may respond
        // to the pullup high value as indicating the end of a reset.
        PAL.DelayMicroseconds(DURATION_US_TIME_SLOT * 3);
    }
    
    void WriteByte(uint8_t val)
    {
        uint8_t data = val;
        
        // LSB first
        for (uint8_t i = 0; i < 8; ++i)
        {
            WriteBit(data & 0x01);
            
            data >>= 1;
        }
    }
    
    uint8_t ReadByte()
    {
        uint8_t retVal = 0;
        
        // Read in LSB first
        for (uint8_t i = 0; i < 8; ++i)
        {
            // Shift prior bits right so that the first bit read in is the
            // LSB by the end of it all
            retVal >>= 1;
            
            retVal |= (ReadBit() << 7);
        }
        
        return retVal;
    }
    
    void WriteBit(uint8_t val)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (val)
            {
                // Pull low for duration shorter than full window indicating a 1
                PAL.PinMode(pin_, OUTPUT);
                PAL.DigitalWrite(pin_, LOW);
                PAL.DelayMicroseconds(DURATION_US_LOW_WRITE_1);
                PAL.PinMode(pin_, INPUT_PULLUP);
                
                // Wait for end of time write time slot
                PAL.DelayMicroseconds(DURATION_US_TIME_SLOT - DURATION_US_LOW_WRITE_1);
            }
            else
            {
                // Pull low for duration longer than the full window to indicate 0
                PAL.PinMode(pin_, OUTPUT);
                PAL.DigitalWrite(pin_, LOW);
                PAL.DelayMicroseconds(DURATION_US_LOW_WRITE_0);
                PAL.PinMode(pin_, INPUT_PULLUP);
            }
        }
    }
    
    uint8_t ReadBit()
    {
        uint8_t retVal = 0;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Pull low for duration indicating a read
            
            PAL.PinMode(pin_, OUTPUT);
            PAL.DigitalWrite(pin_, LOW);
            // commented this out because it's actually working just fine
            // without it given the inherent duration of time to mess around
            // with pin states.
            //PAL.DelayMicroseconds(DURATION_US_LOW_READ);
            PAL.PinMode(pin_, INPUT_PULLUP);
            
            // Wait for end of read time slot
            // "The bus should be sampled 15μs after the bus was pulled low"
            // Tuned, kind of, ultimately during debugging, didn't go back to
            // question it again.
            //PAL.DelayMicroseconds(DURATION_US_LOW_READ_MAX - DURATION_US_LOW_READ);
            PAL.DelayMicroseconds(5);
            retVal = PAL.DigitalRead(pin_);
            
            // Useful debug to spot where this logic recognizes a 1 or 0
            //PAL.DigitalWrite(dbg_, retVal ? HIGH : LOW);
            
            // now wait for remainder of 60us?
            // how long does slave have to complete?
            // Since we're effectively clocking the whole thing, we probably can't
            // wait "too long," so just wait a whole window before going to the next
            // bit.
            PAL.DelayMicroseconds(DURATION_US_TIME_SLOT);
        }
        
        return retVal;
    }
    
    // Returns whether there are any slaves on the bus
    uint8_t PresenceSignal()
    {
        uint8_t retVal = 0;
        
        // Pull low for duration indicating reset
        PAL.PinMode(pin_, OUTPUT);
        PAL.DigitalWrite(pin_, LOW);
        PAL.DelayMicroseconds(DURATION_US_LOW_RESET_SIGNAL);
        PAL.PinMode(pin_, INPUT_PULLUP);
        
        // "If there is a slave present, it should then pull the bus low
        //  within 60μs after it was released by the master and hold it low
        //  for at least 60μs"
        //
        // The delay here is:
        // -  60us padding before response possibly even starts
        // - 180us for up to 3 durations of 60us of low time, since seemingly
        //   there is no bound and we have to give up at some point.
        retVal = WaitLowThenHigh(DURATION_US_TIME_SLOT * 4);
        
        return retVal;
    }
    
    // Simply assert the Reset signal and return control when the bus line
    // is ready to use again.
    void ResetSignal()
    {
        PresenceSignal();
    }
    
    // Read the 64-bit unique ID of the single slave on the bus.
    // Does not work if zero or more than one slave connected.
    void CmdReadRom()
    {
        SlaveId slaveId = 0;
        
        // Issue command to send ID when read from
        WriteByte(CODE_CMD_READ_ROM);
        
        for (uint8_t i = 0; i < 8; ++i)
        {
            slaveId.byteList[i] = ReadByte();
        }

        uint8_t crcReceived   = slaveId.byteList[7];
        uint8_t crcCalculated = CRC8(slaveId.byteList, 7);
        
        // Check CRC
        if (crcReceived != crcCalculated)
        {
            // Set slaveId to zero to indicate failure
            slaveId = 0;
        }
    }
    
    // Address all slaves on the bus.
    // If there is only a single slave, this is the same as directly addressing
    // it.
    // If multiple slaves, only suitable for send-only data (commands).
    void CmdSkipRom()
    {
        WriteByte(CODE_CMD_SKIP_ROM);
    }
    
private:

    uint8_t WaitLowThenHigh(uint32_t timeoutUs, uint32_t *durationUs = NULL)
    {
        uint8_t retVal = 0;
        
        uint8_t isLow  = 0;
        uint8_t isHigh = 0;
        
        uint32_t timeStart = PAL.Micros();
        uint32_t timeNow   = timeStart;
        
        uint32_t timeLowStart = 0;
        
        // Wait for low state
        while (!isLow && (timeNow - timeStart) < timeoutUs)
        {
            isLow = !PAL.DigitalRead(pin_);
            
            timeNow = PAL.Micros();
        }
        
        // Note the time, this is when the low period began
        timeLowStart = timeNow;
        
        // Wait for high state
        while (!isHigh && (timeNow - timeStart) < timeoutUs)
        {
            isHigh = PAL.DigitalRead(pin_);
            
            timeNow = PAL.Micros();
        }
        
        // Determine duration of time the signal was low
        if (durationUs)
        {
            *durationUs = timeNow - timeLowStart;
        }
        
        retVal = isLow && isHigh;
        
        return retVal;
    }
    
private:

    Pin pin_;
};







#endif  // __PROTOCOL_ONE_WIRE_H__