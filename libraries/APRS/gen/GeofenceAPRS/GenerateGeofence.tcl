#!/bin/sh
# \
exec tclsh "$0" "$@"


#
#
# Should I convert to int32_t or int16_t?
# Clearly int16_t is smaller so desirable.
# What about resolution?
#
#
#
# Need to store latitude and longitude
# - lat range: - 90 to  90
# - lng range: -180 to 180
#
#
#
# int32_t
# - max 2,147,483,648
# - used in current code to represent deg * 10,000
# - so basically fixed point with 4 decimal places
# - so lat - 30.0881078 -> - 3,008,810
# - so lng  172.9687500 ->  17,296,875
#
# int16_t
# - max 32,768
# - multiply by 100
# - so lat - 30.0881078 -> - 3,008
# - so lng  172.9687500 ->  17,296
#
#
# Obstacles to going to int16_t:
# - loss of 3 or 4 decimal places of precision
#   - for lat/lng sub-100, it's a loss of 3
#   - for lat/lng 100+, it's a loss of 4
#   - what does that amount to?
#     - google maps shows around less than a mile difference (~3k ft)
#       - 38.695205, -95.228125 to 38.69, -95.22
#     - this is totally fine
#
#
# So - do it, encode as int16_t
#
#


set DATA [list]


proc OnPoint { name lat lng firstLine } {
    global DATA

    lappend DATA "point" $name [list $lat $lng] $firstLine
}

proc OnPath { name latLngList firstLine } {
    global DATA

    lappend DATA "path" $name $latLngList $firstLine
}

proc Analyze { } {
    global DATA

    set nameLenMax 0
    foreach { type name latLngList firstLine } $DATA {
        set len [string length $name]

        if { $len > $nameLenMax } {
            set nameLenMax $len
        }
    }

    set formatStr "%s - %-${nameLenMax}s - %2s points - %3s bytes"

    set bytesPoint 0
    set bytesPath  0

    foreach { type name latLngList firstLine } $DATA {
        if { $type == "point" } {
            set lat [lindex $latLngList 0]
            set lng [lindex $latLngList 1]

            set bytesUsed 4

            puts [format $formatStr "Point" $name 1 $bytesUsed]

            incr bytesPoint $bytesUsed
        } elseif { $type == "path" } {
            set numPoints [llength $latLngList]

            set  bytesUsed [expr $numPoints * 2]
            incr bytesUsed -4   ;# since we're eliminating the last point

            puts [format $formatStr "Path " $name $numPoints $bytesUsed]

            incr bytesPath $bytesUsed
        }
    }

    puts "Bytes Point: $bytesPoint"
    puts "Bytes Path : $bytesPath"
    puts "Bytes Total: [expr $bytesPoint + $bytesPath]"
}







proc EncodeLatLng { latLng } {
    return [expr { round($latLng * 100) }]
}

proc DecodeLatLng { latLngEncoded } {
    return [expr { double($latLngEncoded) / 100.0 }]
}




proc CreateModifiedInputFile { outModInputFile } {
    global DATA

    set fdOut stdout
    if { $outModInputFile != "-" } {
        set fdOut [open $outModInputFile w]
    }

    set headerLine "type	latitude	longitude	name	desc	color"

    foreach { type name latLngList firstLine } $DATA {

        # get the remainder of the string of the first line so we can reproduce
        # it just as it was
        set idxName      [string first $name $firstLine]
        set idxAfter     [expr $idxName + [string length $name]]
        set strAfterName [string range $firstLine $idxAfter end]


        puts $fdOut $headerLine

        if { $type == "point" } {
            set lat [DecodeLatLng [EncodeLatLng [lindex $latLngList 0]]]
            set lng [DecodeLatLng [EncodeLatLng [lindex $latLngList 1]]]

            puts $fdOut "W\t$lat\t$lng\t$name${strAfterName}"

        } elseif { $type == "path" } {
            puts $fdOut $name

            set firstLine 1
            foreach { lat lng } $latLngList {
                set lat [DecodeLatLng [EncodeLatLng $lat]]
                set lng [DecodeLatLng [EncodeLatLng $lng]]

                if { $firstLine } {
                    puts $fdOut "T\t$lat\t$lng\t$name${strAfterName}"
                    set firstLine 0
                } else {
                    puts $fdOut "T\t$lat\t$lng"
                }
            }
        }

        puts $fdOut ""
    }


    puts $fdOut ""

    if { $outModInputFile != "-" } {
        close $fdOut
    }
}








