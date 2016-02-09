#include <Evm.h>


// Some code to test refactoring Evm libs.


void loop()
{
    Evm &evm = Evm::GetInstance();


    evm.MainLoop();
}

void setup()
{
    // nothing to do
}
