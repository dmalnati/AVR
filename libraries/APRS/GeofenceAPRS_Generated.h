#ifndef __GEOFENCE_APRS_DATA_H__
#define __GEOFENCE_APRS_DATA_H__


#include "GeofenceAPRSBoundingArea.h"


class GeofenceAPRSData
{
    static const uint16_t POINT_RADIUS_MILES = 200;

public:
    static uint8_t InPanama(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 902;
        const int16_t POINT_LONGITUDE = -7950;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InSanJose(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 992;
        const int16_t POINT_LONGITUDE = -8408;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InArgentina1(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = -4585;
        const int16_t POINT_LONGITUDE = -6751;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InArgentina2(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = -5483;
        const int16_t POINT_LONGITUDE = -6833;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InWhitehorseYukon(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 6072;
        const int16_t POINT_LONGITUDE = -13505;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InAlaska2(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 5830;
        const int16_t POINT_LONGITUDE = -13442;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InDubai(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 2550;
        const int16_t POINT_LONGITUDE = 5556;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InMoscow(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 5575;
        const int16_t POINT_LONGITUDE = 3762;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InIceland(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = 6396;
        const int16_t POINT_LONGITUDE = -2083;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InJakarta(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = -638;
        const int16_t POINT_LONGITUDE = 10719;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InTelukAmbon(int16_t latitude, int16_t longitude)
    {
        const int16_t POINT_LATITUDE  = -366;
        const int16_t POINT_LONGITUDE = 12819;

        uint8_t retVal = PointInCircle(latitude,
                                       longitude,
                                       POINT_LATITUDE,
                                       POINT_LONGITUDE,
                                       POINT_RADIUS_MILES);

        return retVal;
    }

    static uint8_t InEurope1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 24 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Europe1, 24, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InUSA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 26 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, USA, 26, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InThailand(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Thailand, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InJapan(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Japan, 12, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InSouthKorea(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, SouthKorea, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InChina(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 10 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, China, 10, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InMalaysia(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Malaysia, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InAustralia1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Australia1, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InAustralia2(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 16 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Australia2, 16, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InNewZealand(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, NewZealand, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InSouthAfrica(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 14 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, SouthAfrica, 14, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InMorocco(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Morocco, 12, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InChile(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Chile, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InArgentina(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Argentina, 12, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InBrazil1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Brazil1, 12, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InBrazil2(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Brazil2, 12, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InMexico(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 16 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Mexico, 16, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InTropicalIslands(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, TropicalIslands, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InHawaii(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Hawaii, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InAlaska1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, Alaska1, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_USA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_USA, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_BRAZIL(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_BRAZIL, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_ARGENTINA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_ARGENTINA, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_CHILE(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_CHILE, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_COLOMBIA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_COLOMBIA, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_VENEZUELA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 6 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_VENEZUELA, 6, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_AUSTRALIA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_AUSTRALIA, 8, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_CHINA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 10 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_CHINA, 10, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_EU_AF_OTHER(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 18 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_EU_AF_OTHER, 18, latitude, longitude);
        
        return retVal;
    }

    static uint8_t InREG_APAC(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        // use points to determine if point in polygon
        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, REG_APAC, 8, latitude, longitude);
        
        return retVal;
    }


private:

    // single function to load values to sram as well as do the next step
    // of checking if the point is in the polygon.
    // saves program size to combine these two steps into a single function.
    static uint8_t LoadToSramAndCheckPointInPolygon(int16_t       *buf, 
                                                    const int16_t *latLngList,
                                                    uint8_t        latLngListLen,
                                                    int16_t        latitude,
                                                    int16_t        longitude)
    {
        LoadToSram(buf, latLngList, latLngListLen);

        return PointInPolygon(latitude, longitude, buf, ((latLngListLen + 2) / 2));
    }

    // Takes care of pulling out values from eeprom, as well as populating
    // the final point by duplicating the first.
    // Callers must be sure to allocate enough space in the buffer.
    static void LoadToSram(int16_t       *buf, 
                           const int16_t *latLngList,
                           uint8_t        latLngListLen)
    {
        uint8_t i = 0;

        for (i = 0; i < latLngListLen; ++i)
        {
            int16_t val;
            
            uint16_t pgmByteLocation = (uint16_t)latLngList + (i * sizeof(val));
            
            val = pgm_read_dword_near(pgmByteLocation);
            
            buf[i] = val;
        }

        // add in the final point by duplicating the first
        buf[i + 0] = buf[0];
        buf[i + 1] = buf[1];
    }

    // GEOFENCE ARRAYS (longitude, latitude)
    static const int16_t Europe1        [24] PROGMEM;
    static const int16_t USA            [26] PROGMEM;
    static const int16_t Thailand       [ 8] PROGMEM;
    static const int16_t Japan          [12] PROGMEM;
    static const int16_t SouthKorea     [ 8] PROGMEM;
    static const int16_t China          [10] PROGMEM;
    static const int16_t Malaysia       [ 8] PROGMEM;
    static const int16_t Australia1     [ 8] PROGMEM;
    static const int16_t Australia2     [16] PROGMEM;
    static const int16_t NewZealand     [ 8] PROGMEM;
    static const int16_t SouthAfrica    [14] PROGMEM;
    static const int16_t Morocco        [12] PROGMEM;
    static const int16_t Chile          [ 8] PROGMEM;
    static const int16_t Argentina      [12] PROGMEM;
    static const int16_t Brazil1        [12] PROGMEM;
    static const int16_t Brazil2        [12] PROGMEM;
    static const int16_t Mexico         [16] PROGMEM;
    static const int16_t TropicalIslands[ 8] PROGMEM;
    static const int16_t Hawaii         [ 8] PROGMEM;
    static const int16_t Alaska1        [ 8] PROGMEM;
    static const int16_t REG_USA        [ 8] PROGMEM;
    static const int16_t REG_BRAZIL     [ 8] PROGMEM;
    static const int16_t REG_ARGENTINA  [ 8] PROGMEM;
    static const int16_t REG_CHILE      [ 8] PROGMEM;
    static const int16_t REG_COLOMBIA   [ 8] PROGMEM;
    static const int16_t REG_VENEZUELA  [ 6] PROGMEM;
    static const int16_t REG_AUSTRALIA  [ 8] PROGMEM;
    static const int16_t REG_CHINA      [10] PROGMEM;
    static const int16_t REG_EU_AF_OTHER[18] PROGMEM;
    static const int16_t REG_APAC       [ 8] PROGMEM;
};

const int16_t GeofenceAPRSData::Europe1[24] PROGMEM =
{
     -1322,   3667,
      -424,   3469,
       598,   3800,
      3410,   3314,
      4843,   4197,
      2421,   5663,
      3722,   6091,
      2072,   6763,
      2719,   7071,
      2026,   7190,
     -1552,   5288,
      -743,   4904,
};

const int16_t GeofenceAPRSData::USA[26] PROGMEM =
{
    -12907,   5154,
    -12834,   4241,
    -12048,   3128,
     -9766,   2543,
     -8506,   2899,
     -8255,   2270,
     -7584,   2522,
     -7887,   3027,
     -6154,   4503,
     -6003,   4710,
     -8099,   4515,
    -10116,   4895,
    -10841,   4677,
};

const int16_t GeofenceAPRSData::Thailand[8] PROGMEM =
{
      9637,   2113,
      9875,    500,
     10358,    653,
     10670,   1927,
};

const int16_t GeofenceAPRSData::Japan[12] PROGMEM =
{
     14115,   4613,
     13623,   3899,
     12634,   3247,
     13030,   2842,
     14225,   3374,
     14911,   4409,
};

const int16_t GeofenceAPRSData::SouthKorea[8] PROGMEM =
{
     12366,   3974,
     12621,   3269,
     13162,   3617,
     13008,   4307,
};

const int16_t GeofenceAPRSData::China[10] PROGMEM =
{
     12199,   2211,
     12362,   4154,
     11514,   4124,
      9910,   2873,
     10936,   1796,
};

const int16_t GeofenceAPRSData::Malaysia[8] PROGMEM =
{
      9681,    406,
     10411,    673,
     10637,    112,
      9993,   -281,
};

const int16_t GeofenceAPRSData::Australia1[8] PROGMEM =
{
     11774,  -3051,
     11197,  -3188,
     11394,  -3706,
     12007,  -3571,
};

const int16_t GeofenceAPRSData::Australia2[16] PROGMEM =
{
     13583,  -3503,
     14436,  -4484,
     14959,  -4468,
     15684,  -2616,
     14708,  -1469,
     14300,  -1743,
     14911,  -2718,
     14405,  -3339,
};

const int16_t GeofenceAPRSData::NewZealand[8] PROGMEM =
{
     17295,  -3236,
    -17917,  -3754,
     17005,  -4918,
     16398,  -4569,
};

const int16_t GeofenceAPRSData::SouthAfrica[14] PROGMEM =
{
      2531,  -2477,
      2457,  -3232,
      1629,  -3237,
      1867,  -3585,
      2769,  -3456,
      3313,  -3011,
      2918,  -2360,
};

const int16_t GeofenceAPRSData::Morocco[12] PROGMEM =
{
     -1962,   3360,
     -1982,   2526,
      -784,   2550,
      -793,   3069,
     -1507,   3086,
     -1521,   3418,
};

const int16_t GeofenceAPRSData::Chile[8] PROGMEM =
{
     -7286,  -3456,
     -7280,  -2938,
     -6836,  -2948,
     -6831,  -3460,
};

const int16_t GeofenceAPRSData::Argentina[12] PROGMEM =
{
     -6581,  -3009,
     -6598,  -3260,
     -6073,  -3596,
     -5902,  -3988,
     -5348,  -3514,
     -5626,  -2923,
};

const int16_t GeofenceAPRSData::Brazil1[12] PROGMEM =
{
     -5225,  -3402,
     -5517,  -2198,
     -4837,  -1435,
     -3942,  -1944,
     -4025,  -2280,
     -4632,  -2485,
};

const int16_t GeofenceAPRSData::Brazil2[12] PROGMEM =
{
     -3803,  -1407,
     -4047,  -1296,
     -3658,   -719,
     -4032,   -356,
     -3830,   -235,
     -3287,   -725,
};

const int16_t GeofenceAPRSData::Mexico[16] PROGMEM =
{
     -8576,   2288,
     -8911,   1296,
     -9182,   1346,
     -9296,   1856,
    -10549,   1785,
    -10822,   2473,
    -10333,   2487,
     -9633,   1906,
};

const int16_t GeofenceAPRSData::TropicalIslands[8] PROGMEM =
{
     -8367,   2237,
     -5966,   1163,
     -5733,   1722,
     -7958,   2377,
};

const int16_t GeofenceAPRSData::Hawaii[8] PROGMEM =
{
    -15915,   2124,
    -15584,   1769,
    -15325,   1970,
    -15698,   2310,
};

const int16_t GeofenceAPRSData::Alaska1[8] PROGMEM =
{
    -14238,   5970,
    -15394,   5941,
    -15348,   6507,
    -14157,   6582,
};

const int16_t GeofenceAPRSData::REG_USA[8] PROGMEM =
{
     -2971,   1255,
    -17332,   1358,
    -17121,   7440,
     -2531,   7050,
};

const int16_t GeofenceAPRSData::REG_BRAZIL[8] PROGMEM =
{
     -4641,  -3045,
     -7471,   -597,
     -5572,    945,
     -2900,   -701,
};

const int16_t GeofenceAPRSData::REG_ARGENTINA[8] PROGMEM =
{
     -6601,  -2072,
     -7304,  -5112,
     -6794,  -5238,
     -5212,  -3438,
};

const int16_t GeofenceAPRSData::REG_CHILE[8] PROGMEM =
{
     -7348,  -1731,
     -7919,  -5018,
     -7315,  -5115,
     -6616,  -2059,
};

const int16_t GeofenceAPRSData::REG_COLOMBIA[8] PROGMEM =
{
     -7225,   1255,
     -6658,     97,
     -8042,   -919,
     -8618,    351,
};

const int16_t GeofenceAPRSData::REG_VENEZUELA[6] PROGMEM =
{
     -5607,    971,
     -6636,    110,
     -7172,   1238,
};

const int16_t GeofenceAPRSData::REG_AUSTRALIA[8] PROGMEM =
{
     10582,  -1358,
     16471,   -875,
     16172,  -4704,
     10863,  -4333,
};

const int16_t GeofenceAPRSData::REG_CHINA[10] PROGMEM =
{
     11162,   1680,
      6908,   3617,
      8561,   4938,
     10415,   4339,
     13957,   5385,
};

const int16_t GeofenceAPRSData::REG_EU_AF_OTHER[18] PROGMEM =
{
     16172,   8098,
     -1758,   6990,
     -2760,  -4164,
     10582,  -4229,
      9571,   2405,
      6847,   3617,
      8552,   4957,
     10424,   4358,
    -17895,   6355,
};

const int16_t GeofenceAPRSData::REG_APAC[8] PROGMEM =
{
     16594,   -719,
      9809,  -1290,
      9404,   2243,
     16102,   2227,
};


#endif   // __GEOFENCE_APRS_DATA_H__
