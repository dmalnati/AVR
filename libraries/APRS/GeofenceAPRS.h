#ifndef __APRS_GEOFENCE_H__
#define __APRS_GEOFENCE_H__




class GeofenceAPRS
{
public:

    struct LocationDetails
    {
        uint32_t freqAprs = 144390000;
    };
    
    LocationDetails GetLocationDetails(double latitude, double longitude)
    {
        LocationDetails retVal;
        
        GEOFENCE_position(latitude, longitude);

        retVal.freqAprs = GEOFENCE_APRS_frequency;
        
        return retVal;
    }
    
private:


// Taken from TT7 and adjusted
// http://tt7hab.blogspot.com/2017/02/aprs-automatic-packet-reporting-system.html
// https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_GEOFENCE.c
// https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_GEOFENCE.h


int32_t pointInPolygon(int32_t polyCorners, const int32_t * polygonPgm, int32_t latitude, int32_t longitude)
{
    int32_t buf[polyCorners * 2];
    
    for (int32_t i = 0; i < polyCorners * 2; ++i)
    {
        uint16_t pgmByteLocation = (uint16_t)polygonPgm + (i * 4);
        int32_t  val             = pgm_read_dword_near(pgmByteLocation);
        
        Serial.print("[");
        Serial.print(i);
        Serial.print("] ");
        Serial.print(pgmByteLocation);
        Serial.print(" = ");
        Serial.print(val);
        Serial.println();
        
        buf[i] = val;
    }
    
    int32_t retVal = pointInPolygonSram(polyCorners, buf, latitude, longitude);
    
    Serial.println(retVal ? "YES" : "NO");
    
    return retVal;
}

/*
	Adapted version of pointInPolygon() function from:	http://alienryderflex.com/polygon/
	
	Returns '0' if the point is outside of the polygon and '1' if it's inside.
	
	Expects input DEGREES * 100000 for latitude and longitude. Eg 4961070 for 49.61070 N.
	The reason is to make sure all calculations fit inside int32_t.
	
	However, this function is not very accurate due to rounding within the computation.
*/
int32_t pointInPolygonSram(int32_t polyCorners, const int32_t * polygon, int32_t latitude, int32_t longitude)
{
	int32_t i;
	int32_t j = polyCorners * 2 - 2;
	int32_t oddNodes = 0;

	for (i = 0; i < polyCorners * 2; i += 2)
	{
        uint8_t cond1 = (polygon[i + 1] < latitude && polygon[j + 1] >= latitude);
        uint8_t cond2 = (polygon[j + 1] < latitude && polygon[i + 1] >= latitude);
        uint8_t cond3 = (polygon[i] <= longitude || polygon[j] <= longitude);
        
        if ((cond1 || cond2) && cond3)
		{
			oddNodes ^= (polygon[i] + (latitude - polygon[i + 1])
			/ (polygon[j + 1] - polygon[i + 1]) * (polygon[j] - polygon[i]) < longitude);
		}

		j = i;
	}

	return oddNodes;
}



/*
	Changes GEOFENCE_APRS_frequency and GEOFENCE_no_tx global variables based on the input coordinates.
	
	FREQUENCIES:
						Africa				144.800
						Europe				144.800
						Russia				144.800
						Canada				144.390
						Mexico				144.390
						USA					144.390
						Costa Rica			145.010
						Nicaragua			145.010
						Panama				145.010
						Venezuela			145.010
						Brazil				145.570
						Colombia			144.390
						Chile				144.390
						Argentina			144.930
						Paraguay			144.930
						Uruguay				144.930
						China				144.640
						Japan				144.660
						South Korea			144.620
						Thailand			145.525
						Australia			145.175
						New Zealand			144.575
						Indonesia			144.390
						Malaysia			144.390
		
	NO AIRBORNE APRS:
						France
						Latvia
						Romania				X
						United Kingdom
	
	Expected input FLOAT for latitude and longitude as in GPS_UBX_latitude_Float and GPS_UBX_longitude_Float.
*/


uint8_t GEOFENCE_no_tx;
uint32_t GEOFENCE_APRS_frequency;

void GEOFENCE_position(int32_t latitude, int32_t longitude)
{
    

	// SECTOR 1
	if(longitude > -3800000 && longitude < 7300000)
	{
		
		// S 1/2
		if(latitude > 0.0)
		{
			if(pointInPolygon(9, UK, latitude, longitude) == 1)				{GEOFENCE_no_tx = 1;}
			//else if(pointInPolygon(8, France, latitude, longitude) == 1)		{GEOFENCE_no_tx = 1;}
			//else if(pointInPolygon(6, Romania, latitude, longitude) == 1)		{GEOFENCE_no_tx = 1;}
			else if(pointInPolygon(10, Latvia, latitude, longitude) == 1)		{GEOFENCE_no_tx = 1;}
			else																{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144800000;}
		}
		
		// S 2/2
		else
		{
			if(pointInPolygon(9, Brazil, latitude, longitude) == 1)			{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145570000;}
			else																{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144800000;}
		}
	}
	
	// SECTOR 2
	else if(longitude <= -3800000)
	{
		
		// S 1/2
		if(latitude > 1250000)
		{
																				{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144390000;}
		}
		
		// S 2/2
		else
		{
			if(pointInPolygon(8, ArgParUru, latitude, longitude) == 1)		{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144930000;}
			else if(pointInPolygon(9, Brazil, latitude, longitude) == 1)		{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145570000;}
			else if(pointInPolygon(7, Venezuela, latitude, longitude) == 1)	{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145010000;}
			else if(pointInPolygon(5, CostNicPan, latitude, longitude) == 1)	{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145010000;}
			else																{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144390000;}
		}
	}
	
	// SECTOR 3
	else if(longitude >= 7300000)
	{
		
		// S 1/2
		if(latitude > 1920000)
		{
			if(pointInPolygon(12, China, latitude, longitude) == 1)			{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144640000;}
			else if(pointInPolygon(7, Japan, latitude, longitude) == 1)		{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144660000;}
			else if(pointInPolygon(5, South_Korea, latitude, longitude) == 1)	{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144620000;}
			else if(pointInPolygon(5, Thailand, latitude, longitude) == 1)	{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145525000;}
			else																{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144800000;}
		}
		
		// S 2/2
		else
		{
			if(pointInPolygon(6, Australia, latitude, longitude) == 1)		{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145175000;}
			else if(pointInPolygon(5, New_Zealand, latitude, longitude) == 1)	{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144575000;}
			else if(pointInPolygon(5, Thailand, latitude, longitude) == 1)	{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 145525000;}
			else																{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144390000;}
		}
	}
	
	// shouldn't get here
	else
	{
																				{GEOFENCE_no_tx = 0; GEOFENCE_APRS_frequency = 144800000;}
	}
}
    

private:

