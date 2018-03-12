#ifndef __APRS_GEOFENCE_H__
#define __APRS_GEOFENCE_H__


#include "GeofenceAPRS_Generated.h"


class GeofenceAPRS
{
    static const uint32_t APRS_FREQ_US          = 144390000;
    static const uint32_t APRS_FREQ_INDONESIA   = 144390000;
    static const uint32_t APRS_FREQ_CHILE       = 144390000;
    static const uint32_t APRS_FREQ_MALAYSIA    = 144390000;
    static const uint32_t APRS_FREQ_NEW_ZEALAND = 144575000;
    static const uint32_t APRS_FREQ_SOUTH_KOREA = 144620000;
    static const uint32_t APRS_FREQ_CHINA       = 144640000;
    static const uint32_t APRS_FREQ_JAPAN       = 144660000;
    static const uint32_t APRS_FREQ_EU_RU_AF    = 144800000;
    static const uint32_t APRS_FREQ_ARGENTINA   = 144930000;
    static const uint32_t APRS_FREQ_PANAMA      = 145010000;
    static const uint32_t APRS_FREQ_COSTA_RICA  = 145010000;
    static const uint32_t APRS_FREQ_AUSTRALIA   = 145175000;
    static const uint32_t APRS_FREQ_THAILAND    = 145525000;
    static const uint32_t APRS_FREQ_BRAZIL      = 145570000;

public:

    struct LocationDetails
    {
        uint32_t freqAprs = APRS_FREQ_US;
        uint8_t  deadZone = 0;
    };
    
    //
    // takes latitude and longitude in degrees, with 6 decimal places of
    // precision, in integer format.
    //
    // - so lat - 30.0881078 is represented as -30088107
    // - so lng  172.9687500 is represented as 172968750
    //
    // this is the native TinyGPS format, and so convenient to expose to calling
    // code.
    //
    static LocationDetails GetLocationDetails(int32_t latitudeDegreesMillionths,
                                              int32_t longitudeDegreesMillionths)
    {
        int16_t latitude  = latitudeDegreesMillionths  / 10000;
        int16_t longitude = longitudeDegreesMillionths / 10000;
        
        return GetLocationDetailsInternal(latitude, longitude);
    }
    
private:

    //
    // takes latitude and longitude in degrees, with 2 decimal places of
    // precision, in integer format.
    //
    // - so lat - 30.0881078 is represented as -3008
    // - so lng  172.9687500 is represented as 17296
    //
    static LocationDetails GetLocationDetailsInternal(int16_t latitude,
                                                      int16_t longitude)
    {
        LocationDetails retVal;
        
        if (GeofenceAPRSData::InPanama(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_PANAMA;
        }
        else if (GeofenceAPRSData::InSanJose(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_COSTA_RICA;
        }
        else if (GeofenceAPRSData::InArgentina1(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_ARGENTINA;
        }
        else if (GeofenceAPRSData::InArgentina2(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_ARGENTINA;
        }
        else if (GeofenceAPRSData::InWhitehorseYukon(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else if (GeofenceAPRSData::InAlaska2(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else if (GeofenceAPRSData::InDubai(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_EU_RU_AF;
        }
        else if (GeofenceAPRSData::InMoscow(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_EU_RU_AF;
        }
        else if (GeofenceAPRSData::InIceland(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_EU_RU_AF;
        }
        else if (GeofenceAPRSData::InJakarta(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_INDONESIA;
        }
        else if (GeofenceAPRSData::InTelukAmbon(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_INDONESIA;
        }
        else if (GeofenceAPRSData::InEurope1(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_EU_RU_AF;
        }
        else if (GeofenceAPRSData::InUSA(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else if (GeofenceAPRSData::InThailand(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_THAILAND;
        }
        else if (GeofenceAPRSData::InJapan(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_JAPAN;
        }
        else if (GeofenceAPRSData::InSouthKorea(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_SOUTH_KOREA;
        }
        else if (GeofenceAPRSData::InChina(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_CHINA;
        }
        else if (GeofenceAPRSData::InMalaysia(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_MALAYSIA;
        }
        else if (GeofenceAPRSData::InAustralia1(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_AUSTRALIA;
        }
        else if (GeofenceAPRSData::InAustralia2(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_AUSTRALIA;
        }
        else if (GeofenceAPRSData::InNewZealand(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_NEW_ZEALAND;
        }
        else if (GeofenceAPRSData::InSouthAfrica(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_EU_RU_AF;
        }
        else if (GeofenceAPRSData::InMorocco(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_EU_RU_AF;
        }
        else if (GeofenceAPRSData::InChile(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_CHILE;
        }
        else if (GeofenceAPRSData::InArgentina(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_ARGENTINA;
        }
        else if (GeofenceAPRSData::InBrazil1(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_BRAZIL;
        }
        else if (GeofenceAPRSData::InBrazil2(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_BRAZIL;
        }
        else if (GeofenceAPRSData::InMexico(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else if (GeofenceAPRSData::InTropicalIslands(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else if (GeofenceAPRSData::InHawaii(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else if (GeofenceAPRSData::InAlaska1(latitude, longitude))
        {
            retVal.freqAprs = APRS_FREQ_US;
        }
        else
        {
            retVal.deadZone = 1;
        }
        
        return retVal;
    }
};







#endif  // __APRS_GEOFENCE_H__