#
#
# Expected input format is a series of records.
# Can be single points "W" records
# Can be multiple points indicating a path
#
#
# type	latitude	longitude	name	desc	color
# W	9.0153023	-79.4970703	Panama		#0000ff
# 
# type	latitude	longitude	name	desc	color
# T	60.3269477	-4.2187500	Europe	Length: 16049 km (9973 mi)	#00cc00
# T	53.9560855	-13.6230469
# T	45.5217439	-4.9658203
# T	43.4529189	-13.0957031
# T	33.5047591	-7.4707031
# T	38.6511983	5.0097656
# T	34.3071439	34.1015625
# T	42.2935642	46.9335938
# T	57.1600783	22.1923828
# T	60.9090733	37.2216797
# T	67.6259544	20.7202148
# T	70.7144706	27.1911621
# T	71.8972381	20.2587891
# T	60.5437752	-3.9550781
# 
# type	latitude	longitude	name	desc	color
# T	51.5360856	-129.0673828	USA	Length: 15533 km (9652 mi)	#0000ff
# T	47.3388227	-124.5300293
# T	39.4022443	-125.0024414
# T	32.3242756	-118.6083984
#
#
# Our objective is to:
# - Generate code for the single points and a given radius
# - Generate a bounding box from the path
#   - The last point is just a visual placeholder from drawing.
#   - This point notionally connects the entire box together, but in practice
#     the web interface is a bit annoying and won't let you connect the path
#     end to the path start.
#   - Therefore just remove it.
#   - Later, the geofencing code will just assume that the remaining final point
#     connects to the start point.
#

proc Process { fdIn } {

    set firstLine  ""
    set name       ""
    set latLngList [list]

    set line [gets $fdIn]
    while { ![eof $fdIn] } {
        set linePartList $line
        set firstPart    [lindex $linePartList 0]

        if { $firstPart == "type" } {
            # header
        } elseif { $firstPart == "W" } {
            # single point

            set name [lindex $linePartList 3]
            set lat  [lindex $linePartList 1]
            set lng  [lindex $linePartList 2]

            set firstLine $line

            OnPoint $name $lat $lng $firstLine

            set name ""
        } elseif { $firstPart == "T" } {
            # path
            set nameTmp [lindex $linePartList 3]

            if { $nameTmp != "" } {
                set name $nameTmp

                set firstLine $line
            }

            set lat  [lindex $linePartList 1]
            set lng  [lindex $linePartList 2]

            lappend latLngList $lat $lng
        } elseif { $firstPart == "" } {
            # blank line between records

            if { $name != "" } {
                OnPath $name $latLngList $firstLine
            }

            set name       ""
            set latLngList [list]
            set firstLine  ""
        }

        set line [gets $fdIn]
    }
}


