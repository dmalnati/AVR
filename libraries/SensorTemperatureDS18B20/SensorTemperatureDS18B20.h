#ifndef __SENSOR_TEMPERATURE_DS18B20_H__
#define __SENSOR_TEMPERATURE_DS18B20_H__


#include "Evm.h"
#include "Function.h"
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
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint8_t CODE_CMD_CONVERT         = 0x44;
    static const uint8_t CODE_CMD_READ_SCRATCHPAD = 0xBE;
    
    static const uint16_t MAX_WAIT_BITS = 12000;
    
    static const uint8_t POLL_MS         = 20;
    static const uint8_t MAX_ASYNC_TRIES = 1000 / POLL_MS;  // max 1 sec
    
public:

    SensorTemperatureDS18B20(uint8_t pin)
    : ow_(pin)
    , asyncTriesRemaining_(MAX_ASYNC_TRIES)
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
    
    struct MeasurementAsync
    {
        uint8_t     retVal;
        Measurement m;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        if (m)
        {
            // Ask thermometer to take a reading
            RequestTemperatureReading();
            
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
                retVal = ReadTemperatureFromWire(m);
            }
        }
        
        return retVal;
    }
    
    void GetMeasurementAsync(function<void(MeasurementAsync)> cbFn)
    {
        // Retain callback function
        cbFn_ = cbFn;
        
        // Cancel existing async timer, if any
        // Note -- in this case no callback to the first caller will happen.
        tedAsync_.DeRegisterForTimedEvent();
        
        // Ask thermometer to take a reading
        RequestTemperatureReading();
        
        // Reset max tries counter
        asyncTriesRemaining_ = MAX_ASYNC_TRIES;
        
        // Set up timer to come check for temperature being ready to read
        tedAsync_.SetCallback([this](){
            --asyncTriesRemaining_;
            
            // Check if Thermometer has indicated temperature ready to read
            if (ow_.ReadBit())
            {
                MeasurementAsync ma;

                // Attempt to read, and set return value
                ma.retVal = ReadTemperatureFromWire(&ma.m);
                
                cbFn_(ma);
                
                tedAsync_.DeRegisterForTimedEvent();
            }
            else if (!asyncTriesRemaining_)
            {
                // Ran out of attempts.  Give up.
                MeasurementAsync ma;
                
                ma.retVal = 0;
                
                cbFn_(ma);
                
                tedAsync_.DeRegisterForTimedEvent();
            }
        });
        
        tedAsync_.RegisterForTimedEventInterval(POLL_MS);
    }
    
private:
    
    void RequestTemperatureReading()
    {
        // Reset the bus
        ow_.ResetSignal();
        
        // Address the temperature sensor indirectly, by addressing all
        ow_.CmdSkipRom();
        
        // Tell thermometer to take a reading
        ow_.WriteByte(CODE_CMD_CONVERT);
    }
    
    uint8_t ReadTemperatureFromWire(Measurement *m)
    {
        uint8_t retVal = 0;
        
        // Tell the thermometer to transmit the 9 byte "scratchpad" when read.
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
        
        if (retVal)
        {
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

        return retVal;
    }


    ProtocolOneWire ow_;
    
    function<void(MeasurementAsync)> cbFn_;
    TimedEventHandlerDelegate        tedAsync_;
    uint8_t                          asyncTriesRemaining_;
};


#endif  // __SENSOR_TEMPERATURE_DS18B20_H__