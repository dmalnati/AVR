#ifndef __APRS_POSITION_REPORT_MESSAGE_HAB_TRACKER_1_H__
#define __APRS_POSITION_REPORT_MESSAGE_HAB_TRACKER_1_H__


#include "APRSPositionReportMessage.h"


class APRSPositionReportMessageHABTracker1
: public APRSPositionReportMessage
{
public:


    //
    // 6-character field
    // bxxxxx
    // Where b is a literal b
    // The rest is digits of 1/10 hPa
    //
    // 1hPa = 100 Pascal = 1 millibar (mbar).
    // This unit is 1/10 of a millibar = 10 Pascal.
    // Sea level pressure is 101,325 Pa = 1,013.25 hPa = 10,132.5 of this unit.
    //
    void SetCommentBarometricPressure(double pressure)
    {
        AppendCommentString("b");
        AppendCommentU32PadLeft(round(pressure), 5, 0, 99999);
    }
    
    
    //
    // 4-character field
    // txxx
    // Where t is literal t
    // The rest is digits of farenheit
    // Range -99 to 999
    //
    void SetCommentTemperature(double temperatureF)
    {
        AppendCommentString("t");

        if (temperatureF >= 0)
        {
            AppendCommentU32PadLeft(round(temperatureF), 3, 0, 999);
        }
        else
        {
            AppendCommentString("-");
            AppendCommentU32PadLeft(round(fabs(temperatureF)), 2, 0, 99);
        }
    }
    
    
    //
    // 10-character field
    // Msxxsyyszz
    // Where M is literal M
    // s is sign, where either ' ' or '-'
    // xx,yy,zz are the centi-gauss parameters (fixed point 2 decimal places)
    // Range 0-99 for each
    //
    // Inputs in Gauss
    //
    void SetCommentMagnetics(double xGauss, double yGauss, double zGauss)
    {
        AppendCommentString("M");
        
        for (double val : (double []){ xGauss, yGauss, zGauss })
        {
            if (val >= 0)
            {
                AppendCommentString(" ");
                AppendCommentU32PadLeft(round(val * 100), 2, 0, 99);
            }
            else
            {
                AppendCommentString("-");
                AppendCommentU32PadLeft(round(fabs(val) * 100), 2, 0, 99);
            }
        }
    }
    
    
    //
    // 13-character field
    // Gsxxxsyyyszzz
    // Where G is literal G
    // s is sign, where either ' ' or '-'
    // xxx,yyy,zzz are the fixed point 1 whole 2 decimal places G acceleration
    // Range 0-999 for each
    //
    // Inputs in milli-Gs
    //
    void SetCommentAcceleration(double xmGs, double ymGs, double zmGs)
    {
        AppendCommentString("G");
        
        for (double val : (double []){ xmGs, ymGs, zmGs })
        {
            if (val >= 0)
            {
                AppendCommentString(" ");
                AppendCommentU32PadLeft(round(val / 10), 3, 0, 999);
            }
            else
            {
                AppendCommentString("-");
                AppendCommentU32PadLeft(round(fabs(val) / 10), 3, 0, 999);
            }
        }
    }
    
    
    //
    // 3-character field
    // Vvv
    // Where V is literal V
    // vvv are the fixed point 1 whole 1 decimal place voltage
    // Range 0-99
    //
    void SetCommentVoltage(double voltage)
    {
        AppendCommentString("V");
        AppendCommentU32PadLeft(round(fabs(voltage) * 10), 2, 0, 99);
    }
    
    
    

private:
};


#endif  // __APRS_POSITION_REPORT_MESSAGE_HAB_TRACKER_1_H__





