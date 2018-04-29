#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"



/*


Reproducing a message captured from APRSDroid

Message callback, 41 bytes, dump:
0x00000000: 82 A0 88 A4 62 68 E0 96 | 10000010 10100000 10001000 10100100 01100010 01101000 11100000 10010110 | 9..00000
0x00000008: 88 64 96 88 88 72 AE 92 | 10001000 01100100 10010110 10001000 10001000 01110010 10101110 10010010 | .......)
0x00000010: 88 8A 62 40 63 03 F0 3A | 10001000 10001010 01100010 01000000 01100011 00000011 11110000 00111010 | ...<.=.
0x00000018: 4B 44 32 4B 44 44 20 20 | 01001011 01000100 00110010 01001011 01000100 01000100 00100000 00100000 | .6.../..
0x00000020: 20 3A 68 69 32 7B 32 43 | 00100000 00111010 01101000 01101001 00110010 01111011 00110010 01000011 | ..).)...
0x00000028: F6                      | 11110110                                                                | .

SRC: [KD2KDD-9] DST: [APDR14-0] PATH: [WIDE1-1] DATA: :KD2KDD   :hi2{2





[dst addr]
10000010 - 82 -> 41 = A
10100000 - A0 -> 50 = P
10001000 - 88 -> 44 = D
10100100 - A4 -> 52 = R
01100010 - 62 -> 31 = 1
01101000 - 68 -> 32 = 4
11100000 - E0 // Has been repeated, RR, 0 SSID, 0 stop bit -- Why been repeated?

[src addr]
10010110 - 96 -> 4B = K
10001000 - 88 -> 44 = D
01100100 - 64 -> 32 = 2
10010110 - 96 -> 4B = K
10001000 - 88 -> 44 = D
10001000 - 88 -> 44 = D
01110010 - 72 // Not repeated, RR, 9 SSID, 0 stop bit

[path addr]
10101110 - AE -> 57 = W
10010010 - 92 -> 49 = I
10001000 - 88 -> 44 = D
10001010 - 8A -> 45 = E
01100010 - 62 -> 31 = 1
01000000 - 40 -> 20 = <space>
01100011 - 63 // Not repeated, RR, 1 SSID, 1 stop bit

[AX.25 stuff]
00000011 - 03 // Control Character
11110000 - F0 // Protocol ID

[16 bytes message payload]
00111010 - 3A - :
01001011 - 4B - K
01000100 - 44 - D
00110010 - 32 - 2
01001011 - 4B - K
01000100 - 44 - D
01000100 - 44 - D
00100000 - 20 - <space>
00100000 - 20 - <space>
00100000 - 20 - <space>
00111010 - 3A - :
01101000 - 68 - h
01101001 - 69 - i
00110010 - 32 - 2
01111011 - 7B - {
00110010 - 32 - 2

[FCS]
01000011 - 43
11110110 - F6


Results below match.

*/

static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;


void setup()
{
    @fix@Serial.begin(9600);

    while (1)
    {
        DoMessageTest();
        PAL.Delay(1000);
    }
}

void DoMessageTest()
{
    uint8_t *buf     = bufShared;
    uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf);

    msg.SetDstAddress("APDR14", 0);
    msg.SetSrcAddress("KD2KDD", 9);
    msg.AddRepeaterAddress("WIDE1", 1);

    @fix@Serial.println("Post SetAddress");
    StreamBlob(Serial, buf, bufSize, 1);

    const char *info = ":KD2KDD   :hi2{2";
    uint8_t infoLen = strlen(info);   // 16
    msg.AppendInfo((uint8_t *)info, infoLen);

    @fix@Serial.println("Post AppendInfo");
    StreamBlob(Serial, buf, bufSize, 1);

    // hack in the has-been-repeated bit on the dst address,
    buf[6] |= 0b10000000;
    
    uint8_t bytesUsed = msg.Finalize();
    @fix@Serial.print("Post Finalize (");
    @fix@Serial.print(bytesUsed);
    @fix@Serial.print(" bytes used)");
    @fix@Serial.println();

    @fix@Serial.println("Container buffer");
    StreamBlob(Serial, buf, bufSize, 1);

    @fix@Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);

    @fix@Serial.println();
    @fix@Serial.println();
}
 
void loop()
{

}












