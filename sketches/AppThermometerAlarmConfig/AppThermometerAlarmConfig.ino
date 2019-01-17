#include "AppThermometerAlarm.h"


static AppThermometerAlarmConfig cfg =
{
  .pinThermometer = 14,
  .alarmTempF = 205,
  .pinAlarm = 15,
};

static AppThermometerAlarm app(cfg);


void setup()
{
  app.Run();
}

void loop() {}



