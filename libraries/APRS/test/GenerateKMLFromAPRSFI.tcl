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
    puts "                <altitudeMode>absolute</altitudeMode>"
    puts "                <coordinates>"
    foreach { lat lng alt } $latLngAltList {
        set altMeters [expr double($alt) / 3.28084]

        puts "${lng},${lat},${altMeters}"
    }
    puts "                </coordinates>"
    puts "            </LineString>"
    puts "        </Placemark>"
}






proc CreateKml { msgList } {
    set msgList [lsort -command CmpMsg $msgList]

    KmlStart

    KmlAddStyle "flightPath" "ff0000ff" 20 "ff000000"

    # Create points for each ground station
    set pathList [GetPathList]

    foreach { path } $pathList {
        set latLngList [GetPathLatLngList $path]

        foreach { lat lng } $latLngList {
#            KmlAddPlacemarkPoint $path $path $lat $lng
        }
    }


    # Create primary track showing path traveled
    set latLngAltList [list]

    foreach { msg } $msgList {
        set rawLatLngAltList [MsgGetRawLatLngAlt $msg]

        foreach { rawLat rawLng alt } $rawLatLngAltList { }

        set lat [GetLatOrLngFromDM $rawLat]
        set lng [GetLatOrLngFromDM $rawLng]

        lappend latLngAltList $lat $lng $alt


        # Create line between this point in flight and first ground station
        set pathList [MsgGetPathList $msg]
        set firstPath [lindex $pathList 0]

        set latLngList [GetPathLatLngList $firstPath]

        set latLngAltSkyToGroundList [list $lat $lng $alt [lindex $latLngList 0] [lindex $latLngList 1] 0]

        KmlAddPlacemarkLineString "Sky to $firstPath" "Sky to $firstPath" $latLngAltSkyToGroundList
    }

    KmlAddPlacemarkLineString "Flight Path" "Flight Path" $latLngAltList "flightPath" 1





    KmlEnd
}





























