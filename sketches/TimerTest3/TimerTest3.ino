#include <Evm.h>


// Does an Idle Timer which is Start()'d while already running get re-scheduled properly?

class IdleRestartTest : public IdleCallback
{
public:
    IdleRestartTest(uint8_t pin)
    : pin_(pin)
    {
        pinMode(pin_, OUTPUT);
        
        digitalWrite(pin_, HIGH);
        digitalWrite(pin_, LOW);
    }
    
private:
    void OnCallback()
    {
        digitalWrite(pin_, HIGH);
        digitalWrite(pin_, LOW);
    }

    uint8_t pin_;
};


class TestCoordinator : private TimedCallback
{
public:
    TestCoordinator(uint8_t pin, IdleRestartTest *irt, uint8_t interval)
    : pin_(pin)
    , irt_(irt)
    {
        pinMode(pin_, OUTPUT);
                
        digitalWrite(pin_, HIGH);
        digitalWrite(pin_, LOW);
        
        ScheduleInterval(interval);
    }

private:
    void OnCallback()
    {
        digitalWrite(pin_, HIGH);
        irt_->Start();
        digitalWrite(pin_, LOW);
    }

    uint8_t pin_;
    IdleRestartTest *irt_;
};



void setup()
{
    
}


// It works
void loop()
{
    Evm &evm = Evm::GetInstance();

    const uint8_t PIN_IRT = 0;
    const uint8_t PIN_TC = 1;
    
    IdleRestartTest irt(PIN_IRT);
    TestCoordinator tc(PIN_TC, &irt, 50);

    evm.MainLoop();
}













