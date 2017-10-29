#!/bin/sh
# \
exec tclsh "$0" "$@"


set FIELD_IDX_START_IDX_END_LIST {
    time               1  6
    latitude           8  15
    longitude         17  25
    courseDegrees     27  29
    speedKnots        31  33
    altitudeFt        37  42
}


set FIELD_TYPE_LIST {
    barometricPressure  u16
    temp1               u8
    magX                i8
    magY                i8
    magZ                i8
    accelX              i8
    accelY              i8
    accelZ              i8
    temp2               i8
    voltage             u8
    seqNo               u16
}

proc GetFieldList { } {
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
    puts [join [GetFieldList] ","]
}


proc ProcessAltReport { strAltReport } {
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

    puts [join $valList ","]
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

    ProcessAltReport $testStr
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

            ProcessAltReport $strAltReport
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










