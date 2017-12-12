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

    return $str
}


proc Generate { inputFile } {
    set fd [open $inputFile]

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
    set maxLen [GetMaxByteAndFieldLen $repByteDefList]
    set formatStr "%-${maxLen}s"

    # Calculate number of bytes received
    set byteCount [GetByteCount $repByteDefList]

    # Generate code
    gend "void Print(RFSI4463PRO::${name}_REP &val)"
    gend "\{"
    gend "    Serial.println(\"${name}_REP\");"
    gend ""

    set varNameLast ""
    for { set i 0 } { $i < $byteCount } { incr i } {
        set byte               [lindex $repByteDefList [expr ($i * 2) + 0]]
        set fieldOffsetList    [lindex $repByteDefList [expr ($i * 2) + 1]]
        set fieldOffsetListLen [llength $fieldOffsetList]

        foreach { field offset } $fieldOffsetList {
            set varName "${byte}.$field"
            set varStr [format $formatStr $varName]

            if { $varName != $varNameLast } {
                gend "    Serial.print(\"${varStr}: \");"
                gend "    Serial.print(val.${varName});"
                gend "    Serial.print(\" (0x\");"
                gend "    Serial.print(val.${varName}, HEX);"
                gend "    Serial.print(\")\");"
                gend "    Serial.println();"
            }

            set varNameLast $varName
        }
    }

    gend "\}"

    gend ""
}

proc GetMaxFieldLen { byteDefList } {
    set maxLen 0

    foreach { byte fieldOffsetList } $byteDefList {
        foreach { field offset } $fieldOffsetList {
            set name    ${field}
            set nameLen [string length $name]

            if { $nameLen > $maxLen } {
                set maxLen $nameLen
            }
        }
    }

    return $maxLen
}

