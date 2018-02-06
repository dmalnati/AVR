#!/bin/sh
# \
exec tclsh "$0" "$@"


# http://www.gpsinformation.org/dale/nmea.htm


proc TrimLeft { str chr } {
    set strNew [string trimleft $str $chr]

    if { $strNew == "" && $str != "" } {
        set strNew $chr
    }

    return $strNew
}


# $GPGSV,3,1,10,01,50,147,43,07,62,198,38,08,45,049,35,10,01,041,*74
proc StripNMEAMessage { str } {
    # Get rid of the leading $
    # Get rid of the checksum

    return [string range $str 1 end-3]
}



set       GSA_SEEN     0
array set GPS          [list]
array set PRN__DATA    [list]
set       PRN_LIST_NEW [list]
set       UPDATE_COUNT 0
set       FIRST_TIME   1





proc ClearScreen { } {
    exec clear >@ stdout
}

proc MoveCursorToUpperLeft { } {
    puts -nonewline "\x1b\[0;0f"
}



proc OnUpdate { } {
    global GPS
    global PRN_LIST_NEW
    global UPDATE_COUNT
    global FIRST_TIME

    if { $FIRST_TIME } {
        set FIRST_TIME 0

        ClearScreen
    }

    MoveCursorToUpperLeft
    incr UPDATE_COUNT

    puts "Updates     : $UPDATE_COUNT                                         "
    puts "Fix Type    : $GPS(FIX_SELECTION)                                   "
    puts "Fix State   : $GPS(FIX_STATE) ($GPS(PDOP) $GPS(HDOP) $GPS(VDOP))    "
    puts "Sats in view: $GPS(SATS_IN_VIEW)                                    "

    puts "Fix vs See:"
    set prnListFix [lsort -dictionary $GPS(FIX_PRN_LIST)]
    set prnListSee [lsort -dictionary $PRN_LIST_NEW]
    puts [format "Fix (%02s): %s                        " \
        [llength $prnListFix] $prnListFix]
    puts [format "See (%02s): %s                        " \
        [llength $prnListSee] $prnListSee]

    puts -nonewline "Fix: "
    set lim 72
    for { set i 0 } { $i < $lim } { incr i } {
        if { [lsearch -exact $prnListFix $i] != -1 } {
            puts -nonewline "."
        } else {
            puts -nonewline " "
        }
    }
    puts ""

    puts -nonewline "See: "
    set lim 72
    for { set i 0 } { $i < $lim } { incr i } {
        if { [lsearch $prnListSee $i] != -1 } {
            puts -nonewline "."
        } else {
            puts -nonewline " "
        }
    }
    puts ""

    puts ""
    puts ""
}


proc OnGSA { linePartList } {
    global GSA_SEEN
    global GPS

    set GPS(FIX_SELECTION) [lindex $linePartList 1]
    set GPS(FIX_STATE)     [lindex $linePartList 2]

    set GPS(FIX_PRN_LIST)  [list]
    foreach { prn } [lrange $linePartList 3 14] {
        set prnTrim [TrimLeft $prn 0]

        if { $prnTrim != "" } {
            lappend GPS(FIX_PRN_LIST) $prnTrim
        }
    }

    set GPS(PDOP)          [lindex $linePartList 15]
    set GPS(HDOP)          [lindex $linePartList 16]
    set GPS(VDOP)          [lindex $linePartList 17]

    set GSA_SEEN 1
}


proc OnGSV { linePartList } {
    global GSA_SEEN
    global GPS
    global PRN__DATA
    global PRN_LIST_NEW

    set linePartListLen [llength $linePartList]

    # Update PRN data
    foreach { prn elev azim snr } [lrange $linePartList 4 end] {
        set prnTrim [TrimLeft $prn 0]

        set PRN__DATA($prnTrim) [list $elev $azim $snr]

        lappend PRN_LIST_NEW $prnTrim
    }

    # Check if this is the last update in the batch
    set numSentencesNeeded [lindex $linePartList 1]
    set sentenceNum        [lindex $linePartList 2]

    set satsInView [TrimLeft [lindex $linePartList 3] 0]

    if { $numSentencesNeeded == $sentenceNum } {
        set GPS(SATS_IN_VIEW) $satsInView

        if { $GSA_SEEN } {
            set GSA_SEEN 0

            OnUpdate
        }

        set PRN_LIST_NEW [list]
    }
}


proc Monitor { inFile } {
    set fd stdin
    if { $inFile != "-" } {
        set fd [open $inFile "r"]
    }

    set line [gets $fd]
    while { ![eof $fd] } {
        set line         [StripNMEAMessage $line]
        set linePartList [split $line ","]

        puts $line

        set msgType [lindex $linePartList 0]

        if { $msgType == "GPGSA" } {
            OnGSA $linePartList
        } elseif { $msgType == "GPGSV" } {
            OnGSV $linePartList
        }

        set line [gets $fd]
    }

    if { $inFile != "-" } {
        close $fd
    }
}

proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 1 } {
        puts "Usage: $argv0 <file> (file can be -)"
        exit -1
    }

    set inFile [lindex $argv 0]

    ClearScreen
    Monitor $inFile
}

Main

