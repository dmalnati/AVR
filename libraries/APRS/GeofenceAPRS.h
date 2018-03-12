#ifndef __APRS_GEOFENCE_H__
#define __APRS_GEOFENCE_H__


#include "GeofenceAPRS_Generated.h"


class GeofenceAPRS
{
public:

    struct LocationDetails
    {
        uint32_t freqAprs = 144390000;
        uint8_t  deadZone = 0;
    };
    
    
    static LocationDetails GetLocationDetails(int16_t latitude,
                                              int16_t longitude)
    {
        //const uint32_t APRS_FREQ_US     = 144390000;
        //const uint32_t APRS_FREQ_EUROPE = 144800000;
        //const uint32_t APRS_FREQ_JAPAN  = 144660000;
        
        
        
        LocationDetails retVal;
        
        if (GeofenceAPRSData::InPanama(latitude, longitude))
        {
            Serial.println(F("InPanama"));
        }
        else if (GeofenceAPRSData::InSanJose(latitude, longitude))
        {
            Serial.println(F("InSanJose"));
        }
        else if (GeofenceAPRSData::InArgentina1(latitude, longitude))
        {
            Serial.println(F("InArgentina1"));
        }
        else if (GeofenceAPRSData::InArgentina2(latitude, longitude))
        {
            Serial.println(F("InArgentina2"));
        }
        else if (GeofenceAPRSData::InWhitehorseYukon(latitude, longitude))
        {
            Serial.println(F("InWhitehorseYukon"));
        }
        else if (GeofenceAPRSData::InAlaska2(latitude, longitude))
        {
            Serial.println(F("InAlaska2"));
        }
        else if (GeofenceAPRSData::InDubai(latitude, longitude))
        {
            Serial.println(F("InDubai"));
        }
        else if (GeofenceAPRSData::InMoscow(latitude, longitude))
        {
            Serial.println(F("InMoscow"));
        }
        else if (GeofenceAPRSData::InIceland(latitude, longitude))
        {
            Serial.println(F("InIceland"));
        }
        else if (GeofenceAPRSData::InJakarta(latitude, longitude))
        {
            Serial.println(F("InJakarta"));
        }
        else if (GeofenceAPRSData::InTelukAmbon(latitude, longitude))
        {
            Serial.println(F("InTelukAmbon"));
        }
        else if (GeofenceAPRSData::InEurope1(latitude, longitude))
        {
            Serial.println(F("InEurope1"));
        }
        else if (GeofenceAPRSData::InUSA(latitude, longitude))
        {
            Serial.println(F("InUSA"));
        }
        else if (GeofenceAPRSData::InThailand(latitude, longitude))
        {
            Serial.println(F("InThailand"));
        }
        else if (GeofenceAPRSData::InJapan(latitude, longitude))
        {
            Serial.println(F("InJapan"));
        }
        else if (GeofenceAPRSData::InSouthKorea(latitude, longitude))
        {
            Serial.println(F("InSouthKorea"));
        }
        else if (GeofenceAPRSData::InChina(latitude, longitude))
        {
            Serial.println(F("InChina"));
        }
        else if (GeofenceAPRSData::InMalaysia(latitude, longitude))
        {
            Serial.println(F("InMalaysia"));
        }
        else if (GeofenceAPRSData::InAustralia1(latitude, longitude))
        {
            Serial.println(F("InAustralia1"));
        }
        else if (GeofenceAPRSData::InAustralia2(latitude, longitude))
        {
            Serial.println(F("InAustralia2"));
        }
        else if (GeofenceAPRSData::InNewZealand(latitude, longitude))
        {
            Serial.println(F("InNewZealand"));
        }
        else if (GeofenceAPRSData::InSouthAfrica(latitude, longitude))
        {
            Serial.println(F("InSouthAfrica"));
        }
        else if (GeofenceAPRSData::InMorocco(latitude, longitude))
        {
            Serial.println(F("InMorocco"));
        }
        else if (GeofenceAPRSData::InChile(latitude, longitude))
        {
            Serial.println(F("InChile"));
        }
        else if (GeofenceAPRSData::InArgentina(latitude, longitude))
        {
            Serial.println(F("InArgentina"));
        }
        else if (GeofenceAPRSData::InBrazil1(latitude, longitude))
        {
            Serial.println(F("InBrazil1"));
        }
        else if (GeofenceAPRSData::InBrazil2(latitude, longitude))
        {
            Serial.println(F("InBrazil2"));
        }
        else if (GeofenceAPRSData::InMexico(latitude, longitude))
        {
            Serial.println(F("InMexico"));
        }
        else if (GeofenceAPRSData::InTropicalIslands(latitude, longitude))
        {
            Serial.println(F("InTropicalIslands"));
        }
        else if (GeofenceAPRSData::InHawaii(latitude, longitude))
        {
            Serial.println(F("InHawaii"));
        }
        else if (GeofenceAPRSData::InAlaska1(latitude, longitude))
        {
            Serial.println(F("InAlaska1"));
        }
        else if (GeofenceAPRSData::InEurope2(latitude, longitude))
        {
            Serial.println(F("InEurope2"));
        }



        else
        {
            retVal.deadZone = 1;
        }
        
        return retVal;
    }
};







#endif  // __APRS_GEOFENCE_H__


