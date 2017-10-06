#!/bin/sh
# \
exec tclsh "$0" "$@"



set AVR_BIN_DIR \
    "/cygdrive/c/Program\\ Files\\ \\(x86\\)/Arduino/hardware/tools/avr/bin"

set AVR_BUILD_DIR_ROOT "/cygdrive/c/Users/doug/AppData/Local/Temp"


proc GetMostRecentBuildDir { } {
    global AVR_BUILD_DIR_ROOT

    #set contentList [exec -- ls -rt $AVR_BUILD_DIR_ROOT | grep ^build]
    set contentList [exec -- ls -rt $AVR_BUILD_DIR_ROOT | grep arduino_build]

    return [lindex $contentList end]
}

proc GetAvrNmOutput { } {
    global AVR_BIN_DIR
    global AVR_BUILD_DIR_ROOT
    global VERBOSE

    cd $AVR_BUILD_DIR_ROOT/[GetMostRecentBuildDir]

    set cmd ""

    append cmd "$AVR_BIN_DIR/avr-nm.exe "
    append cmd "-C --size-sort --print-size -td "
    append cmd "*.elf"

    if { $VERBOSE } {
        puts ""
        puts "cd $AVR_BUILD_DIR_ROOT/[GetMostRecentBuildDir]"
        puts $cmd
    }

    set output ""
    set fd [open "| $cmd"]
    set output [read -nonewline $fd]
    close $fd

    return $output
}




#
# nm output
#
# addr      size    flag  symbol
#
#
# 08389204 00000001 V Ivm::GetInstance()::ivm
# 08389267 00000001 b timer0_fract
# 08389276 00000002 B __brkval
# 00005912 00000002 t __empty
# 08389278 00000002 B __flp
# 08388864 00000002 D __malloc_heap_end
# 08388866 00000002 D __malloc_heap_start
# 08388868 00000002 D __malloc_margin
# 00004962 00000002 t __tcf_0
# 00003590 00000002 t __tcf_0
# 08389202 00000002 B Evm::evm_
# 00005876 00000002 W initVariant
# 00001422 00000002 T setup
# 00005912 00000002 W yield
# 00000222 00000010 T port_to_output_PGM
# 00000354 00000012 W RingBuffer<TimedEventHandler*, (unsigned char)2>::~RingBuffer()
# 00000366 00000012 W RingBuffer<InterruptEventHandler*, (unsigned char)0>::~RingBuffer()
# 00000330 00000012 W RingBuffer<void*, (unsigned char)25>::~RingBuffer()
# 00003472 00000096 T TimedEventHandler::RegisterForTimedEvent(unsigned long)
# 00005758 00000108 T digitalWrite
# 00005644 00000114 T pinMode
# 00005444 00000118 T init
# 08389060 00000142 b app
# 00005122 00000148 T __vector_16
# 00004316 00000166 T Ivm::DetachInterruptForPhysicalPin(unsigned char)
# 00003008 00000178 t _GLOBAL__sub_D__Z7TestPN2v
# 00003818 00000226 T Ivm::GetPortAndPortPinFromPhysicalPin(unsigned char, unsigned char*, unsigned char*)
# 00004044 00000272 T Ivm::AttachInterruptForPhysicalPin(unsigned char, PCIntEventHandler*)
# 00006230 00000286 T free
# 00005926 00000304 T malloc
# 00000800 00000588 W App::App()
# 00000800 00000588 W App::App()
#
#
# Pull out analysis of
# - 
#
#
#
# Notably, there are many duplicate lines (not sure why)
#
proc AnalyzeOutput { output } {
    set lineList [split $output "\n"]

    # eliminate duplicate lines
    array set ul_lineList [list]
    foreach { line } $lineList {
        set ul_lineList($line) ""
    }

    set lineList [lsort -dictionary [array names ul_lineList]]


    foreach { line } $lineList {
        set size [GetSize $line]
        set type [FlagType $line]
        set symbol [TrimSymbol [GetSymbol $line]]
        foreach { special class templateArgs member } [GetSymbolProperties $symbol] { }

        OnSymbolData $symbol $type $size $special $class $templateArgs $member
    }
}




set DATA [list]


proc OnSymbolData { symbol type size special class templateArgs member } {
    global DATA

#    puts -nonewline "$type -- $size"
#    if { $special != "" } { puts " ($special)" } else { puts "" }
#    puts "$symbol"
#    puts "    $class $templateArgs $member"
#    puts ""


    lappend DATA $symbol $type $size $special $class $templateArgs $member
}



