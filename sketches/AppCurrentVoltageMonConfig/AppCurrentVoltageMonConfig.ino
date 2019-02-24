#include "AppCurrentVoltageMon.h"


static AppCurrentVoltageMonConfig cfg =
{
    .baud = 9600,
};

static AppCurrentVoltageMon app(cfg);


void setup()
{
  app.Run();
}

void loop() {}



