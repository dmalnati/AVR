#include "PAL.h"
#include "Timer1.h"
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
    @fix@Serial.begin(9600);

    PAL.PinMode(pinSignalA, OUTPUT);
    PAL.PinMode(pinSignalB, OUTPUT);

    PAL.PowerDownTimer1();
    PAL.PowerUpTimer1();
    t.StopTimer();
    //t.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
    t.StopTimer();
    t.StartTimer();

    a = t.GetTimerChannelA();
    b = t.GetTimerChannelB();

    //TestPHASE_CORRECT_PWM_8_BIT_PinDriver();
    //TestPHASE_CORRECT_PWM_8_BIT_PinDriver_and_InterruptDriver();
    //TestPHASE_CORRECT_PWM_8_BIT_InterruptDriver();
    //TestPHASE_CORRECT_PWM_8_BIT_Half_and_Half();
    //TestFAST_PWM_8_BIT();
    TestFAST_PWM_TOP_OCRNA();

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

void TestFAST_PWM_8_BIT()
{
    t.SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);

    a->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SET);
    a->SetValue(150);
    b->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SET);
    b->SetValue(10);

    tedA.SetCallback([&](){
        PAL.DigitalToggle(pinSignalA);
        a->SetValue(a->GetValue() + 1);
    });
    tedA.RegisterForTimedEventInterval(1);
    

    b->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinSignalB);
        b->SetValue(b->GetValue() + 1);
    });
    b->RegisterForInterrupt();
}

void TestFAST_PWM_TOP_OCRNA()
{
    t.SetTimerMode(Timer1::TimerMode::FAST_PWM_TOP_OCRNA);

    a->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE);
    a->SetValue(150);
    b->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SET);
    b->SetValue(100);

    tedA.SetCallback([&](){
        PAL.DigitalToggle(pinSignalA);
        a->SetValue(a->GetValue() + 1);
    });
    tedA.RegisterForTimedEventInterval(1);
}


static uint32_t LOOP_COUNT = 0;
 
void OnTimeout()
{
    ++LOOP_COUNT;
    
    @fix@Serial.println("--------------");
    @fix@Serial.print("Loop: ");
    @fix@Serial.println(LOOP_COUNT);
    @fix@Serial.print("PRR: 0x");
    @fix@Serial.println(PRR, HEX);
    @fix@Serial.print("CPU Prescaler: ");
    @fix@Serial.println(PAL.GetCpuPrescalerValue());
    @fix@Serial.print("CPU Frequency: ");
    @fix@Serial.println(PAL.GetCpuFreq());
    @fix@Serial.print("TimerPrescalerValue: ");
    @fix@Serial.println(t.GetTimerPrescalerValue());
    @fix@Serial.print("Timer ticks/sec: ");
    @fix@Serial.println((double)PAL.GetCpuFreq() / (double)t.GetTimerPrescalerValue());
    @fix@Serial.print("Timer ticks/ms: ");
    @fix@Serial.println((double)PAL.GetCpuFreq() / (double)t.GetTimerPrescalerValue() / 1000.0);
    @fix@Serial.print("Timer ticks/us: ");
    @fix@Serial.println((double)PAL.GetCpuFreq() / (double)t.GetTimerPrescalerValue() / 1000000.0);

    
    @fix@Serial.print("Timer:");
    for (uint8_t i = 0; i < 16; ++i)
    {
        @fix@Serial.print(" ");
        @fix@Serial.print(t.GetTimerValue());
    }
    @fix@Serial.println();

    // capture as fast as possible
    static const uint16_t SAMPLE_COUNT = 16;
    uint8_t valList[SAMPLE_COUNT] = { 0 };
    for (uint16_t i = 0; i < SAMPLE_COUNT; ++i)
    {
        valList[i] = (uint8_t)TCNT1;

        //i -= (i && valList[i] == valList[i - 1]) ? 1 : 0;
    }
    @fix@Serial.print("Timer Capture:");
    for (uint16_t i = 0; i < SAMPLE_COUNT; ++i)
    {
        @fix@Serial.print(" ");
        @fix@Serial.print(valList[i]);
    }
    @fix@Serial.println();
}


void loop() {}