# 2017-11-24 11:04:13 EST: KD2KDD-9>APZ001,K3ARL-5,WIDE1*,qAR,N3DXC:/160410h4016.85N/07637.94WO067/015/A=003093 #(%"%,*/)/! !   *#-#$  #)
#
#
#
# 2017-11-24 12:23:43 EST: KD2KDD-9>APZ001,KB2WAC-1,W2RGI-1,KA2QYE-10,WIDE2*,DISCVR,qAR,N1ATP-13:/172306h4012.64N/07541.31WO078/063/A=080399  !*/%+!! .- !   )"+#" !!.
#
# 2017-11-24 11:20:56 EST: KD2KDD-9>APZ001,AD3O-2,WIDE1*,qAR,KC2WBX-3:/162020h4018.19N/07631.00WO091/022/A=017368 " ,!#**"!/( !// )$"#"  &( [Delayed or out-of-order packet (timestamp)]
#
#
# 2017-11-24 11:23:12 EST: KD2KDD-9>APZ001,WC3XS-3*,WIDE1*,qAR,K3TOW:/162020h4018.19N/07631.00WO091/022/A=017368 " ,!#**"!/( !// )$"#"  &( [Rate limited (< 5 sec)]
#
#
# 2017-11-24 11:23:30 EST: KD2KDD-9>APZ001,WIDE1-1,qAR,NR3I:/162328h4018.18N/07629.12WO091/033/A=020346 !,. #-( (/! !   )$"#"  '! [Location changes too fast (adaptive limit)]
#
#
# 2017-11-24 11:39:35 EST: KD2KDD-9>APZ001,KB2WAC-1,KB2FAF-1,KC2ANT-1,KD2MKR*,DISCVR,qAR,K2ZRO-1:/163858h4017.03N/07620.26WO126/027/A=034608  /---,'# /" !   )#+#"  ). [Delayed or out-of-order packet (timestamp)]
#
#
# 2017-11-24 11:57:50 EST: KD2KDD-9>APZ001,KB2WAC-1*,KB2FAF-1*,KC2ANT-1*,KD2MKR*,DISCVR,qAR,N2PYI-2:/165716h4013.61N/07605.38WO104/042/A=053327  &*-*++!..* !   *# ##  -# [Delayed or out-of-order packet (timestamp)]
#
#
# 2017-11-24 11:59:42 EST: KD2KDD-9>APZ001,WIDE1-1,qAR,VE3OSH:/165940h4013.36N/07603.12WO099/047/A=055699  %,-$-' -.- !   *".#$  -*
#
#
# 2017-11-24 13:04:44 EST: KD2KDD-9>APZ001,KB2WAC-1,KB2FAF-1,KC2ANT-1,W2QYT-1,WIDE2*,DISCVR,qAR,WN8Z-4:/180410h4018.35N/07446.56WO090/025/A=020997 !,$/-,&! .' ! ! +"'## !)% [Delayed or out-of-order packet (timestamp)]
#
#
#
# https://ham.stackexchange.com/questions/6213/help-understanding-path-taken-by-aprs-packet
#
# General Format of paths taken:
# sender>hop1,hop2,hopn*,qAR,lastReceiverWhoIGatedIt
#
# notably hop1 is the original direct receiver
#
#
# Look at a given path:
# KD2KDD-9>APZ001,K3ARL-5,WIDE1*,qAR,N3DXC
#
# K3ARL-5 received directly
# WIDE1 (just ignore I guess, seems to vary whether still left intact)
# * means next paths are the endpoint
# qAR means packet was successfully Igated
# N3DXC igated it (after receiving from last hop before *)
#
#
# Another example path:
# KD2KDD-9>APZ001,WIDE1-1,qAR,NR3I
#
# NR3I received it directly and Igated it
#
#
# Another example path:
# KD2KDD-9>APZ001,KB2WAC-1*,KB2FAF-1*,KC2ANT-1*,KD2MKR*,DISCVR,qAR,N2PYI-2
#
# ... ok I guess multiple * can exist.  that's a lot of hops.
#
#
#
#
#
#
#
# Also note that the timestamps in the file are the aprs.fi timestamps, not the message timestamps.
# Be sure to sort by message timestamp.
#
#
#








# 4019.05N
#
# 40 degrees 19.05 minutes, north
#
proc GetLatOrLngFromDM { rawLatLng } {

    set idxDot [string first "." $rawLatLng]

    set idxStartMin [expr $idxDot - 2]
    set idxEndMin   [expr $idxDot + 2]

    set min  [string range $rawLatLng $idxStartMin $idxEndMin]
    set deg  [string range $rawLatLng 0 [expr $idxStartMin - 1]]
    set nsew [string index $rawLatLng end]

    set latLng [expr $deg + (double($min) / 60)]

    if { $nsew == "S" || $nsew == "W" } {
        set latLng "-$latLng"
    }

#    puts "rawLatLng($rawLatLng)"
#    puts "latLng($latLng)"
#    puts ""

    return $latLng
}



# 0000000000111111111122222222223333333333444
# 0123456789012345678901234567890123456789012
#
# /173348h4015.69N/07524.79WO069/103/A=092625
proc MsgGetRawLatLngAlt { msg } {
    set idxFirstSlash [string first "/" $msg]

    set subStr [string range $msg $idxFirstSlash end]

    set lat [string range $subStr  8 15]
    set lng [string range $subStr 17 25]
    set alt [string range $subStr 37 42]

    # despite being "raw" we're going to strip any leading zeroes

    set lat [string trimleft $lat "0"]
    set lng [string trimleft $lng "0"]
    set alt [string trimleft $alt "0"]

    set latLngAltList [list $lat $lng $alt]

#    puts "subStr($subStr)"
#    puts "latLngAltList($latLngAltList)"
#    puts ""

    return $latLngAltList
}


