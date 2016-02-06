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



class MyIdlePinToggler : public IdleCallback
{
public:
  MyIdlePinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }

protected:
  void OnCallback()
  {
    digitalWrite(pin_, HIGH);
    digitalWrite(pin_, LOW);
  }

  uint8_t pin_;
};

class MyIdlePinTogglerWhoCancelsAfterX : public MyIdlePinToggler
{
public:
  MyIdlePinTogglerWhoCancelsAfterX(uint8_t pin, uint8_t count)
  : MyIdlePinToggler(pin)
  , count_(count)
  {
    // nothing to do
  }

protected:
  virtual void OnCallback()
  {
    for (uint8_t i = 0; i < count_; ++i)
    {
      MyIdlePinToggler::OnCallback();
    }
    
    --count_;

    if (!count_)
    {
      Stop();
    }
  }

private:
  uint8_t count_;
};

class MyTimedPinToggler : public TimedCallback
{
public:
  MyTimedPinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }

protected:
  virtual void OnCallback()
  {
    digitalWrite(pin_, HIGH);
    digitalWrite(pin_, LOW);
  }

  uint8_t pin_;
};

class MyTimedPinTogglerWhoCancelsAfterX : public MyTimedPinToggler
{
public:
  MyTimedPinTogglerWhoCancelsAfterX(uint8_t pin, uint8_t count)
  : MyTimedPinToggler(pin)
  , count_(count)
  {
    // nothing to do
  }

protected:
  virtual void OnCallback()
  {
    for (uint8_t i = 0; i < count_; ++i)
    {
      MyTimedPinToggler::OnCallback();
    }
    
    --count_;

    if (!count_)
    {
      Cancel();
    }
  }

private:
  uint8_t count_;
};


void loop()
{
  Evm &evm = Evm::GetInstance();

  
  // Set up an idle callback the hard way
  //MyIdlePinToggler pt0(0); pt0.Start();
  MyIdlePinTogglerWhoCancelsAfterX ptx0(0, 15); ptx0.Start();
  //MyIdlePinToggler pt3(3); pt3.Start();
  //IdleCallbackFnWrapper ic3(TogglePIN3, NULL); ic3.Start();

  MyIdlePinTogglerWhoCancelsAfterX ptx2(2, 45); ptx2.Start();

  // Set up timed callbacks
  //MyTimedPinToggler tpt2(2); tpt2.Schedule(50);;
  //TimedCallbackFnWrapper tcf2(TogglePIN2, NULL); tcf2.ScheduleInterval(75);

  //TimedCallbackFnWrapper tcf0(TogglePINX, (void *)0); tcf0.ScheduleInterval(1000);
  //TimedCallbackFnWrapper tcf21(TogglePINX, (void *)2); tcf21.ScheduleInterval(150);
  //TimedCallbackFnWrapper tcf3(TogglePINX, (void *)3); tcf3.ScheduleInterval(100);
  MyTimedPinTogglerWhoCancelsAfterX tcx3(3, 7); tcx3.ScheduleInterval(20);
  //TimedCallbackFnWrapper tcf2(TogglePINX, (void *)2); tcf2.ScheduleInterval(0);
  
  evm.MainLoop();
}