proc GetClassList { } {
    global DATA

    array set ul_dataList [list]

    foreach { symbol type size special class templateArgs member } $DATA {
        if { ($templateArgs != "" || $member != "" ) &&
             $class != "atexit" && 
             $class != "yield" &&
             $class != "initVariant" } {
            set idxUnderscore [string first "_" $class]

            if { $idxUnderscore == -1 || $idxUnderscore != 0 } {
                set ul_dataList($class) ""
            }
        }
    }

    set dataList [lsort -command CmpClassBySize [array names ul_dataList]]

    return $dataList
}

proc CmpClassBySize { c1 c2 } {
    set retVal 0

    set c1Size [GetCumulativeSizeByClass $c1]
    set c2Size [GetCumulativeSizeByClass $c2]

    if { $c1Size < $c2Size } {
        set retVal -1
    } elseif { $c1Size > $c2Size } {
        set retVal 1
    } else {
        set retVal 0
    }

    return $retVal
}


proc GetTemplateArgsListByClass { classInput } {
    global DATA

    array set ul_dataList [list]

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $class == $classInput && $templateArgs != "" } {
            set ul_dataList($templateArgs) ""
        }
    }

    set dataList [lsort -dictionary [array names ul_dataList]]

    return $dataList
}


proc GetMemberListByClassAndTemplateArgs { classInput templateArgsInput } {
    global DATA

    set memberList [list]

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $class        == $classInput &&
             $templateArgs == $templateArgsInput &&
             $member       != ""} {
            set ul_dataList($member) ""
        }
    }

    set memberList [lsort -dictionary [array names ul_dataList]]

    return $memberList
}


proc GetSizeByClassTemplateArgsMember { classInput templateArgsInput memberInput } {
    global DATA

    set retVal 0

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $class        == $classInput &&
             $templateArgs == $templateArgsInput &&
             $member       == $memberInput } {
            incr retVal $size
        }
    }

    return $retVal
}


proc GetCumulativeSizeByClass { classInput } {
    global DATA

    set sizeTotal 0

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $class == $classInput } {
            incr sizeTotal $size
        }
    }

    return $sizeTotal
}



proc GetVarList { } {
    global DATA

    array set ul_dataList [list]

puts GetVarList
    foreach { symbol type size special class templateArgs member } $DATA {
        if { ($type == "DATA" || $type == "BSS") && $special == "" } {
            set ul_dataList($symbol) ""
        }
    }

    set dataList [lsort -command CmpVarBySize [array names ul_dataList]]

    return $dataList
} 

proc CmpVarBySize { v1 v2 } {
    set retVal 0

    set v1Size [GetCumulativeSizeBySymbol $v1]
    set v2Size [GetCumulativeSizeBySymbol $v2]

    if { $v1Size < $v2Size } {
        set retVal -1
    } elseif { $v1Size > $v2Size } {
        set retVal 1
    } else {
        set retVal 0
    }

    return $retVal
}

proc GetCumulativeSizeBySymbol { symbolInput } {
    global DATA

    set sizeTotal 0

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $symbol == $symbolInput } {
            incr sizeTotal $size
        }
    }

    return $sizeTotal
}

proc GetSpecialList { } {
    global DATA

    array set ul_dataList [list]

    foreach { symbol type size special class templateArgs member } $DATA {
        if { ($type == "DATA" || $type == "BSS") && $special != "" } {
            set idxUnderscore [string first "_" $class]

            if { $idxUnderscore == -1 || $idxUnderscore != 0 } {
                set ul_dataList($special) ""
            }
        }
    }

    set dataList [lsort -command CmpSpecialBySize [array names ul_dataList]]

    return $dataList
} 

proc CmpSpecialBySize { s1 s2 } {
    set retVal 0

    set s1Size [GetCumulativeSizeBySpecial $s1]
    set s2Size [GetCumulativeSizeBySpecial $s2]

    if { $s1Size < $s2Size } {
        set retVal -1
    } elseif { $s1Size > $s2Size } {
        set retVal 1
    } else {
        set retVal 0
    }

    return $retVal
}

proc GetSpecialArgsListBySpecial { specialInput } {
    global DATA

    array set ul_dataList [list]

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $special == $specialInput } {

            set idxFor [string first " for " $symbol]

            if { $idxFor != -1 } {
                set specialArgs [string range $symbol [expr $idxFor + 5] end]

                set ul_dataList($specialArgs) ""
            }

        }
    }

    set dataList [lsort -dictionary [array names ul_dataList]]

    return $dataList
}


