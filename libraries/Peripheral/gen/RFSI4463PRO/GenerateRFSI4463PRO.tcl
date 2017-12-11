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
#    puts "OnConfigBlockParsed"
#    puts "name: $name"
#    puts "cmd: $cmd"
#    puts "reqByteDefList: $reqByteDefList"
#    puts "repByteDefList: $repByteDefList"



    # Generate struct for reply

    set sep ""
    puts "struct ${name}_REP"
    puts "\{"
    foreach { byte fieldOffsetList } $repByteDefList {
        puts -nonewline $sep
        set sep "\n"

        puts "    struct"
        puts "    \{"
        foreach { field offset } $fieldOffsetList {
            puts "        uint8_t $field;"

            set varName "${byte}.$field"
        }
        puts "    \} ${byte};"
    }
    puts "\};"



    puts ""


    # Generate function to get reply from chip

    set bufSizeRep [GetByteCount $repByteDefList]


    puts "uint8_t Command_${name}(${name}_REP &retVal)"
    puts "\{"
    puts "    const uint8_t CMD_ID   = $cmd;"
    puts "    const uint8_t BUF_SIZE = $bufSizeRep;"
    puts ""
    puts "    uint8_t buf\[BUF_SIZE\];"
    puts ""
    puts "    uint8_t ok = SendAndWaitAndReceive(CMD_ID, buf, BUF_SIZE);"
    puts ""
    puts "    if (ok)"
    puts "    \{"
    puts "        BufferFieldExtractor bfe(buf, BUF_SIZE);"
    puts ""




    set sep ""
    for { set i 0 } { $i < $bufSizeRep } { incr i } {
        puts -nonewline $sep
        set sep "\n"

        set byte               [lindex $repByteDefList [expr ($i * 2) + 0]]
        set fieldOffsetList    [lindex $repByteDefList [expr ($i * 2) + 1]]
        set fieldOffsetListLen [llength $fieldOffsetList]

        set maxLen 0
        foreach { field offset } $fieldOffsetList {
            set fieldLen [string length $field]

            if { $fieldLen > $maxLen } {
                set maxLen $fieldLen
            }
        }
        set formatStr "%-${maxLen}s"

        #
        # Unpacking the data into struct fields can go one of 3 ways
        # - returned data has many bitfields per-byte
        # - returned data maps directly onto struct bytes
        # - returned data has multiple bytes to map onto struct bytes
        #

        # Check if this byte has multi-field elements
        # quick test, if there is more than one field in a byte, then yes


        if { $fieldOffsetListLen > 2 } {
            puts "        uint8_t tmpByte${i} = bfe.GetUI8();"            

            foreach { field offset } $fieldOffsetList {
                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                set bitMask  [GetBitmask $bitStart $bitEnd]
                set bitShift $bitEnd

                set str ""
                append str "        "
                append str "retVal.${byte}."
                append str [format $formatStr $field]
                append str " = "
                append str "(uint8_t)((tmpByte${i} & $bitMask) >> $bitShift);"

                puts $str
            }
        } else {
            # it's one of the other two

            foreach { field offset } $fieldOffsetList {
                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                set str ""
                append str "        "
                append str "retVal.${byte}."
                append str [format $formatStr $field]
                append str " = bfe.GetUI8();"            

                puts $str
            }


        }








    }
    puts "    \}"
    puts ""
    puts "    return ok;"
    puts "\}"




}

proc GetBitmask { bitStart bitEnd } {
    set str "0b"

    for { set i 7 } { $i >= 0 } { incr i -1 } {
        if { $bitEnd <= $i && $i <= $bitStart } {
            append str "1"
        } else {
            append str "0"
        }
    }

    return $str
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
























