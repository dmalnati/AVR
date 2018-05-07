#include "hwdefs.h"
#include "measure.h"

#include "Arduino.h"

static TMeasureCal calibration; 

/**
 * Initialises the measurement system.
 */
void MeasureInit(void)
{
    pinMode(PIN_MEASURE_A, INPUT_ANALOG);
    pinMode(PIN_MEASURE_V, INPUT_ANALOG);
    calibration.cal_i = 1.0;
    calibration.cal_v = 1.0;
}

/**
 * Sets the calibration values.
 */
void MeasureCal(const TMeasureCal *cal)
{
    calibration = *cal;
}

/**
 * Gets a measurement.
 *
 * @param[out] time the timestamp for this measurement
 * @param[out] the current (amperes)
 * @param[out] the voltage (volts)
 */
void MeasureGet(unsigned long *time, float *current, float *voltage)
{
    /*
     * Enhancements
     * - make time not reset after some pathetically short duration (4290.0 + 5)
     *   - should time reset when you reset state also?
     * - make new interval values take effect immediately
     * - display limits when you do status?
     *   - or change status concept to be 'measure' or something, and then have
     *     some kind of "show my my state" command
     *   - or even just show you when 'limit' run and no new value specified
     * - give more space to the mAh column (maybe others too)
     *   - will have to shift over when over 99mAh, which will happen often
     * - anything come to mind to make scripting easier?
     * - useful info in prompt?
     */
    *time = micros();
    *current = calibration.cal_i * 5.0 * analogRead(PIN_MEASURE_A) / 4096;
    *voltage = calibration.cal_v * 19.8 * analogRead(PIN_MEASURE_V) / 4096;
}

