//#include <SoftwareSerial.h>
#include <Evm.h>


const uint8_t PIN_LED = 0;
const uint8_t PIN_SERIAL_TX = 3;
const uint8_t PIN_SERIAL_RX = -1;


// Set up some global controls

Evm<4> evm;
//class EvmFake { public: void SetTimeout(uint32_t, void (*)(void *), void *) { } void MainLoop() { } } evm;

//SoftwareSerial SS(PIN_SERIAL_RX, PIN_SERIAL_TX);
class SSFake { public: void begin(uint8_t) { } void write(uint8_t) { } } SS;


void setup()
{
  // Set up LED pin
  pinMode(PIN_LED, OUTPUT);

  // Set up serial
  SS.begin(9600);

  // Flash to indicate startup
  digitalWrite(PIN_LED, HIGH);
  delay(250);
  digitalWrite(PIN_LED, LOW);
  delay(250);
  digitalWrite(PIN_LED, HIGH);
  delay(250);
  digitalWrite(PIN_LED, LOW);
}







void ToggleLED()
{
  digitalWrite(PIN_LED, HIGH);
  delay(10);
  digitalWrite(PIN_LED, LOW);
}


uint32_t timeoutList[] = { 0, 1, 5, 10, 15, 30, 50, 100, 1000 };
uint8_t  timeoutListLen = sizeof(timeoutList) / sizeof(uint32_t);
uint8_t  timeoutListIdx = 0;
uint8_t  DEFAULT_REMAINING_EVENTS = 4;

uint8_t remainingEvents;

void Timeout(void *userData)
{
  if (remainingEvents)
  {
    // SS.write(9);
    
    ToggleLED();

    --remainingEvents;

    evm.SetTimeout(timeoutList[timeoutListIdx], Timeout, NULL);
  }
  else
  {
    ++timeoutListIdx;
    SetUpNextTest();
  }
}

void SetUpNextTest()
{
  remainingEvents = DEFAULT_REMAINING_EVENTS;

  if (timeoutListIdx < timeoutListLen)
  {
    delay(500);
    
    SS.write(timeoutListIdx);

    delay(20);
    
    evm.SetTimeout(timeoutList[timeoutListIdx], Timeout, NULL);
  }
  else
  {
    SS.write(9);
  }
}

void SetUpTimerTests()
{
    SetUpNextTest();
}

void IdleFunction(void *userData)
{
  ToggleLED();
}

void SetUpIdleFunctionTests()
{
  evm.SetIdleFn(IdleFunction, NULL);
}

void loop()
{
  SetUpTimerTests();
  SetUpIdleFunctionTests();

  evm.MainLoop();
}




















/*

void RawTimerTest()
{
  uint32_t timeLastUs = micros();
  uint32_t timeNowUs   = micros();

  uint8_t count = 0;

  uint32_t timeoutUs = (uint32_t)(((uint64_t)TIMEOUT_US * 1024) / 1000);

  while (1)
  {
    timeNowUs = micros();

    if ((timeNowUs - timeLastUs) >= timeoutUs)
    {
       if (count % 2)
       {
         digitalWrite(PIN_LED, HIGH);
       }
       else
       {
         digitalWrite(PIN_LED, LOW);
       }
  
       ++count;

       timeLastUs = micros();
    }
  }
}


*/









