#include <Evm.h>
#include <MyRadioController.h>



class MyRadioReceiver : public MyRadioController
{
public:


private:

};


void loop()
{
    Evm &evm = Evm::GetInstance();



    evm.MainLoop();
}









void setup()
{
    // nothing to do
}












