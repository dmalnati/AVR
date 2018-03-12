#ifndef __GEOFENCE_APRS_DATA_H__
#define __GEOFENCE_APRS_DATA_H__


#include "GeofenceAPRSBoundingArea.h"


class GeofenceAPRSData
{

    static const uint16_t POINT_RADIUS_MILES = 100;

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
        const uint8_t BUF_SIZE = 20 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Europe1, 20);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InUSA(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 32 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, USA, 32);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InThailand(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Thailand, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InJapan(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Japan, 12);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InSouthKorea(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, SouthKorea, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InChina(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 10 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, China, 10);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InMalaysia(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Malaysia, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InAustralia1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Australia1, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InAustralia2(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 16 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Australia2, 16);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InNewZealand(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, NewZealand, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InSouthAfrica(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 14 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, SouthAfrica, 14);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InMorocco(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Morocco, 12);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InChile(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Chile, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InArgentina(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Argentina, 12);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InBrazil1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Brazil1, 12);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InBrazil2(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 12 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Brazil2, 12);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InMexico(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 16 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Mexico, 16);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InTropicalIslands(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, TropicalIslands, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InHawaii(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Hawaii, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InAlaska1(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Alaska1, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }

    static uint8_t InEurope2(int16_t latitude, int16_t longitude)
    {
        // load into sram, making room for final connecting point which isn't
        // included in the data
        const uint8_t BUF_SIZE = 8 + 2;
        int16_t buf[BUF_SIZE];
        
        LoadToSram(buf, Europe2, 8);
        
        // use points to determine if point in polygon
        uint8_t retVal = PointInPolygon(latitude, longitude, buf, BUF_SIZE / 2);
        
        return retVal;
    }


private:

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
    static const int16_t Europe1        [20] PROGMEM;
    static const int16_t USA            [32] PROGMEM;
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
    static const int16_t Europe2        [ 8] PROGMEM;
};












const int16_t GeofenceAPRSData::Europe1[20] PROGMEM =
{
        10,   4975,
         4,   3834,
      3410,   3431,
      4693,   4229,
      2219,   5716,
      3722,   6091,
      2072,   6763,
      2719,   7071,
      2026,   7190,
         2,   6112,
};

const int16_t GeofenceAPRSData::USA[32] PROGMEM =
{
    -12907,   5154,
    -12453,   4734,
    -12500,   3940,
    -11861,   3232,
     -9726,   2705,
     -8402,   2973,
     -8183,   2455,
     -7886,   2522,
     -8097,   3094,
     -7603,   3504,
     -7373,   3992,
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
     14080,   4595,
     13685,   3858,
     12696,   3314,
     13056,   2931,
     14146,   3425,
     14695,   4383,
};

const int16_t GeofenceAPRSData::SouthKorea[8] PROGMEM =
{
     12458,   3789,
     12582,   3314,
     13087,   3539,
     12942,   3944,
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
      9738,    426,
     10351,    638,
     10560,    158,
     10011,   -198,
};

const int16_t GeofenceAPRSData::Australia1[8] PROGMEM =
{
     11773,  -3096,
     11364,  -3191,
     11549,  -3578,
     11878,  -3499,
};

const int16_t GeofenceAPRSData::Australia2[16] PROGMEM =
{
     13645,  -3485,
     14467,  -4434,
     14889,  -4440,
     15548,  -2647,
     14590,  -1550,
     14370,  -1693,
     15038,  -2725,
     14484,  -3405,
};

const int16_t GeofenceAPRSData::NewZealand[8] PROGMEM =
{
     17319,  -3328,
     17996,  -3772,
     17086,  -4707,
     16721,  -4468,
};

const int16_t GeofenceAPRSData::SouthAfrica[14] PROGMEM =
{
      2531,  -2477,
      2457,  -3232,
      1683,  -3281,
      1907,  -3568,
      2747,  -3440,
      3313,  -3011,
      2918,  -2360,
};

const int16_t GeofenceAPRSData::Morocco[12] PROGMEM =
{
     -1857,   3345,
     -1861,   2708,
      -778,   2678,
      -789,   3020,
     -1553,   3045,
     -1536,   3356,
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
     -8082,   2076,
     -5946,   1221,
     -5770,   1638,
     -7958,   2377,
};

const int16_t GeofenceAPRSData::Hawaii[8] PROGMEM =
{
    -15901,   2132,
    -15580,   1829,
    -15407,   1962,
    -15759,   2249,
};

const int16_t GeofenceAPRSData::Alaska1[8] PROGMEM =
{
    -14238,   5970,
    -15394,   5941,
    -15348,   6507,
    -14157,   6582,
};

const int16_t GeofenceAPRSData::Europe2[8] PROGMEM =
{
        -1,   6110,
         1,   3730,
      -976,   3321,
     -1169,   5723,
};


#endif   // __GEOFENCE_APRS_DATA_H__