proc Generate { fdOut } {
    global DATA

    proc P { str } { upvar 1 fdOut fdOut; puts $fdOut $str }


    # Calculate max name length for pretty printing later
    set nameLenMax 0
    foreach { type name latLngList firstLine } $DATA {
        set len [string length $name]

        if { $len > $nameLenMax } {
            set nameLenMax $len
        }
    }


    # Generate code
    P "#ifndef __GEOFENCE_APRS_DATA_H__"
    P "#define __GEOFENCE_APRS_DATA_H__"
    P ""
    P ""
    P "#include \"GeofenceAPRSBoundingArea.h\""
    P ""
    P ""
    P "class GeofenceAPRSData"
    P "\{"
    P "    static const uint16_t POINT_RADIUS_MILES = 150;"
    P ""
    P "public:"

    set sep ""
    foreach { type name latLngList firstLine } $DATA {
        puts -nonewline $fdOut $sep

        if { $type == "point" } {

            set lat [EncodeLatLng [lindex $latLngList 0]]
            set lng [EncodeLatLng [lindex $latLngList 1]]

            P "    static uint8_t In${name}(int16_t latitude,\
                                            int16_t longitude)"
            P "    \{"
            P "        const int16_t POINT_LATITUDE  = $lat;"
            P "        const int16_t POINT_LONGITUDE = $lng;"
            P ""
            P "        uint8_t retVal = PointInCircle(latitude,"
            P "                                       longitude,"
            P "                                       POINT_LATITUDE,"
            P "                                       POINT_LONGITUDE,"
            P "                                       POINT_RADIUS_MILES);"
            P ""
            P "        return retVal;"
            P "    \}"
        } elseif { $type == "path" } {
            set latLngListTrunc [lrange $latLngList 0 end-2]
            set numPoints       [llength $latLngListTrunc]

            P "    static uint8_t In${name}(int16_t latitude,\
                                            int16_t longitude)"
            P "    \{"
            P "        // load into sram, making room for final connecting point which isn't"
            P "        // included in the data"
            P "        const uint8_t BUF_SIZE = $numPoints + 2;"
            P "        int16_t buf\[BUF_SIZE\];"
            P "        "
            P "        // use points to determine if point in polygon"
            P "        uint8_t retVal = LoadToSramAndCheckPointInPolygon(buf, $name, $numPoints, latitude, longitude);"
            P "        "
            P "        return retVal;"
            P "    \}"
        }

        set sep "\n"
    }

    P ""
    P ""
    P "private:"
    P ""

    P {    // single function to load values to sram as well as do the next step
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
}

    P {    // Takes care of pulling out values from eeprom, as well as populating
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
}

    P "    // GEOFENCE ARRAYS (longitude, latitude)"


    set formatStr "    static const int16_t %-${nameLenMax}s\[%2s\] PROGMEM;"

    foreach { type name latLngList firstLine } $DATA {
        if { $type == "point" } {
        } elseif { $type == "path" } {
            set numPoints [expr [llength $latLngList] - 2]

            P [format $formatStr $name $numPoints]
        }
    }

    P "\};"


    set formatStrDef \
        "const int16_t GeofenceAPRSData::%-s\[%s\] PROGMEM ="

    set formatStrData "    %6s, %6s,"

    set sep ""
    foreach { type name latLngList firstLine } $DATA {
        if { $type == "path" } {
            puts -nonewline $fdOut $sep

            set latLngListTrunc [lrange $latLngList 0 end-2]
            set numPoints       [llength $latLngListTrunc]

            P [format $formatStrDef $name $numPoints $numPoints]
            P "\{"
            foreach { lat lng } $latLngListTrunc {
                set lat [EncodeLatLng $lat]
                set lng [EncodeLatLng $lng]

                P [format $formatStrData $lng $lat]
            }
            P "\};"
        }

        set sep "\n"
    }


    P ""
    P ""
    P "#endif   // __GEOFENCE_APRS_DATA_H__"
}

proc AnalyzeAndGenerate { gpsVisTxtFile outGenCodeFile } {
    set fdIn [open $gpsVisTxtFile]

    Process $fdIn

    close $fdIn




    Analyze



    set fdOut stdout
    if { $outGenCodeFile != "-" } {
        set fdOut [open $outGenCodeFile w]
    }

    Generate $fdOut

    if { $outGenCodeFile != "-" } {
        close $fdOut
    }
}

proc GenerateIfElseChain { { debugOutput 0 } } {
    global DATA

    set prefix "if"
    foreach { type name latLngList firstLine } $DATA {
        puts "${prefix} (GeofenceAPRSData::In${name}(latitude, longitude))"
        puts "\{"
        if { $debugOutput } {
            puts "    Serial.println(F(\"In${name}\"));"
        }
        puts "\}"

        set prefix "else if"
    }

}


proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 2 && $argc != 3 && $argc != 4 } {
        puts "Usage: $argv0 <gpsVisTxtFile>  \
                            <outGenCodeFile> \
                            \[<outModInputFile>\]
                            \[-v\]"
        exit -1
    }

    set gpsVisTxtFile   [lindex $argv 0]
    set outGenCodeFile  [lindex $argv 1]
    set outModInputFile [lindex $argv 2]
    set verbose         [lindex $argv 3]

    AnalyzeAndGenerate $gpsVisTxtFile $outGenCodeFile

    if { $outModInputFile != "" } {
        CreateModifiedInputFile $outModInputFile
    }

    if { $verbose != "" } {
        if { $verbose == "-v" } {
            GenerateIfElseChain
        } elseif { $verbose == "-vv" } {
            set debugOutput 1
            GenerateIfElseChain $debugOutput
        }
    }
}

Main
























