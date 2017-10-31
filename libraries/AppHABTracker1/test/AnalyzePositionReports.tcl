#!/bin/sh
# \
exec tclsh "$0" "$@"


set FIELD_IDX_START_IDX_END_LIST {
    time               1  6
    latitude           8  15
    longitude         17  25
    courseDegrees     27  29
    speedMph          31  33
    altitudeFt        37  42
}


set FIELD_TYPE_LIST {
    barometricPressureMbar  u16
    tempF1                  u8
    magX                    i8
    magY                    i8
    magZ                    i8
    accelX                  i8
    accelY                  i8
    accelZ                  i8
    tempF2                  i8
    voltage                 u8
    seqNo                   u16
}

proc GetCsvColumnList { } {
    global FIELD_IDX_START_IDX_END_LIST
    global FIELD_TYPE_LIST

    set retVal [list]

    foreach { field idxStart idxEnd } $FIELD_IDX_START_IDX_END_LIST {
        lappend retVal $field
    }

    foreach { field type } $FIELD_TYPE_LIST {
        lappend retVal $field
    }

    return $retVal
}

proc Init { } {
    puts [join [GetCsvColumnList] ","]
}


proc ProcessPosReport { strAltReport } {
    if { [catch { ProcessPosReportCatchMe $strAltReport } err] } {
        puts "ERR strAltReport($strAltReport)"
    }
}

proc ProcessPosReportCatchMe { strAltReport } {
    global FIELD_IDX_START_IDX_END_LIST
    global FIELD_TYPE_LIST

    set valList [list]

    foreach { field idxStart idxEnd } $FIELD_IDX_START_IDX_END_LIST {
        set $field [string range $strAltReport $idxStart $idxEnd]

        upvar 0 $field val

        #puts "${field}($val)"

        lappend valList $val
    }

    set dataBinaryEncoded [string range $strAltReport 43 end]


    set dataBinaryEncodedWorking $dataBinaryEncoded
    foreach { field type } $FIELD_TYPE_LIST {
        set byteList [EnsureSize [split $dataBinaryEncodedWorking {}] $type]

        if { $type == "u16" } {
            set $field [BinaryDecodeU16 $byteList]
        } elseif { $type == "u8" } {
            set $field [BinaryDecodeU8 $byteList]
        } elseif { $type == "i8" } {
            set $field [BinaryDecodeI8 $byteList]
        }

        upvar 0 $field val

        #puts "${field}($val)"

        lappend valList $val

        # Remove used bytes
        set dataBinaryEncodedWorking \
            [RemoveLeadingBytes $dataBinaryEncodedWorking \
                                [EncodeTypeToEncodedByteLen $type]]
    }

    #puts [join $valList ","]



    set valListO [list]


    # Time
    set timeO [ProcessTime $time]
    #puts $timeO
    lappend valListO $timeO

    # latitude
    set latitudeO [ProcessLatLong $latitude]
    #puts $latitudeO
    lappend valListO $latitudeO

    # longitude
    set longitudeO [ProcessLatLong $longitude]
    #puts $longitudeO
    lappend valListO $longitudeO

    # courseDegrees (no change necessary)
    #puts $courseDegrees
    lappend valListO $courseDegrees

    # speedMph (actually in knots, not mph, but I fake the header for the csv)
    set speedMphO [ProcessSpeed $speedMph]
    #puts $speedMphO
    lappend valListO $speedMphO

    # altitudeFt (no change necessary)
    set altitudeFtO [ProcessAltitudeFt $altitudeFt]
    #puts $altitudeFtO
    lappend valListO $altitudeFtO

    # barometricPressureMbar (no change necessary)
    #puts $barometricPressureMbar
    lappend valListO $barometricPressureMbar

    # tempF1 (no change necessary)
    #puts $tempF1
    lappend valListO $tempF1

    # magX
    set magXO [ProcessMag $magX]
    #puts $magXO
    lappend valListO $magXO

    # magY
    set magYO [ProcessMag $magY]
    #puts $magYO
    lappend valListO $magYO

    # magZ
    set magZO [ProcessMag $magZ]
    #puts $magZO
    lappend valListO $magZO

    # accelX
    set accelXO [ProcessAccel $accelX]
    #puts $accelXO
    lappend valListO $accelXO

    # accelY
    set accelYO [ProcessAccel $accelY]
    #puts $accelYO
    lappend valListO $accelYO

    # accelZ
    set accelZO [ProcessAccel $accelZ]
    #puts $accelZO
    lappend valListO $accelZO

    # tempF2 (no change necessary)
    #puts $tempF2
    lappend valListO $tempF2

    # voltage
    set voltageO [ProcessVoltage $voltage]
    #puts $voltageO
    lappend valListO $voltageO

    # seqNo (no change necessary)
    #puts $seqNo
    lappend valListO $seqNo


    puts [join $valListO ","]

}