# KD2KDD-9>APZ001,WC3XS-3*,WIDE1*,qAR,K3TOW:/173348h4015.69N/07524.79WO069/103/A=092625
proc MsgGetPathList { msg } {
    set pathList [list]

    set idxFirstRightComma [string first "," $msg]
    set idxFirstColon      [string first ":" $msg]


    set subStr [string range $msg \
                             [expr $idxFirstRightComma + 1] \
                             [expr $idxFirstColon - 1]]

    set pathListTmp [split $subStr ","]

    array set ul_excludeList [list]

    foreach { path } $pathListTmp {
        set path [string trimright $path "*"]

        # eliminate goofy stuff seen, including:
        #   6 exclude(1)
        #   6 exclude(WIDE2)
        #   6 exclude(WIDE2-1)
        #   9 exclude(qAS)
        #  14 exclude(qAo)
        #  21 exclude(WIDE2-2)
        #  39 exclude(DISCVR)
        # 145 exclude(WIDE1)
        # 402 exclude(WIDE1-1)
        # 680 exclude(qAR)

        if { [string first "WIDE" $path] == -1 &&
             $path != "DISCVR"                 &&
             [string length $path] >= 4 } {
            lappend pathList $path
        }
    }

#    puts "subStr($subStr)"
#    puts "pathList($pathList)"

    return $pathList
}


proc CmpMsg { m1 m2 } {
    set t1 [MsgGetTime $m1]
    set t2 [MsgGetTime $m2]

    set cmpVal [string compare $t1 $t2]

    set retVal 0

    if { $cmpVal < 0 } {
        set retVal -1
    } elseif { $cmpVal == 0 } {
        set retVal 0
    } else {
        set retVal 1
    }

    return $retVal
}


#
# 01234567
#
# /175408h
#
proc MsgGetTime { msg } {
    set idxFirstSlash [string first "/" $msg]

    set idxTimeStart [expr $idxFirstSlash + 1]
    set idxTimeEnd   [expr $idxFirstSlash + 6]

    set timeStr [string range $msg $idxTimeStart $idxTimeEnd]

#    puts "time($timeStr) <- msg($msg)"

    return $timeStr
}


#
# Two forms:
#
# 00000000001111111111222222
# 01234567890123456789012345
#
# 2017-11-24 12:40:44 EST: <msg>
# 2017-11-24 12:40:44 EST: <msg> [<errstr>]
# 
proc GetMsgFromLine { line } {
    set idxStart 25
    set idxEnd   [expr [string length $line] - 1]
    set idxLastBracket [string last "\]" $line]

    if { $idxLastBracket != -1 } {
        set idxFirstBracket [string last "\[" $line $idxLastBracket]

        set idxEnd [expr $idxFirstBracket - 2]
    }

    set msg [string range $line $idxStart $idxEnd]

#    puts "line($line)"
#    puts "msg($msg)"
#    puts ""

    return $msg
}

proc GetMsgList { inputFile } {
    set fd [open $inputFile]

    set line [gets $fd]

    set msgList [list]

    while { ![eof $fd] } {
        set msg [GetMsgFromLine $line]

        lappend msgList $msg

        set line [gets $fd]
    }

    close $fd

    return $msgList
}







proc GetPathLatLngList { path } {
    global PATH__LAT_LNG_LIST

    set pathLatLngList [list 0 0]

    if { [info exists PATH__LAT_LNG_LIST($path)] } {
        set pathLatLngList $PATH__LAT_LNG_LIST($path)
    }

    return $pathLatLngList
}

proc GetPathList { } {
    global PATH__LAT_LNG_LIST

    set pathList [lsort -dictionary [array names PATH__LAT_LNG_LIST]]

    return $pathList
}

array set PATH__LAT_LNG_LIST [list]
proc CachePathLatLngFile { pathLatLngFile } {
    global PATH__LAT_LNG_LIST

    set fd [open $pathLatLngFile]
    set lineList [split [read $fd] "\n"]
    close $fd

    foreach { line } $lineList {
        foreach { path lat lng } [split $line ","] {
            set PATH__LAT_LNG_LIST($path) [list $lat $lng]
        }
    }
}

