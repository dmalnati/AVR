#!/bin/sh
# \
exec tclsh "$0" "$@"



proc KmlStart { } {
    puts {<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
    <Document>
    }
}


proc KmlEnd { } {
    puts {    </Document>
</kml>
    }
}



proc KmlAddStyle { id lineColor lineWidth polyColor } {
    puts "    <Style id=\"${id}\">"
    puts "      <LineStyle>"
    puts "        <color>${lineColor}</color>"
    puts "        <width>${lineWidth}</width>"
    puts "      </LineStyle>"
    puts "      <PolyStyle>"
    puts "        <color>${polyColor}</color>"
    puts "      </PolyStyle>"
    puts "    </Style>"
}


proc KmlAddPlacemarkPoint { name desc lat lng } {
    puts "        <Placemark>"
    puts "            <name>$name</name>"
    puts "            <description>$desc</description>"
    puts "            <Point>"
    puts "                <coordinates>"
    puts "                    ${lng},${lat},0"
    puts "                </coordinates>"
    puts "            </Point>"
    puts "        </Placemark>"
}

proc KmlAddPlacemarkLineString { name desc latLngAltList { styleId "" } { extrude 0 } { tessellate 0 } } {
    puts "        <Placemark>"
    puts "            <name>$name</name>"
    puts "            <description>$desc</description>"
    if { $styleId != "" } {
        puts "            <styleUrl>#${styleId}</styleUrl>"
    }
    puts "            <LineString>"
    puts "                <extrude>${extrude}</extrude>"
    puts "                <tessellate>${tessellate}</tessellate>"
#    puts "                <altitudeMode>absolute</altitudeMode>"
    puts "                <coordinates>"
    foreach { lat lng alt } $latLngAltList {
        set altMeters [expr double($alt) / 3.28084]

        puts "${lng},${lat},${altMeters}"
    }
    puts "                </coordinates>"
    puts "            </LineString>"
    puts "        </Placemark>"
}






proc CreateKml { spotList showSpots } {
    KmlStart

    KmlAddStyle "spot"       "ffffffff"  2 "ff000000"
    KmlAddStyle "flightPath" "ff0000ff" 30 "ff000000"

    # Create primary track showing path traveled
    set latLngAltList [list]

    foreach { spot } $spotList {

        # just one line, now broken out
        foreach { txId txLat txLng txAlt rxId rxLat rxLng rxAlt } $spot {}

        # keep track of each location of TX for plotting later
        lappend latLngAltList $txLat $txLng $txAlt

        # Create line between this point in flight and ground station
        set latLngAltSkyToGroundList [list $txLat $txLng $txAlt $rxLat $rxLng $rxAlt]

        if { $showSpots } {
            KmlAddPlacemarkLineString "Sky to $rxId" "Sky to $rxId" $latLngAltSkyToGroundList "spot" 0 1
        }
    }

    KmlAddPlacemarkLineString "Flight Path" "Flight Path" $latLngAltList "flightPath" 1

    KmlEnd
}






proc CmpLine { valListA valListB } {
    set tsA "[lindex $valListA 0] [lindex $valListA 1]"
    set tsB "[lindex $valListB 0] [lindex $valListB 1]"

    return [string compare $tsA $tsB]
}

proc GetSpotList { } {
    global SPOT_LIST

    return $SPOT_LIST
}

# For each line
# your own: id lat lng alt
# receiver: id lat lng alt
#
# Data looks like this
#
#  2018-05-11 18:38 	 KD2KDD 	 14.097204 	 -11 	 0 	 GN49 	 2 	 AA3GZ 	 FN20kh 	 2140 	 251 
#  2018-05-11 18:34 	 KD2KDD 	 14.097205 	 -11 	 0 	 GN49 	 2 	 AA3GZ 	 FN20kh 	 2140 	 251 
#
#
# grids can be 4 or 6 (probably less than 4 also but let's assume for now)
#
set SPOT_LIST [list]
proc ProcessData { } {
    global DATA
    global SPOT_LIST

    set DATA [lsort -command CmpLine $DATA]

    foreach { valList } $DATA {
        set txId    [lindex $valList 2]
        set txGrid  [lindex $valList 6]
        set txPower [lindex $valList 7]
        set rxId    [lindex $valList 8]
        set rxGrid  [lindex $valList 9]

        set txLatLngList [GridToLatLng $txGrid]
        set txAlt        [PowerToAlt $txPower]
        set rxLatLngList [GridToLatLng $rxGrid]

        #puts "${txGrid}($txLatLngList) ${txPower}($txAlt) ${rxGrid}($rxLatLngList)"

        set txLat [lindex $txLatLngList 0]
        set txLng [lindex $txLatLngList 1]

        set rxLat [lindex $rxLatLngList 0]
        set rxLng [lindex $rxLatLngList 1]

        lappend SPOT_LIST [list $txId $txLat $txLng $txAlt $rxId $rxLat $rxLng 0]
    }
}