proc ProcessVoltage { voltage } {
    return [format "%.01f" [expr $voltage / 10.0]]
}

proc ProcessAccel { accel } {
    return [format "%.01f" [expr $accel / 10.0]]
}

proc ProcessMag { mag } {
    return [format "%.02f" [expr $mag / 100.0]]
}

proc ProcessAltitudeFt { alt } {
    return [string trimleft $alt "0"]
}

proc ProcessSpeed { speedKnots } {
    set KNOTS_TO_MPH 1.15078

    set speedKnots [string trimleft $speedKnots "0"]

    if { $speedKnots == "" } {
        set speedKnots 0
    }

    return [format "%.01f" [expr $speedKnots * $KNOTS_TO_MPH]]
}

#  4044.22N to  40 44 13.2
# 07402.03W to -74  2 18
#
# Output format is degrees minutes seconds
proc ProcessLatLong { latLong } {
    set idxDot [string first "." $latLong]

    set degs [string range $latLong 0                  [expr $idxDot - 3]]
    set mins [string range $latLong [expr $idxDot - 2] [expr $idxDot - 1]]
    set secs [string range $latLong [expr $idxDot]      end-1]
    set nsew [string range $latLong end end]

    set degs [string trimleft $degs 0]
    set mins [string trimleft $mins 0]
    set secs [format "%.01f" [expr $secs * 60]]

    if { $nsew == "W" } {
        set degs "-${degs}"
    }

    return "$degs $mins $secs"
}

# 032322 to 03:23:22
proc ProcessTime { time } {
    set timeO ""
    append timeO [string range $time 0 1]:
    append timeO [string range $time 2 3]:
    append timeO [string range $time 4 5]

    return $timeO
}

proc EnsureSize { byteList type } {
    set byteLen [EncodeTypeToEncodedByteLen $type]]

    set retVal $byteList

    while { [llength $retVal] < $byteLen } {
        lappend retVal " "
    }

    return $retVal
}

proc RemoveLeadingBytes { str byteLen } {
    set retVal [string range $str [expr $byteLen] end]

    return $retVal
}

proc BinaryDecodeI8 { byteList } {
    set val [BinaryDecodeU8 $byteList]

    set val [binary format s $val]
    binary scan $val c val

    return $val
}

proc BinaryDecodeU8 { byteList } {
    set be0 [GetU8FromByte [lindex $byteList 0]]
    set be1 [GetU8FromByte [lindex $byteList 1]]

    set b0 [expr [expr ($be0 - 32) * 16] + [expr $be1 - 32]]

    return $b0
}

proc BinaryDecodeU16 { byteList } {
    set b0 [BinaryDecodeU8 [lrange $byteList 0 1]]
    set b1 [BinaryDecodeU8 [lrange $byteList 2 3]]

    # AVR put the bytes in network byte order, which conveniently is the order
    # this computer is running, so just simplify and do manually
    set val [expr ($b0 * 256) + $b1]

    return $val
}

proc GetI8FromByte { b } {
    binary scan $b c val

    return $val
}

proc GetU8FromByte { b } {
    binary scan $b c val

    return [expr { $val & 0xff }]
}

proc EncodeTypeToEncodedByteLen { type } {
    set byteLen 0

    if { $type == "u16" } {
        set byteLen 4
    } elseif { $type == "u8" } {
        set byteLen 2
    } elseif { $type == "i8" } {
        set byteLen 2
    }

    return $byteLen
}

proc Test { } {
    set testStr \
        "/032322h4044.22N/07402.03WO123/000/A=000036\"')\$\$(.+ % ' ! */'\$*\"/"

    ProcessPosReport $testStr
}


# Searching for strings like this:
# 000000000011111111112222222222333333333344444444445555555555666666666
# 012345678901234567890123456789012345678901234567890123456789012345678
# /032322h4044.22N/07402.03WO123/000/A=000036"')$$(.+ % ' ! */'$*"/# #)
#
# Note that some terminals render <space> as no characters at all when
# highlighted.  Other renditions may also.
# In code in this program, spaces are assumed if at the end of data.
proc WatchStdin { } {
    set fd stdin

    set line [gets $fd]

    while { ![eof $fd] } {

        set idxAltitudeStart [string first "/A=" $line]
        if { $idxAltitudeStart != -1 } {
            set idxStart [expr $idxAltitudeStart - 34]

            set strAltReport [string range $line $idxStart end]

            ProcessPosReport $strAltReport
        }

        set line [gets $fd]
    }
}

proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 1 || [lindex $argv 0] != "-" } {
        puts "Usage: $argv0 -"
        exit -1
    }

    Init
    WatchStdin
    #Test
}


Main










