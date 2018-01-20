#include "AX25UIMessageTransmitter.h"


static AX25UIMessageTransmitter<> amt;


void setup()
{
    amt.Init([](){},
             [](){});
}

void loop() {}
