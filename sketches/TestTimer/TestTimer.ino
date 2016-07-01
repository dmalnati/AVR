#include "PAL.h"
#include "Timer.h"
#include "Evm.h"


static Evm::Instance<10,10,10> evm;

static Timer1 t;
static TimerChannel *a = NULL;
static TimerChannel *b = NULL;

static const uint8_t PIN_SIGNAL_A = 27;
static const uint8_t PIN_SIGNAL_B = 28;

Pin pinSignalA(PIN_SIGNAL_A);
Pin pinSignalB(PIN_SIGNAL_B);

TimedEventHandlerDelegate tedMain;
uint8_t stateA = 0;
TimedEventHandlerDelegate tedA;
uint8_t stateB = 0;
TimedEventHandlerDelegate tedB;

void setup()
{
    Serial.begin(9600);

    PAL.PinMode(pinSignalA, OUTPUT);
    PAL.PinMode(pinSignalB, OUTPUT);

    t.PowerDownTimer();
    t.PowerUpTimer();
    t.StopTimer();
    t.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);

    a = t.GetTimerChannelA();
    b = t.GetTimerChannelB();

    //TestPHASE_CORRECT_PWM_8_BIT_PinDriver();
    //TestPHASE_CORRECT_PWM_8_BIT_PinDriver_and_InterruptDriver();
    //TestPHASE_CORRECT_PWM_8_BIT_InterruptDriver();
    TestPHASE_CORRECT_PWM_8_BIT_Half_and_Half();

    tedMain.SetCallback([](){ OnTimeout(); });
    tedMain.RegisterForTimedEventInterval(3000);

    evm.MainLoop();
}

void TestNORMAL()
{
    t.SetTimerMode(Timer1::TimerMode::NORMAL);
}

void TestPHASE_CORRECT_PWM_8_BIT_PinDriver()
{
    t.SetTimerMode(Timer1::TimerMode::PHASE_CORRECT_PWM_8_BIT);

    a->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::CLEAR);
    a->SetValue(150);
    b->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::SET);
    b->SetValue(10);
}

void TestPHASE_CORRECT_PWM_8_BIT_PinDriver_and_InterruptDriver()
{
    t.SetTimerMode(Timer1::TimerMode::PHASE_CORRECT_PWM_8_BIT);

    a->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::CLEAR);
    a->SetValue(150);
    b->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::SET);
    b->SetValue(10);


    a->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalA);
    });
    a->RegisterForInterrupt();

    tedA.SetCallback([&](){
        if (stateA) a->DeRegisterForInterrupt();
        else a->RegisterForInterrupt();

        stateA = !stateA;
    });
    tedA.RegisterForTimedEventInterval(1500);
    

    b->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalB);
    });
    b->RegisterForInterrupt();

    tedB.SetCallback([&](){
        if (stateB) b->DeRegisterForInterrupt();
        else b->RegisterForInterrupt();

        stateB = !stateB;
    });
    tedB.RegisterForTimedEventInterval(330);
}

void TestPHASE_CORRECT_PWM_8_BIT_InterruptDriver()
{
    t.SetTimerMode(Timer1::TimerMode::PHASE_CORRECT_PWM_8_BIT);

    a->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::NONE);
    a->SetValue(150);
    b->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::NONE);
    b->SetValue(10);


    a->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalA);
    });
    a->RegisterForInterrupt();

    tedA.SetCallback([&](){
        if (stateA) a->DeRegisterForInterrupt();
        else a->RegisterForInterrupt();

        stateA = !stateA;
    });
    tedA.RegisterForTimedEventInterval(1500);
    

    b->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalB);
    });
    b->RegisterForInterrupt();

    tedB.SetCallback([&](){
        if (stateB) b->DeRegisterForInterrupt();
        else b->RegisterForInterrupt();

        stateB = !stateB;
    });
    tedB.RegisterForTimedEventInterval(330);
}

void TestPHASE_CORRECT_PWM_8_BIT_Half_and_Half()
{
    t.SetTimerMode(Timer1::TimerMode::PHASE_CORRECT_PWM_8_BIT);

    a->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::SET);
    a->SetValue(150);
    b->SetPhaseCorrectPWMModeBehavior(TimerChannel::PhaseCorrectPWMModeBehavior::NONE);
    b->SetValue(10);


    a->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalA);
    });
    a->RegisterForInterrupt();

    tedA.SetCallback([&](){
        if (stateA) a->DeRegisterForInterrupt();
        else a->RegisterForInterrupt();

        stateA = !stateA;
    });
    tedA.RegisterForTimedEventInterval(1500);
    

    b->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalB);
    });
    b->RegisterForInterrupt();

    tedB.SetCallback([&](){
        if (stateB) b->DeRegisterForInterrupt();
        else b->RegisterForInterrupt();

        stateB = !stateB;
    });
    tedB.RegisterForTimedEventInterval(330);
}


static uint32_t LOOP_COUNT = 0;
 
void OnTimeout()
{
    ++LOOP_COUNT;
    
    Serial.println("--------------");
    Serial.print("Loop: ");
    Serial.println(LOOP_COUNT);
    Serial.print("PRR: 0x");
    Serial.println(PRR, HEX);
    Serial.print("CPU Prescaler: ");
    Serial.println(PAL.GetCpuPrescalerValue());
    Serial.print("CPU Frequency: ");
    Serial.println(PAL.GetCpuFreq());
    Serial.print("TimerPrescalerValue: ");
    Serial.println(t.GetTimerPrescalerValue());
    Serial.print("Timer ticks/sec: ");
    Serial.println((double)PAL.GetCpuFreq() / (double)t.GetTimerPrescalerValue());
    Serial.print("Timer ticks/ms: ");
    Serial.println((double)PAL.GetCpuFreq() / (double)t.GetTimerPrescalerValue() / 1000.0);
    Serial.print("Timer ticks/us: ");
    Serial.println((double)PAL.GetCpuFreq() / (double)t.GetTimerPrescalerValue() / 1000000.0);
    Serial.print("TCCRA: 0x");
    Serial.println(t.GetTCCRA(), HEX);
    Serial.print("TCCRB: 0x");
    Serial.println(t.GetTCCRB(), HEX);
    Serial.print("OCRA: 0x");
    Serial.println(t.GetOCRA(), HEX);
    Serial.print("OCRB: 0x");
    Serial.println(t.GetOCRB(), HEX);
    Serial.print("TIMSK: 0x");
    Serial.println(TIMSK1, HEX);
    Serial.print("TIFR: 0x");
    Serial.println(TIFR1, HEX);

    
    Serial.print("WaveformGenerationMode: ");
    Serial.println(t.GetWaveformGenerationMode());
    Serial.print("Timer:");
    for (uint8_t i = 0; i < 16; ++i)
    {
        Serial.print(" ");
        Serial.print(t.GetTimerValue());
    }
    Serial.println();

    // capture as fast as possible
    static const uint16_t SAMPLE_COUNT = 16;
    uint8_t valList[SAMPLE_COUNT] = { 0 };
    for (uint16_t i = 0; i < SAMPLE_COUNT; ++i)
    {
        valList[i] = (uint8_t)TCNT1;

        //i -= (i && valList[i] == valList[i - 1]) ? 1 : 0;
    }
    Serial.print("Timer Capture:");
    for (uint16_t i = 0; i < SAMPLE_COUNT; ++i)
    {
        Serial.print(" ");
        Serial.print(valList[i]);
    }
    Serial.println();
}


void loop() {}









