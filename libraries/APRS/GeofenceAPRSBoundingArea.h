#ifndef __GEOFENCE_APRS_BOUNDING_AREA_H__
#define __GEOFENCE_APRS_BOUNDING_AREA_H__



// Taken from TT7 and modified for 16-bit latitude/longitude, plus a few
// cosmetic changes
//
// http://tt7hab.blogspot.com/2017/02/aprs-automatic-packet-reporting-system.html
// https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_GEOFENCE.c
// https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_GEOFENCE.h
//

/*
	Adapted version of pointInPolygon() function from:	http://alienryderflex.com/polygon/
	
	Returns '0' if the point is outside of the polygon and '1' if it's inside.
	
	Expects input DEGREES * 100000 for latitude and longitude. Eg 4961070 for 49.61070 N.
	The reason is to make sure all calculations fit inside int32_t.
	
	However, this function is not very accurate due to rounding within the computation.
*/
uint8_t PointInPolygon(int16_t        latitude,
                       int16_t        longitude,
                       const int16_t *polygon,
                       uint8_t        polyCorners)
{
	uint8_t j = polyCorners * 2 - 2;
	int32_t oddNodes = 0;
    
	for (uint8_t i = 0; i < polyCorners * 2; i += 2)
	{
        int16_t n1Lng = polygon[i + 0];
        int16_t n1Lat = polygon[i + 1];
        
        int16_t n2Lng = polygon[j + 0];
        int16_t n2Lat = polygon[j + 1];
        
        uint8_t cond1 = (n1Lat < latitude && n2Lat >= latitude);
        uint8_t cond2 = (n2Lat < latitude && n1Lat >= latitude);
        uint8_t cond3 = (n1Lng <= longitude || n2Lng <= longitude);
        
        if ((cond1 || cond2) && cond3)
		{
            // this line is the integer version which basically doesn't work in
            // a lot of scenarios
			//oddNodes ^= (n1Lng + (latitude - n1Lat) / (n2Lat - n1Lat) * (n2Lng - n1Lng) < longitude);

            // this line is the floating point version which does work, but
            // costs an extra 1000 bytes progmem
			oddNodes ^= (n1Lng + (double)(latitude - n1Lat) / (double)(n2Lat - n1Lat) * (double)(n2Lng - n1Lng) < longitude);
		}

		j = i;
	}

	return !!oddNodes;
}



// This page covers the approx distance between each degree of
// latitude and longitude
// https://www.thoughtco.com/degree-of-latitude-and-longitude-distance-4070616
//
// Longitude varies by where you are on earth.  I'm taking a value which is
// correct at around +/- 40degs, which means:
// - for locations closer to the poles   - could be incorrectly outside radius
// - for locations closer to the equator - could be incorrectly inside  radius
//
// This works in our case since we're going to be targeting random island and
// small points in the "incorrectly inside" region, which is ok, because
// they're small and we'd rather hit them with luck than miss them.
//
// Not even doing a real circle calculation, just boxing it in.
static uint8_t PointInCircle(int16_t  latitude,
                             int16_t  longitude,
                             int16_t  circLatitude,
                             int16_t  circLongitude,
                             uint16_t circRadiusMiles)
{
    const uint16_t MILES_PER_DEGREE_LAT = 69;
    const uint16_t MILES_PER_DEGREE_LNG = 50;
    
    int16_t diffLat = latitude  - circLatitude;
    int16_t diffLng = longitude - circLongitude;
    
    int16_t diffLatDegs = diffLat / 100;
    int16_t diffLngDegs = diffLng / 100;
    
    uint16_t diffLatMiles = MILES_PER_DEGREE_LAT * abs(diffLatDegs);
    uint16_t diffLngMiles = MILES_PER_DEGREE_LNG * abs(diffLngDegs);
    
    uint8_t retVal = (diffLatMiles < circRadiusMiles &&
                      diffLngMiles < circRadiusMiles);

    return retVal;
}



#endif  // __GEOFENCE_APRS_BOUNDING_AREA_H__



