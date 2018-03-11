#ifndef __GEOFENCE_APRS_DATA_H__
#define __GEOFENCE_APRS_DATA_H__


class GeofenceAPRSData
{
public:

    static void GetLatLngList_Europe(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Europe;
        *latLngListLen = 26;
    }

    static void GetLatLngList_USA(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = USA;
        *latLngListLen = 32;
    }

    static void GetLatLngList_Thailand(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Thailand;
        *latLngListLen = 8;
    }

    static void GetLatLngList_Japan(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Japan;
        *latLngListLen = 12;
    }

    static void GetLatLngList_SouthKorea(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = SouthKorea;
        *latLngListLen = 8;
    }

    static void GetLatLngList_China(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = China;
        *latLngListLen = 10;
    }

    static void GetLatLngList_Malaysia(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Malaysia;
        *latLngListLen = 8;
    }

    static void GetLatLngList_Australia1(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Australia1;
        *latLngListLen = 8;
    }

    static void GetLatLngList_Australia2(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Australia2;
        *latLngListLen = 16;
    }

    static void GetLatLngList_NewZealand(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = NewZealand;
        *latLngListLen = 8;
    }

    static void GetLatLngList_SouthAfrica(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = SouthAfrica;
        *latLngListLen = 14;
    }

    static void GetLatLngList_Morocco(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Morocco;
        *latLngListLen = 12;
    }

    static void GetLatLngList_Chile(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Chile;
        *latLngListLen = 8;
    }

    static void GetLatLngList_Argentina(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Argentina;
        *latLngListLen = 12;
    }

    static void GetLatLngList_Brazil1(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Brazil1;
        *latLngListLen = 12;
    }

    static void GetLatLngList_Brazil2(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Brazil2;
        *latLngListLen = 12;
    }

    static void GetLatLngList_Mexico(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = Mexico;
        *latLngListLen = 16;
    }

    static void GetLatLngList_TropicalIslands(const int16_t **latLngList, uint8_t *latLngListLen)
    {
        *latLngList    = TropicalIslands;
        *latLngListLen = 8;
    }


private:

    // GEOFENCE ARRAYS (longitude, latitude)
    static const int16_t Europe         [26] PROGMEM;
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
};


const int16_t GeofenceAPRSData::Europe[26] PROGMEM =
{
      -422,   6033,
     -1362,   5396,
      -497,   4552,
     -1310,   4345,
      -747,   3350,
       501,   3865,
      3410,   3431,
      4693,   4229,
      2219,   5716,
      3722,   6091,
      2072,   6763,
      2719,   7071,
      2026,   7190,
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


#endif   // __GEOFENCE_APRS_DATA_H__
