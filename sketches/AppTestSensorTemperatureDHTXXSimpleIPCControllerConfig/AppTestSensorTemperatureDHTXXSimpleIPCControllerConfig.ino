#include "AppTestSensorTemperatureDHTXXSimpleIPCController.h"


static AppTestSensorTemperatureDHTXXSimpleIPCControllerConfig cfg = {
    .realm   = 1,
    .srcAddr = 9,
    .dstAddr = 0,

    .pinRFRX = 14,
    .pinRFTX = 15,
    
    .pinSensor = 11
};


static AppTestSensorTemperatureDHTXXSimpleIPCController app(cfg);


void setup() 
{
    app.Run();
}

void loop() { }