proc GetMaxByteAndFieldLen { byteDefList } {
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
    # Nice header
    gen "////////////////////////////////////////////////////////////////////"
    gen "//"
    gen "// ${name} ($cmd)"
    gen "//"
    gen "////////////////////////////////////////////////////////////////////"
    gen ""


    ################################################################
    #
    # Generate Request Struct Code
    #
    ################################################################


    # Look at response size
    set byteCountReq [GetByteCount $reqByteDefList]

    if { $byteCountReq } {
        # Figure out field widths
        set maxFieldLen [GetMaxFieldLen $reqByteDefList]
        set formatStr "%-${maxFieldLen}s"

        # Generate struct for reply
        set varNameLast ""
        set sep ""
        gen "struct ${name}_REQ"
        gen "\{"
        for { set i 0 } { $i < $byteCountReq } { incr i } {
            set byte               [lindex $reqByteDefList [expr ($i * 2) + 0]]
            set fieldOffsetList    [lindex $reqByteDefList [expr ($i * 2) + 1]]
            set fieldOffsetListLen [llength $fieldOffsetList]

            gen -nonewline $sep
            set sep "\n"

            if { $fieldOffsetListLen > 2 } {
                gen "    struct"
                gen "    \{"
                foreach { field offset } $fieldOffsetList {
                    gen "        uint8_t [format $formatStr $field] = 0;"

                    set varName "${byte}.$field"
                }
                gen "    \} ${byte};"
            } else {
                set field  [lindex $fieldOffsetList 0]
                set offset [lindex $fieldOffsetList 1]

                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                if { $bitStart > 7 } {
                    set bitWidth [expr $bitStart + 1]
                    set varName "${byte}.$field"

                    gen "    struct"
                    gen "    \{"
                    gen "        uint${bitWidth}_t [format $formatStr $field]\
                                 = 0;"
                    gen "    \} ${byte};"

                    set varNameLast $varName
                } else {
                    set varName "${byte}.$field"
                    if { $varName != $varNameLast } {
                        gen "    struct"
                        gen "    \{"
                        gen "        uint8_t [format $formatStr $field] = 0;"
                        set varName "${byte}.$field"
                        gen "    \} ${byte};"
                    } else {
                        set sep ""
                    }
                }
            }
        }
        gen "\};"

        gen ""
    }





    ################################################################
    #
    # Generate Reply Struct Code
    #
    ################################################################


    # Look at response size
    set byteCountRep [GetByteCount $repByteDefList]

    if { $byteCountRep } {
        # Figure out field widths
        set maxFieldLen [GetMaxFieldLen $repByteDefList]
        set formatStr "%-${maxFieldLen}s"

        # Generate struct for reply
        set varNameLast ""
        set sep ""
        gen "struct ${name}_REP"
        gen "\{"
        for { set i 0 } { $i < $byteCountRep } { incr i } {
            set byte               [lindex $repByteDefList [expr ($i * 2) + 0]]
            set fieldOffsetList    [lindex $repByteDefList [expr ($i * 2) + 1]]
            set fieldOffsetListLen [llength $fieldOffsetList]

            gen -nonewline $sep
            set sep "\n"

            if { $fieldOffsetListLen > 2 } {
                gen "    struct"
                gen "    \{"
                foreach { field offset } $fieldOffsetList {
                    gen "        uint8_t [format $formatStr $field] = 0;"

                    set varName "${byte}.$field"
                }
                gen "    \} ${byte};"
            } else {
                set field  [lindex $fieldOffsetList 0]
                set offset [lindex $fieldOffsetList 1]

                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                if { $bitStart > 7 } {
                    set bitWidth [expr $bitStart + 1]
                    set varName "${byte}.$field"

                    gen "    struct"
                    gen "    \{"
                    gen "        uint${bitWidth}_t [format $formatStr $field]\
                                 = 0;"
                    gen "    \} ${byte};"

                    set varNameLast $varName
                } else {
                    set varName "${byte}.$field"
                    if { $varName != $varNameLast } {
                        gen "    struct"
                        gen "    \{"
                        gen "        uint8_t [format $formatStr $field] = 0;"
                        set varName "${byte}.$field"
                        gen "    \} ${byte};"
                    } else {
                        set sep ""
                    }
                }
            }
        }
        gen "\};"

        gen ""
    }





    ################################################################
    #
    # Generate Command Req Rep Code
    #
    ################################################################

    # Generate function to get reply from chip
    gen -nonewline "uint8_t Command_${name}("
    if { $byteCountReq } {
        gen -nonewline "${name}_REQ &req"
    }
    if { $byteCountRep } {
        if { $byteCountReq } {
            gen -nonewline ", "
        }

        gen -nonewline "${name}_REP &rep"
    }
    gen ")"
    gen "\{"
    gen "    const uint8_t CMD_ID       = $cmd;"
    gen "    const uint8_t BUF_SIZE_REQ = $byteCountReq;"
    gen "    const uint8_t BUF_SIZE_REP = $byteCountRep;"
    gen ""
    gen "    uint8_t bufReq\[BUF_SIZE_REQ\];"
    gen "    uint8_t bufRep\[BUF_SIZE_REP\];"
    gen ""

    if { $byteCountReq } {
        gen "    // pack request data into buffer"

        # Determine field padding
        set maxLen [GetMaxByteAndFieldLen $reqByteDefList]
        set formatStr "%-${maxLen}s"

        # Generate code
        set varNameLast ""
        set sep ""
        for { set i 0 } { $i < $byteCountReq } { incr i } {
            gen -nonewline $sep
            set sep "\n"

            set byte               [lindex $reqByteDefList [expr ($i * 2) + 0]]
            set fieldOffsetList    [lindex $reqByteDefList [expr ($i * 2) + 1]]
            set fieldOffsetListLen [llength $fieldOffsetList]

            #
            # Packing the data into struct fields can go one of 3 ways
            # - data has many bitfields per-byte
            # - data maps directly onto struct bytes
            # - data has multiple bytes to map onto struct bytes
            #

            # Check if this byte has multi-field elements
            # quick test, if there is more than one field in a byte, then yes

            if { $fieldOffsetListLen > 2 } {
                gen "    uint8_t tmpReqByte${i} = 0;"            

                foreach { field offset } $fieldOffsetList {
                    set offsetPartList [split $offset ":"]

                    set bitStart [lindex $offsetPartList 0]
                    set bitEnd   [lindex $offsetPartList 1]

                    set bitMask  [GetBitmask [expr $bitStart - $bitEnd] 0]
                    set bitShift $bitEnd

                    set str ""
                    append str "    "
                    append str "tmpReqByte${i} |= "
                    append str "(uint8_t)((req."
                    append str [format $formatStr "${byte}.${field}"]
                    append str " & $bitMask) << $bitShift);"

                    gen $str
                }

                gen "    bufReq\[$i\] = tmpReqByte${i};"
            } else {
                # it's one of the other two
                set field  [lindex $fieldOffsetList 0]
                set offset [lindex $fieldOffsetList 1]

                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                if { $bitStart > 7 } {
                    set bitWidth [expr $bitStart + 1]
                    set fnSuffix "s"
                    if { $bitWidth > 16 } {
                        set fnSuffix "l"
                    }
                    set varName "${byte}.$field"


                    set tmpVarName \
                        "tmpReqByte${i}_[expr $i + ($bitWidth / 8) - 1]"

                    set str ""
                    append str "    "
                    append str "uint${bitWidth}_t $tmpVarName = "
                    append str "PAL.hton${fnSuffix}(req.$varName);"
                    gen $str

                    set str ""
                    append str "    "
                    append str "memcpy(&bufReq\[$i\],"
                    append str " &$tmpVarName, sizeof($tmpVarName));"
                    gen $str

                    set varNameLast $varName
                } else {
                    set varName "${byte}.$field"

                    if { $varName != $varNameLast } {
                        set str ""
                        append str "    "
                        append str "bufReq\[$i\] = "
                        append str "req."
                        append str [format $formatStr "${byte}.${field}"]
                        append str ";"

                        gen $str
                    } else {
                        set sep ""
                    }
                }
            }
        }

        gen ""
    }



    set str ""
    append str "    "
    append str "uint8_t ok = SendAndWaitAndReceive(CMD_ID, "
    append str "bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);"
    gen $str
    gen ""

    if { $byteCountRep } {
        gen "    if (ok)"
        gen "    \{"
        gen "        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);"
        gen ""

        # Determine field padding
        set maxLen [GetMaxByteAndFieldLen $repByteDefList]
        set formatStr "%-${maxLen}s"

        # Generate code
        set varNameLast ""
        set sep ""
        for { set i 0 } { $i < $byteCountRep } { incr i } {
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
                    append str "rep."
                    append str [format $formatStr "${byte}.${field}"]
                    append str " = "
                    append str "(uint8_t)((tmpByte${i} & $bitMask) "
                    append str ">> $bitShift);"

                    gen $str
                }
            } else {
                # it's one of the other two
                set field  [lindex $fieldOffsetList 0]
                set offset [lindex $fieldOffsetList 1]

                set offsetPartList [split $offset ":"]

                set bitStart [lindex $offsetPartList 0]
                set bitEnd   [lindex $offsetPartList 1]

                if { $bitStart > 7 } {
                    set bitWidth [expr $bitStart + 1]
                    set fnSuffix "S"
                    if { $bitWidth > 16 } {
                        set fnSuffix "L"
                    }
                    set varName "${byte}.$field"

                    set str ""
                    append str "        "
                    append str "rep."
                    append str [format $formatStr "${byte}.${field}"]
                    append str " = bfe.GetUI${bitWidth}NTOH${fnSuffix}();"

                    gen $str

                    set varNameLast $varName
                } else {
                    set varName "${byte}.$field"

                    if { $varName != $varNameLast } {
                        set str ""
                        append str "        "
                        append str "rep."
                        append str [format $formatStr "${byte}.${field}"]
                        append str " = bfe.GetUI8();"            

                        gen $str
                    } else {
                        set sep ""
                    }
                }
            }
        }

        gen "    \}"
        gen ""
    }

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
























