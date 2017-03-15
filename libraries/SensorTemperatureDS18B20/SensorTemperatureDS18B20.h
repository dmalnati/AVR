#ifndef __SENSOR_TEMPERATURE_DS18B20_H__
#define __SENSOR_TEMPERATURE_DS18B20_H__


#include "ProtocolOneWire.h"
#include "crc.h"


/*
 * At the moment written to expect to be the Master on a 1-Wire bus with
 * the temperature sensor as the only slave.
 *
 * Does not attempt to discover slaves/IDs and instead addresses all with the
 * request for temperature.
 */
class SensorTemperatureDS18B20
{
private:
    static const uint8_t CODE_CMD_CONVERT         = 0x44;
    static const uint8_t CODE_CMD_READ_SCRATCHPAD = 0xBE;
    
    static const uint16_t MAX_WAIT_BITS = 12000;
    
public:

    SensorTemperatureDS18B20(uint8_t pin)
    : ow_(pin)
    {
        // Nothing to do
    }
    
    ~SensorTemperatureDS18B20()
    {
        // Nothing to do
    }
    
    void Init()
    {
        ow_.Init();
    }

    struct Measurement
    {
        int16_t tempF;
        int16_t tempC;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        if (m)
        {
            // Reset the bus
            ow_.ResetSignal();
            
            // Address the temperature sensor indirectly, by addressing all
            ow_.CmdSkipRom();
            
            // Tell thermometer to take a reading
            ow_.WriteByte(CODE_CMD_CONVERT);
            
            // Wait for thermometer to complete its reading.
            // While read is not yet complete
            uint16_t readsRemaining = MAX_WAIT_BITS;
            uint8_t thermometerReady = 0;
            uint8_t cont = 1;
            while (cont)
            {
                if (!ow_.ReadBit())
                {
                    --readsRemaining;
                    
                    if (!readsRemaining)
                    {
                        cont = 0;
                    }
                }
                else
                {
                    thermometerReady = 1;
                    
                    cont = 0;
                }
            }
            
            // Check if we gave up or if the thermometer is actually ready
            // to be read.
            if (thermometerReady)
            {
                // Tell the thermometer to transmit the 9 byte "scratchpad"
                // when read.
                ow_.ResetSignal();
                ow_.CmdSkipRom();
                ow_.WriteByte(CODE_CMD_READ_SCRATCHPAD);
                
                // scratchpad bytes:
                // 0 - Temperature LSB
                // 1 - Temperature MSB
                // 2 - Temperature alarm High
                // 3 - Temperature alarm Low
                // 4 - Configuration register
                // 5 - Reserved
                // 6 - Reserved
                // 7 - Reserved
                // 8 - CRC
                uint8_t buf[9] = { 0 };
                
                for (uint8_t i = 0; i < 9; ++i)
                {
                    buf[i] = ow_.ReadByte();
                }
                
                // Confirm checksum
                uint8_t crcReceived = buf[8];
                uint8_t crcCalculated = CRC8(buf, 8);
                
                // Temperature reading is only successful if received and
                // the checksum matches
                retVal = (crcReceived == crcCalculated);

                // Extract temperature bytes
                uint16_t byteBuf;
                memcpy((void *)&byteBuf, (void *)buf, 2);
                
                // convert byteBuf into network byte order, then convert to
                // host byte order.
                PAL.Swap2((uint8_t *)&byteBuf);
                
                // Now convert to host byte order
                byteBuf = PAL.ntohs(byteBuf);
                
                // Copy bytes into signed integer instead of casting which would
                // have undefined effects.
                int16_t tempC;
                memcpy((void *)&tempC, (void *)&byteBuf, 2);
                
                // Convert to actual temperature.
                // Based on default 12-bit resolution, the integer as-is is
                // suitable for direct use.
                // It is the number of 0.0625 degree celcius units make up
                // the actual temperature.
                tempC = (int16_t)(tempC * 0.0625);
                
                // Convert to Fahrenheit
                int16_t tempF = (int16_t)((tempC * (9.0 / 5.0)) + 32.0);

                // Fill out measurement structure
                m->tempF = tempF;
                m->tempC = tempC;
            }
        }
        
        return retVal;
    }

private:

    ProtocolOneWire ow_;
};


#endif  // __SENSOR_TEMPERATURE_DS18B20_H__