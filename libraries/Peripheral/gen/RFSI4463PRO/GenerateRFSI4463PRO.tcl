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
    GenerateDebugCode $name $cmd $reqByteDefList $repByteDefList
    GenerateCode      $name $cmd $reqByteDefList $repByteDefList
}

proc GenerateDebugCode { name cmd reqByteDefList repByteDefList } {
    # Nice header
    gend "////////////////////////////////////////////////////////////////////"
    gend "//"
    gend "// ${name} ($cmd)"
    gend "//"
    gend "////////////////////////////////////////////////////////////////////"
    gend ""

    # Determine field padding
    set maxLen [GetMaxFieldLen $repByteDefList]
    set formatStr "%-${maxLen}s"

    # Calculate number of bytes received
    set byteCount [GetByteCount $repByteDefList]

    # Generate code
    gend "void Print(RFSI4463PRO::${name}_REP &val)"
    gend "\{"
    gend "    Serial.println(\"${name}_REP\");"
    gend ""

    for { set i 0 } { $i < $byteCount } { incr i } {
        set byte               [lindex $repByteDefList [expr ($i * 2) + 0]]
        set fieldOffsetList    [lindex $repByteDefList [expr ($i * 2) + 1]]
        set fieldOffsetListLen [llength $fieldOffsetList]

        foreach { field offset } $fieldOffsetList {
            set varName "${byte}.$field"
            set varStr [format $formatStr $varName]

            gend "    Serial.print(\"${varStr}: \");"
            gend "    Serial.print(val.${varName});"
            gend "    Serial.print(\" (0x\");"
            gend "    Serial.print(val.${varName}, HEX);"
            gend "    Serial.print(\")\");"
            gend "    Serial.println();"
        }
    }

    gend "\}"

    gend ""
}

proc GetMaxFieldLen { byteDefList } {
    set maxLen 0

    foreach { byte fieldOffsetList } $byteDefList {

        foreach { field offset } $fieldOffsetList {
            set name    "${byte}.${field}"
            set nameLen [string length $name]

            if { $nameLen > $maxLen } {
                set maxLen $nameLen
            }
        }
    }

    return $maxLen
}


proc GenerateCode { name cmd reqByteDefList repByteDefList } {
#    puts "OnConfigBlockParsed"
#    puts "name: $name"
#    puts "cmd: $cmd"
#    puts "reqByteDefList: $reqByteDefList"
#    puts "repByteDefList: $repByteDefList"



    # Nice header
    gen "////////////////////////////////////////////////////////////////////"
    gen "//"
    gen "// ${name} ($cmd)"
    gen "//"
    gen "////////////////////////////////////////////////////////////////////"
    gen ""


    # Generate struct for reply

    set sep ""
    gen "struct ${name}_REP"
    gen "\{"
    foreach { byte fieldOffsetList } $repByteDefList {
        gen -nonewline $sep
        set sep "\n"

        gen "    struct"
        gen "    \{"
        foreach { field offset } $fieldOffsetList {
            gen "        uint8_t $field;"

            set varName "${byte}.$field"
        }
        gen "    \} ${byte};"
    }
    gen "\};"



    gen ""


    # Generate function to get reply from chip

    set byteCount [GetByteCount $repByteDefList]


    gen "uint8_t Command_${name}(${name}_REP &retVal)"
    gen "\{"
    gen "    const uint8_t CMD_ID   = $cmd;"
    gen "    const uint8_t BUF_SIZE = $byteCount;"
    gen ""
    gen "    uint8_t buf\[BUF_SIZE\];"
    gen ""
    gen "    uint8_t ok = SendAndWaitAndReceive(CMD_ID, buf, BUF_SIZE);"
    gen ""
    gen "    if (ok)"
    gen "    \{"
    gen "        BufferFieldExtractor bfe(buf, BUF_SIZE);"
    gen ""



    # Determine field padding
    set maxLen [GetMaxFieldLen $repByteDefList]
    set formatStr "%-${maxLen}s"


    # Generate code
    set sep ""
    for { set i 0 } { $i < $byteCount } { incr i } {
        gen -nonewline $sep
        set sep "\n"

        set byte               [lindex $repByteDefList [expr ($i * 2) + 0]]
        set fieldOffsetList    [lindex $repByteDefList [expr ($i * 2) + 1]]
        set fieldOffsetListLen [llength $fieldOffsetList]

        #
        # Unpacking the data into struct fields can go one of 3 ways
        # - returned data has many bitfields per-byte
        # - returned data maps directly onto struct bytes
        # - returned data has multiple bytes to map onto struct bytes
        #

        # Check if this byte has multi-field elements
        # quick test, if there is more than one field in a byte, then yes


        if { $fieldOffsetListLen > 2 } {
            gen "        uint8_t tmpByte${i} = bfe.GetUI8();"            

            foreach { field offset } $fieldOffsetList {
                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                set bitMask  [GetBitmask $bitStart $bitEnd]
                set bitShift $bitEnd

                set str ""
                append str "        "
                append str "retVal."
                append str [format $formatStr "${byte}.${field}"]
                append str " = "
                append str "(uint8_t)((tmpByte${i} & $bitMask) >> $bitShift);"

                gen $str
            }
        } else {
            # it's one of the other two

            foreach { field offset } $fieldOffsetList {
                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                set str ""
                append str "        "
                append str "retVal."
                append str [format $formatStr "${byte}.${field}"]
                append str " = bfe.GetUI8();"            

                gen $str
            }


        }








    }
    gen "    \}"
    gen ""
    gen "    return ok;"
    gen "\}"



    gen ""
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




set FD_GEN_OUT     stdout
set FD_GEN_DBG_OUT stdout
proc SetupOutputFiles { outGenFile outGenDebugFile } {
    global FD_GEN_OUT
    global FD_GEN_DBG_OUT

    set FD_GEN_OUT     [open $outGenFile      "w"]
    set FD_GEN_DBG_OUT [open $outGenDebugFile "w"]
}

proc gen { args } {
    global FD_GEN_OUT

    set arg1 [lindex $args 0]

    if { $arg1 == "-nonewline" } {
        set str [lindex $args 1]

        puts -nonewline $FD_GEN_OUT "$str"
        puts -nonewline "$str"
    } else {
        set str $arg1

        puts $FD_GEN_OUT $str
        puts $str
    }
}

proc gend { args } {
    global FD_GEN_DBG_OUT

    set arg1 [lindex $args 0]

    if { $arg1 == "-nonewline" } {
        set str [lindex $args 1]

        puts -nonewline $FD_GEN_DBG_OUT "$str"
        puts -nonewline "$str"
    } else {
        set str $arg1

        puts $FD_GEN_DBG_OUT $str
        puts $str
    }
}


proc Main { } {
    global argc
    global argv
    global argv0

    if { $argc != 3 } {
        puts "Usage: $argv0 <inFile> <outGenFile> <outGenDebugFile>"
        exit -1
    }

    set inputFile       [lindex $argv 0]
    set outGenFile      [lindex $argv 1]
    set outGenDebugFile [lindex $argv 2]

    SetupOutputFiles $outGenFile $outGenDebugFile

    Generate $inputFile
}

Main
