proc GetSizeBySpecialSpecialArgs { specialInput specialArgsInput } {
    global DATA

    set retVal 0

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $special == $specialInput } {
            set idxFor [string first " for " $symbol]

            if { $idxFor != -1 } {
                set specialArgs [string range $symbol [expr $idxFor + 5] end]

                if { $specialArgs == $specialArgsInput } {
                    incr retVal $size
                }
            }
        }
    }

    return $retVal
}


proc GetCumulativeSizeBySpecial { specialInput } {
    global DATA

    set sizeSpecial 0

    foreach { symbol type size special class templateArgs member } $DATA {
        if { $special == $specialInput } {
            incr sizeSpecial $size
        }
    }

    return $sizeSpecial
}



proc GetCumulativeSizeText { } {
    set size 0

    set classList [GetClassList]

    foreach { class } [GetClassList] {
        incr size [GetCumulativeSizeByClass $class]
    }

    return $size
}

proc GetCumulativeSizeDataVariables { } {
    set size 0

    set varList [GetVarList]

    foreach { var } [GetVarList] {
        incr size [GetCumulativeSizeBySymbol $var]
    }

    return $size
}

proc GetCumulativeSizeDataSpecial { } {
    set size 0

    foreach { special } [GetSpecialList] {
        incr size [GetCumulativeSizeBySpecial $special]
    }

    return $size
}





proc GetSize { line } {
    return [string trimleft [lindex $line 1] 0]
}

proc GetSymbolProperties { symbol } {
    set special      ""
    set class        ""
    set templateArgs ""
    set member       ""

    # check for "vtable for" and "guard variable for"
    set idxFor [string first " for " $symbol]

    if { $idxFor != -1 } {
        set special [lindex $symbol 0]

        set symbol [string range $symbol [expr $idxFor + 5] end]
    }


    set idxTemplateStart [string first "<"  $symbol]
    set idxTemplateEnd   [string last  ">"  $symbol]
    set idxScope         [string last  "::" $symbol]


    if { $idxTemplateStart != -1 } {
        set class        [string range $symbol 0 [expr $idxTemplateStart - 1]]
        set templateArgs [string range $symbol $idxTemplateStart $idxTemplateEnd]

        if { $idxScope != -1 } {
            set member [string range $symbol [expr $idxScope + 2] end]
        }
    } else {
        if { $idxScope != -1 } {
            set class  [string range $symbol 0 [expr $idxScope - 1]]
            set member [string range $symbol [expr $idxScope + 2] end]
        } else {
            set class $symbol
        }
    }

    return [list $special $class $templateArgs $member]
}


proc TrimSymbol { symbol } {
    set trimSymbol ""

    set mode "KEEP"

    set cLast ""

    foreach { c } [split $symbol {}] {
        if { $mode == "KEEP" } {
            if { $c == "(" } {
                if { $cLast == "<" || $cLast == " " } {
                    set mode "DISCARD"
                } else {
                    append trimSymbol $c

                    set mode "DISCARD_BUT_KEEP_RIGHT_PAREN"
                }
            } else {
                append trimSymbol $c

                set cLast $c
            }
        } else {
            if { $c == ")" } {
                if { $mode == "DISCARD_BUT_KEEP_RIGHT_PAREN" } {
                    append trimSymbol $c
                }
                set mode "KEEP"
            }
        }
    }

    set idxClone [string first "\[clone" $trimSymbol]
    if { $idxClone != -1 } {
        set trimSymbol [string range $trimSymbol 0 [expr $idxClone - 1]]
    }

    return $trimSymbol
}

proc GetSymbol { line } {
    return [string range $line 20 end]
}


#
# We care about memory, like SRAM.  So whatever goes in there:
# - data
#
#
proc FlagType { line } {
    set retVal "UNKNOWN"

    set flag [lindex $line 2]

    switch -exact -- $flag {
        B -
        b -
        S -
        s {
            set retVal "BSS"
        }

        D -
        d -
        G -
        g -
        R -
        r -
        V -
        v {
            set retVal "DATA"
        }

        T -
        t {
            set retVal "TEXT"
        }

        W -
        w {
            set retVal "WEAK"
            #set retVal "TEXT"
        }


        default {
            set retVal "UNKNOWN"
        }
    }

    return $retVal
}