proc Process { inputFile pathLatLngFile } {
    set msgList [GetMsgList $inputFile]

    CachePathLatLngFile $pathLatLngFile
    CreateKml $msgList
}











#
# https://aprs.fi/info/?call=<path>
#
# https://aprs.fi/info/?call=W2GSA-10
#
#
# Look for the following line.  It has both "Location" and "locator" on it"
#
# <tr><th valign='top'>Location:</th> <td valign='top'>40°15.44' N 74°05.50' W - locator <a rel='nofollow'
#
#
# >40°15.44' N 74°05.50' W - locat
#
proc FetchPathLatLng { path } {
    set latLngList [list 0 0]

    set url "https://aprs.fi/info/?call=${path}"

    set cmd "| wget -O - $url"

    set fd [open $cmd r]
    set lineList [split [read $fd] "\n"]
    catch {close $fd}

    foreach { line } $lineList {
        set idxLocator [string first "locator" $line]

        if { [string first "Location" $line] != -1 &&
             $idxLocator != -1 } {
            
#            puts "line($line)"

            set idxNsew [expr $idxLocator - 4]
            set nsew [string range $line $idxNsew $idxNsew]

            set idxMin [expr $idxNsew - 7]
            set min [string range $line $idxMin [expr $idxMin + 4]]

            set idxDeg [expr [string last " " $line $idxMin] + 1]
            set deg [string range $line $idxDeg [expr $idxMin - 2]]

            set rawLng "${deg}${min}${nsew}"

#            puts "nsew($nsew)"
#            puts "min($min)"
#            puts "deg($deg)"
#            puts "rawLng($rawLng)"



            set idxNsew [expr $idxDeg - 2]
            set nsew [string range $line $idxNsew $idxNsew]

            set idxMin [expr $idxNsew - 7]
            set min [string range $line $idxMin [expr $idxMin + 4]]

            set idxDeg [expr [string last ">" $line $idxMin] + 1]
            set deg [string range $line $idxDeg [expr $idxMin - 2]]

            set rawLat "${deg}${min}${nsew}"

#            puts "nsew($nsew)"
#            puts "min($min)"
#            puts "deg($deg)"
#            puts "rawLat($rawLat)"


            set lat [GetLatOrLngFromDM $rawLat]
            set lng [GetLatOrLngFromDM $rawLng]

#            puts "lat($lat)"
#            puts "lng($lng)"

            set latLngList [list $lat $lng]
        }
    }

    return $latLngList
}

proc FetchPathDataAndSerializeLatLngFile { msgList pathLatLngFile } {
    set fd [open $pathLatLngFile w]

    array set path__latLngList [list]

    foreach { msg } $msgList {
        set pathList [MsgGetPathList $msg]

        foreach { path } $pathList {
            if { ![info exists path__latLngList($path)] } {
                puts -nonewline "$path ... "

                set latLngList [FetchPathLatLng $path]

                puts $latLngList

                set path__latLngList($path) $latLngList
            }
        }
    }

    set pathList [lsort -dictionary [array names path__latLngList]]

    foreach { path } $pathList {
        foreach { lat lng } $path__latLngList($path) {
            puts $fd "${path},${lat},${lng}"
        }
    }

    close $fd
}

proc MakeLatLngFile { inputFile pathLatLngFile } {
    set msgList [GetMsgList $inputFile]

    FetchPathDataAndSerializeLatLngFile $msgList $pathLatLngFile
}


proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 2 && $argc != 3 } {
        puts "Usage: "
        puts "    $argv0 <inputFile> <pathLatLngFile.csv>"
        puts "    $argv0 <inputFile> -makePathLatLngFile <pathLatLngFile.csv>"
        exit -1
    }

    set inputFile [lindex $argv 0]

    if { $argc == 2 } {
        set pathLatLngFile [lindex $argv 1]

        Process $inputFile $pathLatLngFile
    } elseif { $argc == 3 } {
        set pathLatLngFile [lindex $argv 2]

        MakeLatLngFile $inputFile $pathLatLngFile
    }
}

Main
























