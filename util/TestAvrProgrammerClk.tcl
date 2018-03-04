#!/bin/sh
# \
exec tclsh "$0" "$@"


set AVRDUDE_CMD "/cygdrive/c/Program\\ Files\\ \\(x86\\)/Arduino/hardware/tools/avr/bin/avrdude.exe"
set AVRDUDE_CFG "C:\\\\Program Files (x86)\\\\Arduino\\\\hardware\\\\tools\\\\avr\\\\etc\\\\avrdude.conf"

set CHIP_ID "atmega328p"
set PROGRAMMER "usbtiny"



proc FixPath { path } {
    set retVal ""

    foreach { c } [split $path {}] {
        if { $c == "\\" || $c == "/" } {
            append retVal "\\\\"
        } else {
            append retVal $c
        }
    }

    return $retVal
}

proc GetSuccessVal { str } {
    set retVal 1

    set lineList [split $str "\n"]

    foreach { line } $lineList {
        if { [string first "verification error" $line] != -1 } {
            set retVal 0
        }
    }

    return $retVal
}

proc CmdMaker { hexFile clk } {
    global AVRDUDE_CMD
    global AVRDUDE_CFG
    global CHIP_ID
    global PROGRAMMER

    set cmd "$AVRDUDE_CMD -C \"$AVRDUDE_CFG\"         \
                          -p $CHIP_ID                 \
                          -c $PROGRAMMER              \
                          -U \"flash:w:${hexFile}:i\" \
                          -B $clk"

    return $cmd
}

proc Test { hexFile countEachClk clkList } {

    puts "Command format: [CmdMaker <hexFile> <clk>]"
    puts "Testing $countEachClk x [join $clkList {, }]"

    set hexFile [FixPath $hexFile]

    foreach { clk } $clkList {
        puts -nonewline "Testing $clk: "
        flush stdout

        if { $countEachClk } {
            set countOk 0

            set testsRemaining $countEachClk

            while { $testsRemaining } {
                set cmd [CmdMaker $hexFile $clk]

                # for some reason, avrdude doesn't return success code in this
                # mode, have to parse the text
                set fd [open "| $cmd" r]
                read $fd
                catch {close $fd} outputStr
                set retVal [GetSuccessVal $outputStr]

                if { $retVal } {
                    puts -nonewline "+"

                    incr countOk
                } else {
                    puts -nonewline "-"
                }
                flush stdout

                incr testsRemaining -1
            }

            set pctOk [expr { int(double($countOk) / $countEachClk * 100.0) }]

            puts ""
            puts "    $pctOk% ($countOk / $countEachClk)"
        }
    }

}

proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc < 3 } {
        puts "Usage: $argv0 <hexFile> <countEachClk> <clk1> \[<clk2> ...\]"
        exit -1
    }

    set hexFile      [lindex $argv 0]
    set countEachClk [lindex $argv 1]
    set clkList      [lrange $argv 2 end]

    Test $hexFile $countEachClk $clkList
}

Main