proc Report { } {
    global VERBOSE

    set sizeTextTotal    [GetCumulativeSizeText]
    set sizeVarTotal     [GetCumulativeSizeDataVariables]
    set sizeSpecialTotal [GetCumulativeSizeDataSpecial]
    set sizeSramTotal    [expr $sizeVarTotal + $sizeSpecialTotal]


    puts ""


    puts "---------------------------------"
    puts [format "\[     TEXT:    %5i Bytes      \]" $sizeTextTotal]
    puts "---------------------------------"


    set formatStrHdr    "\[Total: %5i Bytes, %2i%% of TEXT\]"
    set formatStrHdrLen [string length $formatStrHdr]
    set formatStr       "$formatStrHdr   %s"

    set classList [GetClassList]
    foreach { class } $classList {
        set sizeText [GetCumulativeSizeByClass $class]

        set sizePctTotal [expr int(double($sizeText) / double($sizeTextTotal) * 100.0)]

        puts [format $formatStr $sizeText $sizePctTotal $class]

        if { $VERBOSE } {
            foreach { templateArgs } [GetTemplateArgsListByClass $class] {
                foreach { member } [GetMemberListByClassAndTemplateArgs $class $templateArgs] {
                    set sizeText     [GetSizeByClassTemplateArgsMember $class $templateArgs $member]
                    set sizePctTotal [expr int(double($sizeText) / double($sizeTextTotal) * 100.0)]

                    puts [format $formatStr $sizeText $sizePctTotal "    ${templateArgs}::${member}"]
                }
            }
        }
    }
    puts ""
    puts ""



    set formatStrHdr    "\[Total: %5i Bytes, %2i%% of Vars, %2i%% of DATA/BSS\]"
    set formatStrHdrLen [string length $formatStrHdr]
    set formatStr       "$formatStrHdr   %s"


    puts "--------------------------------------------------"
    puts [format "\[       DATA - Variables: %5i Bytes            \]" $sizeVarTotal]
    puts "--------------------------------------------------"

    set varList [GetVarList]
    foreach { var } $varList {
        set sizeVar [GetCumulativeSizeBySymbol $var]

        set sizePctTotalVar  [expr int(double($sizeVar) / double($sizeVarTotal)  * 100.0)]
        set sizePctTotalSram [expr int(double($sizeVar) / double($sizeSramTotal) * 100.0)]

        puts [format $formatStr $sizeVar $sizePctTotalVar $sizePctTotalSram $var]
    }
    puts ""
    puts ""


    puts "--------------------------------------------------"
    puts [format "\[       DATA - Special: %5i Bytes              \]" $sizeSpecialTotal]
    puts "--------------------------------------------------"
    set specialList [GetSpecialList]

    foreach { special } $specialList {
        set sizeSpecial [GetCumulativeSizeBySpecial $special]

        set sizePctTotalSpecial [expr int(double($sizeSpecial) / double($sizeSpecialTotal) * 100.0)]
        set sizePctTotalSram    [expr int(double($sizeSpecial) / double($sizeSramTotal)    * 100.0)]

        puts [format $formatStr $sizeSpecial $sizePctTotalSpecial $sizePctTotalSram $special]

        if { $VERBOSE } {
            foreach { specialArgs } [GetSpecialArgsListBySpecial $special] {
                set sizeSpecial [GetSizeBySpecialSpecialArgs $special $specialArgs]

                set sizePctTotalSpecial [expr int(double($sizeSpecial) / double($sizeSpecialTotal) * 100.0)]
                set sizePctTotalSram    [expr int(double($sizeSpecial) / double($sizeSramTotal)    * 100.0)]

                puts [format $formatStr \
                             $sizeSpecial \
                             $sizePctTotalSpecial \
                             $sizePctTotalSram \
                             "    $special $specialArgs"]
            }
        }
    }
    puts ""
    puts ""




    puts "--------------------------------------------"
    puts "\[       SUMMARY                            \]"
    puts "--------------------------------------------"
    set sizeVarPctTotalSram     [expr int(double($sizeVarTotal)     / double($sizeSramTotal) * 100.0)]
    set sizeSpecialPctTotalSram [expr int(double($sizeSpecialTotal) / double($sizeSramTotal) * 100.0)]

    puts "PROGMEM : [format "%5i Bytes" $sizeTextTotal]"
    puts "DATA/BSS: [format "%5i Bytes" $sizeSramTotal]"
    puts " Vars   :[format " %5i Bytes ( %2i%% )"    $sizeVarTotal     $sizeVarPctTotalSram]"
    puts " Special:[format " %5i Bytes ( %2i%% )" $sizeSpecialTotal $sizeSpecialPctTotalSram]"
    puts ""
}


proc Analyze { } {
    set output [GetAvrNmOutput]

    AnalyzeOutput $output
}


set VERBOSE 0

proc Main { } {
    global argc
    global argv
    global argv0

    global VERBOSE

    if { $argc == 1 && [lindex $argv 0] == "-v" } {
        set VERBOSE 1
    }

    Analyze
    Report
}


Main