    // GEOFENCE ARRAYS (longitude, latitude)
    static const int32_t ArgParUru[]   PROGMEM;
    static const int32_t Australia[]   PROGMEM;
    static const int32_t Brazil[]      PROGMEM;
    static const int32_t China[]       PROGMEM;
    static const int32_t CostNicPan[]  PROGMEM;
    static const int32_t France[]      PROGMEM;
    static const int32_t Japan[]       PROGMEM;
    static const int32_t Latvia[]      PROGMEM;
    static const int32_t New_Zealand[] PROGMEM;
    static const int32_t Romania[]     PROGMEM;
    static const int32_t South_Korea[] PROGMEM;
    static const int32_t Thailand[]    PROGMEM;
    static const int32_t UK[]          PROGMEM;
    static const int32_t Venezuela[]   PROGMEM;
};


// 680 bytes of data

const int32_t GeofenceAPRS::ArgParUru[] PROGMEM =
{
    -5779910,		-1867750,
    -5348140,		-2666710,
    -5713990,		-2977390,
    -4962520,		-3451560,
    -6024900,		-5856250,
    -7349850,		-5035950,
    -6754390,		-2143260,
    -5779910,		-1867750
};

const int32_t GeofenceAPRS::Australia[] PROGMEM =
{
    14756840,		-4692030,
    16602540,		-2915220,
    14414060,		-918890,
    9878910,		-1169530,
    11241210,		-3977480,
    14756840,		-4692030
};

