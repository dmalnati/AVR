#include <stdint.h>

#include <Sys.h>


struct AppTemplateCfg
{
    uint8_t val;
};



class AppTemplate
{
public:
    App(AppTemplateCfg &cfg)
    : cfg_(cfg)
    , evm_()
    , sys_(evm_)
    {
        // Nothing to do
    }
 
    void Run()
    {
        evm_.MainLoop();
    }


    // Accounting work
    static const uint8_t COUNT_IDLE_TIME_EVENT_HANDLER = 0;
    static const uint8_t COUNT_TIMED_EVENT_HANDLER     = 0;
    static const uint8_t COUNT_INTERRUPT_EVENT_HANDLER = 0;
    
private:






    // Config object
    AppTemplateCfg &cfg_;


    // Create statically-sized Evm
    typedef
    Evm<
        COUNT_IDLE_TIME_EVENT_HANDLER,
        COUNT_TIMED_EVENT_HANDLER,
        COUNT_INTERRUPT_EVENT_HANDLER
    > EvmT;

    EvmT evm_;

    // Create Sys container to pass around
    typedef
    Sys<EvmT> SysT;
    
    SysT sys_;
};


