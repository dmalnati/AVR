#!/bin/sh
# \
exec tclsh "$0" "$@"



# https://rietman.wordpress.com/2008/09/25/how-to-calculate-the-nmea-checksum/
#
# Calculating the checksum is very easy.
# It is the representation of two hexadecimal characters of an XOR of all 
# characters in the sentence between, but not including, 
# the $ and the * character.
#
# Lets assume the following NMEA sentence:
#
# $GPGLL,5300.97914,N,00259.98174,E,125926,A*28
# 
# In this sentence the checksum is the character representation of the 
# hexadecimal value 28. The string that the checksum is calculated over is
# 
# GPGLL,5300.97914,N,00259.98174,E,125926,A
# 
# To calculate the checksum you parse all characters between $ and * from the 
# NMEA sentence into a new string.  
#
# In the examples below the name of this new string is 
# stringToCalculateTheChecksumOver. 
#
# Then just XOR the first character with the next character, 
# until the end of the string.
#
#
proc GetChecksummedLine { line } {

    set idxStar [string first "*" $line]

    set subStr [string range $line 1 [expr $idxStar - 1]]

    set xorVal 0
    foreach { c } [split $subStr {}] {
        binary scan $c c cInt 

        set xorVal [expr $xorVal ^ $cInt]
    }

    set asHex [format "%02X" $xorVal]

    return "\$${subStr}*${asHex}"
}



proc ChecksumEachLine { inFile } {
    set fd stdin
    if { $inFile != "-" } {
        set fd [open $inFile "r"]
    }

    set line [gets $fd]
    while { ![eof $fd] } {
        if { $line != "" && [string index $line 0] != "#" } {
            set lineChecksummed [GetChecksummedLine $line]

            puts $lineChecksummed
        } else {
            puts $line
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

    ChecksumEachLine $inFile
}

Main