const int32_t GeofenceAPRS::Brazil[] PROGMEM =
{
    -5704100,		-2976440,
    -4965820,		-3445220,
    -2830080,		-587830,
    -5150390,		430260,
    -6055660,		500340,
    -7417970,		-649000,
    -5774410,		-1856290,
    -5334960,		-2666710,
    -5704100,		-2976440
};
    
const int32_t GeofenceAPRS::China[] PROGMEM =
{
    8718750,		4938240,
    7163090,		3694990,
    8375980,		2814950,
    11039060,		1596130,
    12423340,		2198380,
    12392580,		3943620,
    13543950,		4673990,
    12172850,		5405940,
    10511720,		4209820,
    8718750,		4938240
};

const int32_t GeofenceAPRS::CostNicPan[] PROGMEM =
{
    -8876950,		1199630,
    -8020020,		480640,
    -7661870,		942740,
    -8270510,		1539010,
    -8876950,		1199630
};
    
const int32_t GeofenceAPRS::France[] PROGMEM =
{
    795410,			4876340,
    191160,			5094460,
    -364750,		4829780,
    -149410,		4334120,
    298830,			4256930,
    740480,			4378700,
    588870,			4645300,
    795410,			4876340
};
    
const int32_t GeofenceAPRS::Japan[] PROGMEM =
{
    13917480,		4582880,
    15209470,		4585940,
    14422850,		2340280,
    12471680,		2705910,
    13917480,		4582880
};

const int32_t GeofenceAPRS::Latvia[] PROGMEM =
{
    2664180,		5568380,
    2817990,		5620670,
    2778440,		5733250,
    2500490,		5800230,
    2414790,		5717200,
    2178590,		5768650,
    2081910,		5607200,
    2219240,		5644430,
    2568600,		5618230,
    2664180,		5568380
};
    
const int32_t GeofenceAPRS::New_Zealand[] PROGMEM =
{
    17999990,		-3778810,
    17999990,		-5522580,
    15433590,		-4582880,
    17226560,		-2876770,
    17999990,		-3778810
};

const int32_t GeofenceAPRS::Romania[] PROGMEM =
{
    2712520,		4795310,
    2295040,		4797520,
    2063230,		4585940,
    2324710,		4386620,
    2913570,		4385040,
    2712520,		4795310
};

const int32_t GeofenceAPRS::South_Korea[] PROGMEM =
{
    13493410,		4101310,
    12410160,		3880550,
    12552980,		2849770,
    13493410,		4101310
};

const int32_t GeofenceAPRS::Thailand[] PROGMEM =
{
    10579830,		1437080,
    10248050,		1409400,
    10252440,		607500,
    9806400,		594390,
    9775630,		2050940,
    10428220,		1822940,
    10579830,		1437080
};

const int32_t GeofenceAPRS::UK[] PROGMEM =
{
    -65920,			6097310,
    -758060,		5807790,
    -821780,		5423960,
    -476810,		5380070,
    -586670,		4976710,
    130740,			5085450,
    186770,			5278950,
    -204350,		5597380,
    -65920,			6097310
};

const int32_t GeofenceAPRS::Venezuela[] PROGMEM =
{
    -6665410,		18680,
    -6099610,		541910,
    -5952390,		938400,
    -7215820,		1249020,
    -7248780,		710090,
    -6796140,		572530,
    -6665410,		18680
};
































#endif  // __APRS_GEOFENCE_H__


