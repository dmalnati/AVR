#!/bin/sh
# \
exec tclsh "$0" "$@"


set MAX_COL     16
set TABLE_SIZE 128


array set BUCKET__VAL [list]

proc DrawTable { lowerThresh } {
    global BUCKET__VAL
    global MAX_COL
    global TABLE_SIZE

    # clear
    MoveCursorToUpperLeft

    # show header
    puts -nonewline [format "%5s" ""]
    for { set i 0 } { $i < $MAX_COL } { incr i } {
        puts -nonewline [format "%4s" "+$i"]
    }
    puts ""

    set bucketsRemaining $TABLE_SIZE

    set rowStart 0
    set cont 1
    while { $cont } {
        puts -nonewline [format "%3s: " $rowStart]
        for { set i 0 } { $i < $MAX_COL } { incr i } {
            set val 0

            set bucket [expr $rowStart + $i]

            if { [info exists BUCKET__VAL($bucket)] } {
                set valMaybe $BUCKET__VAL($bucket)

                if { $valMaybe >= $lowerThresh && $valMaybe <= 255 } {
                    set val $valMaybe
                } else {
                    set val 0
                }
            }

            puts -nonewline [format "%4s" "$val"]
        }
        puts ""

        incr bucketsRemaining -$MAX_COL

        if { $bucketsRemaining <= 0 } {
            set cont 0
        }

        incr rowStart $MAX_COL
    }

}


proc WatchStdin { lowerThresh } {
    global BUCKET__VAL

    set fd stdin

    set line [gets $fd]
    while { ![eof $fd] } {
        if { $line == "" } {
            DrawTable $lowerThresh
        } else {
            set linePartList [split $line ":"]

            if { [llength $linePartList] == 2 } {
                set bucket [lindex $linePartList 0]
                set val [string trim [lindex $linePartList 1]]

                set BUCKET__VAL($bucket) $val
            }
        }

        set line [gets $fd]
    }
}

proc ClearScreen { } {
    exec clear >@ stdout
}

proc MoveCursorToUpperLeft { } {
    puts -nonewline "\x1b\[0;0f"
}

proc Analyze { lowerThresh } {
    ClearScreen

    DrawTable $lowerThresh

    WatchStdin $lowerThresh
}

proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 1 } {
        puts "Usage: $argv0 \[<lowerThresh>\]"
        exit -1
    }

    set lowerThresh [lindex $argv 0]

    if { $lowerThresh == "" } {
        set lowerThresh 50
    }

    Analyze $lowerThresh
}


Main










