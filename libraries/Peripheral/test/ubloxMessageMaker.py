#!/usr/bin/python



# http://stackoverflow.com/questions/5574702/how-to-print-to-stderr-in-python
from __future__ import print_function
import sys


import sys
import os

from struct import *




def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)



# https://docs.python.org/2/library/struct.html
# https://www.u-blox.com/sites/default/files/products/documents/u-blox6_ReceiverDescrProtSpec_%28GPS.G6-SW-10018%29_Public.pdf?utm_source=en%2Fimages%2Fdownloads%2FProduct_Docs%2Fu-blox6_ReceiverDescriptionProtocolSpec_%28GPS.G6-SW-10018%29.pdf



'''
Types to care about:
(multi-byte values are Little Endian)

Code  Type                          Size (Bytes)
----------------------------------------------
U1    Unsigned Char                 1
I1    Signed Char                   1
X1    Bitfield                      1
U2    Unsigned Short                2
I2    Signed Short                  2
X2    Bitfield                      2
U4    Unsigned Long                 4
I4    Signed Long                   4
X4    Bitfield                      4
R4    IEEE 754 Single Precision     4
R8    IEEE 754 Double Precision     8
CH    ASCII / ISO 8859.1 Encoding   1

'''

class UbxMessageMaker():
    def __init__(self):
        self.debugOnly = False

        self.ubxCode__conversionData = {
            "U1": ( self.IntCast, "B",  1 ),
            "I1": ( self.IntCast, "b",  1 ),
            "X1": ( self.IntCast, "B",  1 ),
            "U2": ( self.IntCast, "<H", 2 ),
            "I2": ( self.IntCast, "<h", 2 ),
            "X2": ( self.IntCast, "<H", 2 ),
            "U4": ( self.IntCast, "<I", 4 ),
            "I4": ( self.IntCast, "<i", 4 ),
            "X4": ( self.IntCast, "<I", 4 ),
            "R4": ( float,        "f",  4 ),
            "R8": ( float,        "d",  8 ),
            "CH": ( str,          "c",  1 )
        }

    def DebugOnly(self):
        self.debugOnly = True


    def IntCast(self, strVal):
        return int(strVal, 0)


    def PrintByteList(self, byteList):
        if self.debugOnly:
            self.PrintDebug(''.join('{:02X} '.format(b) for b in byteList))
        else:
            sys.stdout.write(byteList)
            sys.stdout.flush()

    def PrintDebug(self, strMsg):
        if self.debugOnly:
            print(strMsg)

    def ByteToHexStr(self, byte):
        return '{:02X}'.format(byte)

    def ByteListToHexStr(self, byteList):
        hexStr = ""
        sep    = ""

        for byte in byteList:
            hexStr += sep
            hexStr += '{:02X}'.format(byte)

            sep = " "

        return hexStr

    def PrintMessage(self, colNameList, rowValList, byteList):
        idx = 0

        self.PrintDebug("{:15s}: ".format("Sync Char 1") +
                        self.ByteToHexStr(byteList[idx]))
        idx += 1

        self.PrintDebug("{:15s}: ".format("Sync Char 2") +
                         self.ByteToHexStr(byteList[idx]))
        idx += 1

        self.PrintDebug("{:15s}: ".format("CLASS") +
                         self.ByteToHexStr(byteList[idx]))
        idx += 1

        self.PrintDebug("{:15s}: ".format("ID") +
                        self.ByteToHexStr(byteList[idx]))
        idx += 1

        self.PrintDebug("{:15s}: ".format("LENGTH") +
                        self.ByteListToHexStr(byteList[idx:idx+2]))
        idx += 2

        ubxCodeList  = colNameList[2:]
        valAsStrList = rowValList[2:]

        for ubxCode, valAsStr in zip(ubxCodeList, valAsStrList):
            (fnCvtToIntermediateForm, bytePackFormat, size) = \
                self.ubxCode__conversionData[ubxCode]

            self.PrintDebug("{:15s}: ".format(ubxCode + "(" + valAsStr + ")") 
                            + self.ByteListToHexStr(byteList[idx:idx+size]))

            idx += size


        self.PrintDebug("{:15s}: ".format("Checksum A") +
                        self.ByteToHexStr(byteList[idx]))
        idx += 1

        self.PrintDebug("{:15s}: ".format("Checksum B") +
                        self.ByteToHexStr(byteList[idx]))


    def Checksum(self, byteList):
        ckA = 0
        ckB = 0

        # we should be doing 8-bit math here, but we'll just mod at the end
        # for the same effect
        for byte in byteList:
            ckA += byte
            ckB += ckA

        # go 8-bit
        ckA &= 0xFF
        ckB &= 0xFF

        return (ckA, ckB)
        

    def EncodeValue(self, ubxCode, valueAsStr):
        byteList = bytearray()

        (fnCvtToIntermediateForm, bytePackFormat, size) = \
            self.ubxCode__conversionData[ubxCode]

        intermediateForm = fnCvtToIntermediateForm(valueAsStr)

        byteList.extend(pack(bytePackFormat, intermediateForm))

        return byteList


    def EncodeValueList(self, ubxCodeList, valAsStrList):
        byteList = bytearray()

        for ubxCode, valAsStr in zip(ubxCodeList, valAsStrList):
            byteListTmp = self.EncodeValue(ubxCode, valAsStr)

            byteList.extend(byteListTmp)

        return byteList


    def ConvertLine(self, colNameList, rowValList):
        # Create byteList for complete message
        byteList = bytearray()

        # pack sync char 1
        byteList.extend(pack("B", 0xB5))

        # pack sync char 2
        byteList.extend(pack("B", 0x62))

        # pack class
        classVal = rowValList[0]
        byteList.extend(pack("B", int(classVal, 0)))

        # pack id
        idVal = rowValList[1]
        byteList.extend(pack("B", int(idVal, 0)))

        # Encode fields (everything after CLASS and ID columns)
        ubxCodeList  = colNameList[2:]
        valAsStrList = rowValList[2:]
        byteListPayload = self.EncodeValueList(ubxCodeList, valAsStrList)

        # pack payload length (little endian)
        byteList.extend(pack("<H", len(byteListPayload)))

        # pack payload
        byteList.extend(byteListPayload)

        # calculate checksum (CLASS, ID, LENGTH, PAYLOAD)
        byteListChecksumData = byteList[2:]
        ckA, ckB = self.Checksum(byteListChecksumData)

        # pack checksum
        byteList.extend(pack("B", ckA))
        byteList.extend(pack("B", ckB))

        return byteList

    def GetMsgLengthFromFields(self, byteList):
        NON_FIELD_DATA_LENGTH = 8
        return len(byteList) - NON_FIELD_DATA_LENGTH

    def ConvertFile(self, filename):
        fd = open(filename, "r")

        # process header -- split on comma and strip whitespace
        line = fd.readline().rstrip("\n")

        colNameList = [x.strip() for x in line.split(',')]

        self.PrintDebug("colNameList: " + str(colNameList))

        if len(colNameList) < 2 or \
           colNameList[0] != "CLASS" or \
           colNameList[1] != "ID":
            eprint("Header line must have a CLASS and ID column to start.")
        else:
            # process every next line
            line = fd.readline().rstrip("\n")
            while line != "":
                # split on comma and strip whitespace
                rowValList = [x.strip() for x in line.split(',')]

                self.PrintDebug("")
                self.PrintDebug("rowValList: " + str(rowValList))

                if len(colNameList) == len(rowValList):
                    byteList = self.ConvertLine(colNameList, rowValList)

                    self.PrintDebug(
                        str(self.GetMsgLengthFromFields(byteList)) +
                        " field bytes, " +
                        str(len(byteList)) + " bytes total"
                    )
                    self.PrintByteList(byteList)
                    self.PrintMessage(colNameList, rowValList, byteList)

                else:
                    eprint("line(" +
                           line    +
                           ") has the wrong number of elements.")

                line = fd.readline().rstrip("\n")

        fd.close()



def Main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: "   + sys.argv[0] + " [-debug] <input.csv>")
        sys.exit(-1)


    # Parse arguments
    argList    = []
    argListTmp = sys.argv[1:]

    debugOnly = False

    for arg in argListTmp:
        if arg == "-debug":
            debugOnly = True
        else:
            argList.append(arg)

    filename = argList[0]

    # Create UBX converter
    ubx = UbxMessageMaker()

    # Conditionally put in debug mode
    if debugOnly:
        ubx.DebugOnly()

    # Convert file
    ubx.ConvertFile(filename)



Main()

























