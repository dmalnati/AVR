#!/bin/sh
# \
exec tclsh "$0" "$@"




proc CountCharInString { char str } {
    set count 0

    foreach { c } [split $str {}] {
        if { $c == $char } {
            incr count
        }
    }

    return $count
}

proc ElementsExist { arr_name args } {
    upvar 1 $arr_name arr

    set retVal 1

    foreach { key } $args {
        set retVal [expr $retVal && [info exists arr($key)]]
    }

    return $retVal
}



proc GetList { fd } {
    set braceImbalance 1

    set str ""

    set line [gets $fd]
    set cont [expr ![eof $fd]]
    while { $cont } {
        set leftBraceCount  [CountCharInString "\{" $line]
        set rightBraceCount [CountCharInString "\}" $line]

        incr braceImbalance $leftBraceCount
        incr braceImbalance -$rightBraceCount
        
        # sanitize input to avoid square brackets
        regsub -all {\[} $line " " line
        regsub -all {\]} $line " " line


        if { $braceImbalance == 0 } {
            set cont 0
        } else {
            append str "$line\n"
        }

        if { $cont } {
            set line [gets $fd]

            set cont [expr ![eof $fd]]
        }
    }

#    puts "str($str)"
#
#    foreach { byte fieldOffsetList } $str {
#        puts "byte: $byte"
#
#        foreach { field offset } $fieldOffsetList {
#            puts "  $field $offset"
#        }
#    }

    return $str
}


proc Generate { inputFile } {
    set fd [open $inputFile]

    set state "BUILDING_LIST"

    set strToList ""

    array set name__val [list]


    set line [gets $fd]

    while { ![eof $fd] } {
        set line [string trimright $line]

        set firstChar [string index $line 0]

        if { $firstChar == "#" || $line == "" } {
            # ignore
        } else {
            set linePartList [split $line " "]

            set firstWord [lindex $linePartList 0]

            if { $firstWord == "REQ" } {
                set name__val(REQ) [GetList $fd]
            } elseif { $firstWord == "REP" } {
                set name__val(REP) [GetList $fd]
            } elseif { $firstWord == "CMD" } {
                set name__val(CMD) [lindex $line 1]
            } else {
                set name__val(NAME) [lindex $line 0]
            }
        }

        if { [ElementsExist name__val NAME CMD REQ REP] } {
            OnConfigBlockParsed $name__val(NAME) \
                                $name__val(CMD) \
                                $name__val(REQ) \
                                $name__val(REP)

            unset name__val(NAME)
            unset name__val(CMD)
            unset name__val(REQ)
            unset name__val(REP)
        }

        set line [gets $fd]
    }

    close $fd
}




proc GetByteCount { byteDefList } {
    return [expr [llength $byteDefList] / 2]
}

proc OnConfigBlockParsed { name cmd reqByteDefList repByteDefList } {
    puts "OnConfigBlockParsed"
    puts "name: $name"
    puts "cmd: $cmd"
    puts "reqByteDefList: $reqByteDefList"
    puts "repByteDefList: $repByteDefList"



    # Generate struct for reply

    set maxVarNameLen 0

    set sep ""
    puts "struct ${name}_REP"
    puts "\{"
    foreach { byte fieldOffsetList } $repByteDefList {
        puts $sep
        set sep "\n"

        puts "    struct ${byte}"
        puts "    \{"
        foreach { field offset } $fieldOffsetList {
            puts "        uint8_t $field;"

            set varName "${byte}.$field"
            set varNameLen [string length $varName]
            if { $varNameLen > $maxVarNameLen } {
                set maxVarNameLen $varNameLen
            }
        }
        puts "    \};"
    }
    puts "\};"



    puts ""


    # Generate function to get reply from chip

    set bufSizeRep [GetByteCount $repByteDefList]
    set formatStr  "%-${maxVarNameLen}s"


    puts "uint8_t Command_${name}(${name}_REP &retVal)"
    puts "\{"
    puts "    const uint8_t BUF_SIZE = $bufSizeRep;"
    puts "    uint8_t       buf\[BUF_SIZE\];"
    puts ""
    puts "    const uint8_t CMD_ID = $cmd;"
    puts ""
    puts "    uint8_t ok = SendAndWaitAndReceive(CMD_ID, buf, BUF_SIZE);"
    puts ""
    puts "    if (ok)"
    puts "    \{"
    puts "        BufferFieldExtractor bfe(buf, BUF_SIZE);"
    puts ""
    foreach { byte fieldOffsetList } $repByteDefList {
        foreach { field offset } $fieldOffsetList {
            puts "        retVal.[format $formatStr ${byte}.${field}] = bfe.GetUI8();"
        }
    }
    puts "\};"
    puts "    \}"
    puts ""
    puts "    return ok;"
    puts "\}"




}






proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 1 } {
        puts "Usage: $argv0 <inputFile>"
        exit -1
    }

    set inputFile [lindex $argv 0]

    Generate $inputFile
}

Main
