proc PowerToAlt { power } {
    set ftDbmWattList {
             0   0     0.001
          2222   3     0.002
          4444   7     0.005
          6667  10     0.01
          8889  13     0.02
         11111  17     0.05
         13333  20     0.1
         15556  23     0.2
         17778  27     0.5
         20000  30     1
         22222  33     2
         24444  37     5
         26667  40    10
         28889  43    20
         31111  47    50
         33333  50   100
         35556  53   200
         37778  57   500
         40000  60  1000
    }

    set alt 0

    foreach { ft dbm watt } $ftDbmWattList {
        if { $power == $watt } {
            set alt $ft
        }
    }

    return $alt
}

proc GridToLatLng { grid } {
    set gridLen [string length $grid]

    set lat 0
    set lng 0

    # Get the value of the ASCII character 'A' and '0'
    binary scan "A" c aInt
    binary scan "0" c zeroInt

    if { $gridLen >= 2 } {
        set lngChar [string index $grid 0]
        set latChar [string index $grid 1]

        binary scan $lngChar c lngCharInt
        set lng [expr ($lngCharInt - $aInt) * 200000]

        binary scan $latChar c latCharInt
        set lat [expr ($latCharInt - $aInt) * 100000]
    }

    if { $gridLen >= 4 } {
        set lngChar [string index $grid 2]
        set latChar [string index $grid 3]

        binary scan $lngChar c lngCharInt
        set lng [expr $lng + (($lngCharInt - $zeroInt) * 20000)]

        binary scan $latChar c latCharInt
        set lat [expr $lat + (($latCharInt - $zeroInt) * 10000)]
    }

    if { $gridLen >= 6 } {
        set lngChar [string index $grid 4]
        set latChar [string index $grid 5]

        binary scan $lngChar c lngCharInt
        set lng [expr $lng + (($lngCharInt - $aInt) * 834)]

        binary scan $latChar c latCharInt
        set lat [expr $lat + (($latCharInt - $aInt) * 417)]
    }

    # bring out of 10-thousandths
    set lat [expr double($lat) / 10000]
    set lng [expr double($lng) / 10000]

    # Do final conversion into gps cooridnates
    set lng [expr $lng - 180]
    set lat [expr $lat -  90]


    return [list $lat $lng]
}


set DATA [list]
proc OnLineData { valList } {
    global DATA

    lappend DATA $valList
}


#
#  Timestamp	Call	MHz	SNR	Drift	Grid	Pwr	Reporter	RGrid	km	az
#  2018-05-11 18:38 	 KD2KDD 	 14.097204 	 -11 	 0 	 GN49 	 2 	 AA3GZ 	 FN20kh 	 2140 	 251 
#  2018-05-11 18:34 	 KD2KDD 	 14.097205 	 -11 	 0 	 GN49 	 2 	 AA3GZ 	 FN20kh 	 2140 	 251 
#  2018-05-11 16:52 	 KD2KDD 	 14.097236 	 -22 	 0 	 GN38 	 5 	 VE1VDM 	 FN85ij 	 850 	 250 
#  2018-05-11 16:50 	 KD2KDD 	 14.097245 	 -16 	 3 	 GN38 	 5 	 VE1VDM 	 FN85ij 	 850 	 250 
#  2018-05-11 16:44 	 KD2KDD 	 14.097238 	 -12 	 1 	 GN28 	 5 	 VE1VDM 	 FN85ij 	 714 	 244
#
#  notionally tab-separated values
#
#  sadly, if you:
#  - copy from wsprnet into notepad, values look like above
#  - copy from wsprnet into google docs, then from google docs to notepad, you
#    will see each value on a line of its own.
#
#  We will handle both scenarios here.
#
proc ProcessFile { inputFile } {
    set fd [open $inputFile "r"]

    set valList [list]

    set line [gets $fd]
    while { ![eof $fd] } {
        set linePartList $line

        set partList $linePartList

        set cont 1
        while { $cont } {
            if { [llength $partList] } {
                # copy head element from partList
                lappend valList [lindex $partList 0]

                # remove head element from partList
                set partList [lreplace $partList 0 0]

                # Check if we have enough values to process a line
                if { [llength $valList] == 12 } {

                    # exclude the header line if present
                    if { [lindex $valList 0] != "Timestamp" } {
                        OnLineData $valList
                    }

                    set valList [list]
                }
            } else {
                set cont 0
            }
        }

        set line [gets $fd]
    }

    if { [llength $valList] } {
        puts "ERR -- values remain in valList($valList)"
    }

    close $fd
}







proc Process { inputFile showSpots } {
    ProcessFile $inputFile
    ProcessData
    CreateKml [GetSpotList] $showSpots
}












proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 1 && $argc != 2 } {
        puts "Usage: "
        puts "    $argv0 <inputFile> \[-noSpots\]"
        exit -1
    }

    set inputFile   [lindex $argv 0]
    set noSpotsFlag [lindex $argv 1]

    set showSpots 1
    if { $noSpotsFlag == "-noSpots" } {
        set showSpots 0
    }

    Process $inputFile $showSpots
}

Main
























