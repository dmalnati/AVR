
#include <TestNoDynamicMemory.h>

 
template <typename EvmT>
struct Sys
{
    Sys(EvmT &evm) : evm(evm) { }

    EvmT &evm;
};
 
 
class App
{
public:
    App()
    : evm_()
    , sys_(evm_)
    , uoe_(evm_)
    {
        // Nothing to do
    }
 
    void Run()
    {
        
        IdleTimeEventHandler<SysT> iteh(sys_);
        
        evm_.MainLoop();
    }
 
private:
    static const uint8_t FADER_COUNT = 10;
 
    static const uint8_t TEST = Fader<FADER_COUNT>::COUNT_IDLE_TIME_EVENT_HANDLER * 2;
 
 
    typedef
    Evm<
        //Fader<FADER_COUNT>::COUNT_IDLE_TIME_EVENT_HANDLER * 2,
        TEST,
        Fader<FADER_COUNT>::COUNT_TIMED_EVENT_HANDLER * 2,
        Fader<FADER_COUNT>::COUNT_INTERRUPT_EVENT_HANDLER * 2
    > EvmT;
 
    typedef Sys<EvmT> SysT;
 
 
    Fader<FADER_COUNT> fader1_;
    Fader<FADER_COUNT> fader2_;
 
 
    EvmT evm_;
    SysT sys_;
    UserOfEvm <EvmT>uoe_;
};
 
 
 
//////////////////////////////

App app;

void loop()
{
    app.Run();
}










void setup()
{
}


























