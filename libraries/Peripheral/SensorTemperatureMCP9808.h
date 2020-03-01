#ifndef __SENSOR_TEMPERATURE_MCP9808__
#define __SENSOR_TEMPERATURE_MCP9808__


#include "I2C.h"
#include "PAL.h"


// https://cdn-shop.adafruit.com/datasheets/MCP9808.pdf


class SensorTemperatureMCP9808
{
protected:
    static const uint8_t ADDR = 0x18;

public:

    static const uint32_t MS_MAX_TEMP_SENSE_DURATION = 300;

    struct Configuration
    {
        uint8_t tHyst;
        uint8_t shdn;
        uint8_t critLock;
        uint8_t winLock;
        uint8_t intClear;
        uint8_t alertStat;
        uint8_t alertCntl;
        uint8_t alertSel;
        uint8_t alertPol;
        uint8_t alertMod;
    };

    struct Measurement
    {
        uint16_t      manufacturerId;
        uint8_t       deviceId;
        uint8_t       deviceRevision;
        Configuration cfg;
        int8_t        sensorResolution;
        int8_t        tempF;
        int8_t        tempC;
        int8_t        tempAlertUpper;
        int8_t        tempAlertLower;
        int8_t        tempCritical;
    };


public:

    SensorTemperatureMCP9808(uint8_t addr = ADDR)
    : addr_(addr)
    {
        // Nothing to do
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Power Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void Sleep()
    {
        // Get current configuration
        uint16_t regVal;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::CONFIG, regVal);

        // Set the shutdown bit high
        regVal |= _BV(8);

        // Send the updated configuration back
        I2C.WriteRegister16(addr_, (uint8_t)RegisterPointer::CONFIG, regVal);
    }

    void Wake()
    {
        // Get current configuration
        uint16_t regVal;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::CONFIG, regVal);

        // Set the shutdown bit low
        regVal &= ~_BV(8);

        // Send the updated configuration back
        I2C.WriteRegister16(addr_, (uint8_t)RegisterPointer::CONFIG, regVal);
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Sensing Control
    //
    ///////////////////////////////////////////////////////////////////////////

    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;

        if (m)
        {
            // Check that communication is working by confirming the
            // manufacturer id matches the specification.
            m->manufacturerId = GetManufacturerId();

            if (m->manufacturerId == 0x0054)
            {
                retVal = 1;

                m->deviceId = GetDeviceId();
                m->deviceRevision = GetDeviceRevision();

                m->cfg = GetConfiguration();
                m->sensorResolution = GetSensorResolution();
                m->tempF = GetTempF();
                m->tempC = GetTempC();
                m->tempAlertUpper = GetTempAlertUpper();
                m->tempAlertLower = GetTempAlertLower();
                m->tempCritical = GetTempCritical();
            }
        }

        return retVal;
    }

    Configuration GetConfiguration()
    {
        uint16_t regVal;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::CONFIG, regVal);

        Configuration cfg;
        cfg.tHyst     = (regVal & 0b0000011000000000) >> 9;
        cfg.shdn      = (regVal & 0b0000000100000000) >> 8;
        cfg.critLock  = (regVal & 0b0000000010000000) >> 7;
        cfg.winLock   = (regVal & 0b0000000001000000) >> 6;
        cfg.intClear  = (regVal & 0b0000000000100000) >> 5;
        cfg.alertStat = (regVal & 0b0000000000010000) >> 4;
        cfg.alertCntl = (regVal & 0b0000000000001000) >> 3;
        cfg.alertSel  = (regVal & 0b0000000000000100) >> 2;
        cfg.alertPol  = (regVal & 0b0000000000000010) >> 1;
        cfg.alertMod  = (regVal & 0b0000000000000001) >> 0;

        return cfg;
    }

    int8_t GetTempC()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::TEMP, regVal);

        return GetTempFromRegisterVal(regVal, 4);
    }

    int8_t GetTempF()
    {
        return (GetTempC() * 9.0 / 5.0) + 32;
    }

    int8_t GetTempAlertUpper()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::ALERT_UPPER, regVal);

        return GetTempFromRegisterVal(regVal, 2);
    }

    int8_t GetTempAlertLower()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::ALERT_LOWER, regVal);

        return GetTempFromRegisterVal(regVal, 2);
    }

    int8_t GetTempCritical()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::CRITICAL_TEMP, regVal);

        return GetTempFromRegisterVal(regVal, 2);
    }

    uint16_t GetManufacturerId()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::MANUFACTURER_ID, regVal);

        return regVal;
    }

    uint8_t GetDeviceId()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::DEVICE_ID, regVal);

        return (uint8_t)(regVal >> 8);
    }

    uint8_t GetDeviceRevision()
    {
        uint16_t regVal = 0;
        I2C.ReadRegister16(addr_, (uint8_t)RegisterPointer::DEVICE_ID, regVal);

        return (uint8_t)(regVal & 0x00FF);
    }

    uint8_t GetSensorResolution()
    {
        uint8_t regVal = 0;
        I2C.ReadRegister(addr_, (uint8_t)RegisterPointer::SENSOR_RESOLUTION, regVal);

        return regVal;
    }

    void SetSensorResolution(uint8_t val)
    {
        I2C.WriteRegister(addr_, (uint8_t)RegisterPointer::SENSOR_RESOLUTION, val);
    }


private:

    int8_t GetTempFromRegisterVal(uint16_t regVal, uint8_t subDegPlaces)
    {
        int8_t temp = (regVal & 0x0FFF) >> subDegPlaces;

        if (regVal & 0x1000)
        {
            temp -= 256;
        }

        return temp;
    }

    enum class RegisterPointer : uint8_t
    {
        RESERVED = 0,
        CONFIG,
        ALERT_UPPER,
        ALERT_LOWER,
        CRITICAL_TEMP,
        TEMP,
        MANUFACTURER_ID,
        DEVICE_ID,
        SENSOR_RESOLUTION,
    };


private:

    uint8_t addr_;
};




#endif  // __SENSOR_TEMPERATURE_MCP9808__












