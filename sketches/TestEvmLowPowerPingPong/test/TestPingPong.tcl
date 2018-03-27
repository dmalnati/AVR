#!/bin/sh
# \
exec tclsh "$0" "$@"


proc Run { startSecs stepSecs stopSecs cmd } {
    puts "secs,secsMeasured"
    flush stdout

    set fd [open "| $cmd" "w+"]

    set cont 1
    set secs $startSecs
    while { $cont } {
        set ms [expr $secs * 1000]

        puts -nonewline "$secs"

        puts $fd "ping $ms"
        flush $fd

        set line [gets $fd]
        set msStart [clock milliseconds]

        set line [gets $fd]
        set msStop [clock milliseconds]

        set msDiff    [expr $msStop - $msStart]
        set msDiffInt [expr int($msDiff / 1000)]
        set msDiffF   [format "%0.3f" [expr $msDiff / 1000.0]]

        puts ",$msDiffF"
        flush stdout

        set secs [expr $secs + $stepSecs]

        if { $stopSecs == "-" || $secs <= $stopSecs } {
            set cont 1
        } else {
            set cont 0
        }
    }

    catch {close $fd}
}

proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc < 4 } {
        Puts "Usage: $argv0 <startSecs> <stepSecs> <stopSecs> <cmdToRunToGetFdToReadAndWriteFrom>"
        exit -1
    }

    set startSecs [lindex $argv 0]
    set stepSecs  [lindex $argv 1]
    set stopSecs  [lindex $argv 2]
    set cmd       [lrange $argv 3 end]

    Run $startSecs $stepSecs $stopSecs $cmd
}


Main

