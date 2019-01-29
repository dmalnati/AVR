#ifndef __APP_AMP_TO_VOLT_H__
#define __APP_AMP_TO_VOLT_H__


#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "TimedEventHandler.h"
#include "SensorCurrentVoltageINA3221.h"


/*
 * This program is designed to make it possible to measure current using a
 * voltage measurement.
 *
 * Basically, the oscilloscope works well, but I can't get measurements of
 * current very easily.
 *
 * I'd like to be able to take a current measruement and have it presented as
 * a measure of voltage so I can capture both voltage and current in the same
 * scope measurements.
 *
 * For example, assume you want to express a range of Amperage values 0-1.
 * This could be expressed in voltage values 0-1.
 *
 */

struct AppAmpToVoltConfig
{
};


class AppAmpToVolt
{
public:
    AppAmpToVolt(AppAmpToVoltConfig &cfg)
    : cfg_(cfg)
    , c1_(sensor_.GetChannel1())
    , dbg1_(16, LOW)
    , dbg2_(17, LOW)
    {
        // Nothing to do
    }
    
    void Run()
    {
        LogStart(9600);
        Log("Starting");
        
        // Set up faster I2C comms
        SetupI2C();
        
        // Configure sensor to perform for our purposes
        SetupSensor();
        
        // Set up analog out
        SetupAnalogOut();
        
        // Prepare optimized mapping from amps to volts
        MakeHashLookup();
        
        // Set up timers
        ted_.SetCallback([this](){
            OnCallback();
        });
        ted_.RegisterForTimedEventInterval(0);

        // Handle events
        Log("Running");
        evm_.MainLoop();
    }

private:

    void SetupI2C()
    {
        I2C.SetFreqMax();
        
        Log("I2C Bus Speed set to Max: ", I2C.GetFreq());
    }
    
    void SetupSensor()
    {
        /*
         * I'm seeing a 2.5ms delay between current change and the sensor
         * picking it up by default.
         *
         * Reading the spec, we have:
         * - 3 channels, each with 2 voltage readings
         * - each can support averaging, each taking between 140us to 8.244ms
         *
         * For my initial application, I need only channel 1 shunt voltage.
         *
         * This is configured below.
         *
         */
    
        sensor_.SetChannel2Enable(0);
        sensor_.SetChannel3Enable(0);
        sensor_.SetShuntVoltageConversionTimeUs(140);
        sensor_.SetOperatingMode(0b00000101);   // shunt-only, continuous
    }

    void SetupAnalogOut()
    {
        // Analog out 1: Use part of PORTB and PORTD
        // 0 - PD2
        // 1 - PD3
        // 2 - PD4
        // 3 - PD5
        // 4 - PD6
        // 5 - PD7
        // 6 - PB0
        // 7 - PB1
        
        
        // R2R  76543210 (the bits of the actual analog R2R ladder)
        // D      765432
        // B    10
        
        
        // PORTD
        //Pin(2,  LOW);
        //Pin(3,  LOW);
        Pin(4,  LOW);
        Pin(5,  LOW);
        Pin(6,  LOW);
        Pin(11, LOW);
        Pin(12, LOW);
        Pin(13, LOW);
        
        // PORTB
        Pin(14, LOW);
        Pin(15, LOW);
        
        //OnMilliAmpReading(0);
        PublishAnalogOut1(1);   // Try to keep negative voltage from blowing the scale out
    }
    
