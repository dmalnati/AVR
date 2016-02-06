#include <Evm.h>

void setup() 
{
  // Set up LED pin
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  // Flash to indicate startup
  digitalWrite(0, HIGH);
  delay(250);
  digitalWrite(0, LOW);
  delay(250);
  digitalWrite(0, HIGH);
  delay(250);
  digitalWrite(0, LOW);
}



void TogglePIN0(void *)
{
  static uint8_t pin = 0;

  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void TogglePIN2(void *)
{
  static uint8_t pin = 2;

  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void TogglePIN3(void *)
{
  static uint8_t pin = 3;

  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}


void TogglePINX(void *userData)
{
  uint8_t pin = (uint16_t)userData;

  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}



class MyPinToggler : public IdleCallback
{
public:
  MyPinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }

private:
  void OnCallback()
  {
    digitalWrite(pin_, HIGH);
    digitalWrite(pin_, LOW);
  }

  uint8_t pin_;
};

class MyTimedPinToggler : public TimedCallback
{
public:
  MyTimedPinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }

private:
  void OnCallback()
  {
    digitalWrite(pin_, HIGH);
    digitalWrite(pin_, LOW);
  }

  uint8_t pin_;
};


void loop()
{
  Evm &evm = Evm::GetInstance();

  
  // Set up an idle callback the hard way
  MyPinToggler pt0(0); pt0.Start();
  //MyPinToggler pt3(3); pt3.Start();
  //IdleCallbackFnWrapper ic3(TogglePIN3, NULL); ic3.Start();

  // Set up timed callbacks
  //MyTimedPinToggler tpt2(2); tpt2.Schedule(50);;
  //TimedCallbackFnWrapper tcf2(TogglePIN2, NULL); tcf2.ScheduleInterval(75);

  //TimedCallbackFnWrapper tcf0(TogglePINX, (void *)0); tcf0.ScheduleInterval(1000);
  //TimedCallbackFnWrapper tcf21(TogglePINX, (void *)2); tcf21.ScheduleInterval(150);
  //TimedCallbackFnWrapper tcf3(TogglePINX, (void *)3); tcf3.ScheduleInterval(100);
  //TimedCallbackFnWrapper tcf2(TogglePINX, (void *)2); tcf2.ScheduleInterval(0);
  
  evm.MainLoop();
}