    void MakeHashLookup()
    {
        // Have to wait for runtime to get VCC reading and subsequent
        // calculation of step size
        uint16_t vccMillivolts = PAL.ReadVccMillivoltsMaxInTime();
        
        STEP_SIZE_MILLI_VOLTS =
            (float)MILLI_AMPS_MAX / (((float)MILLI_AMPS_MAX / (float)vccMillivolts) * 256.0);

        
        Log("VCC Millivolts       : ", vccMillivolts);
        Log("MILLI_AMPS_MAX       : ", MILLI_AMPS_MAX);
        Log("BITS_TO_DROP         : ", BITS_TO_DROP);
        Log("STEP_SIZE_MILLI_AMPS : ", STEP_SIZE_MILLI_AMPS);
        Log("BUCKET_COUNT         : ", BUCKET_COUNT);
        Log("STEP_SIZE_MILLI_VOLTS: ", STEP_SIZE_MILLI_VOLTS);
        
        for (uint16_t i = 0; i < BUCKET_COUNT; ++i)
        {
            uint16_t milliAmps = (i * STEP_SIZE_MILLI_AMPS);
            uint8_t  analogOut = milliAmps / STEP_SIZE_MILLI_VOLTS;
            
            milliAmpsToAnalogOut_[i] = analogOut;
            
            uint16_t milliAmpsCalcd = analogOut * STEP_SIZE_MILLI_VOLTS;
            
            uint16_t diff = milliAmps - milliAmpsCalcd;
            
            Log("[", i, "] mA(", milliAmps, ") -> mAc(", milliAmpsCalcd, "); d(", diff, "), aO(", analogOut, ")");
        }
    }
    
    void OnCallback()
    {
        uint16_t milliAmps = 0;
        
        PAL.DigitalWrite(dbg1_, HIGH);
        if (c1_->GetShuntMilliAmps(milliAmps))
        {
            OnMilliAmpReading(milliAmps);
        }
        PAL.DigitalWrite(dbg1_, LOW);
    }
    
    void OnMilliAmpReading(uint16_t milliAmps)
    {
        // Only prepared to handle a certain upper threshold, so cap it
        if (milliAmps > MILLI_AMPS_MAX)
        {
            milliAmps = MILLI_AMPS_MAX;
        }
        
        uint8_t idx = (milliAmps >> BITS_TO_DROP);
        uint8_t analogOut = milliAmpsToAnalogOut_[idx];

        static uint16_t analogOutLast = 0;
        
        if (analogOutLast != analogOut)
        {
            PAL.DigitalWrite(dbg2_, HIGH);
            PublishAnalogOut1(analogOut);
            PAL.DigitalWrite(dbg2_, LOW);
            
            //Log(milliAmps, ", [", idx, "] ao(", analogOut, ") = ", analogOut * STEP_SIZE_MILLI_VOLTS);
        }
        analogOutLast = analogOut;
    }
    
    void PublishAnalogOut1(uint8_t val)
    {
        // Software compensate for weird voltage upswing when current rises
        // (see spreadsheet)
        val = val >> 1;
        
        // Calculate bits to distribute to the ports hooked up to R2R
        uint8_t portB = val >> 6;
        uint8_t portD = val << 2;
        
        PORTB = portB;
        PORTD = portD;
    }


    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppAmpToVoltConfig &cfg_;
    
    TimedEventHandlerDelegate ted_;
    
    SensorCurrentVoltageINA3221           sensor_;
    SensorCurrentVoltageINA3221::Channel *c1_;
    
    static const     uint16_t MILLI_AMPS_MAX        = 1000;
    static const     uint8_t  BITS_TO_DROP          = 4;
    static const     uint8_t  STEP_SIZE_MILLI_AMPS  = (1 << BITS_TO_DROP);
    static const     uint16_t BUCKET_COUNT          =
        ( (MILLI_AMPS_MAX + 1) / STEP_SIZE_MILLI_AMPS) +
        (((MILLI_AMPS_MAX + 1) % STEP_SIZE_MILLI_AMPS) == 0 ? 0 : 1);
    float STEP_SIZE_MILLI_VOLTS;
    
    uint8_t milliAmpsToAnalogOut_[BUCKET_COUNT];
    
    
    
    // Debug
    Pin dbg1_;
    Pin dbg2_;
};





#endif  // __APP_AMP_TO_VOLT_H__






















