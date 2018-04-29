// I2C interface by default
//
#include "Wire.h"
#include "SparkFunIMU.h"
#include "SparkFunLSM303C.h"
#include "LSM303CTypes.h"

// #define DEBUG 1 in SparkFunLSM303C.h turns on debugging statements.
// Redefine to 0 to turn them off.

LSM303C myIMU;

void setup()
{
  @fix@Serial.begin(115200);
  if (myIMU.begin() != IMU_SUCCESS)
  {
    @fix@Serial.println("Failed setup.");
    while(1);
  }
}

void loop()
{
  //Get all parameters
  @fix@Serial.print("\nAccelerometer:\n");
  @fix@Serial.print(" X = ");
  @fix@Serial.println(myIMU.readAccelX(), 4);
  @fix@Serial.print(" Y = ");
  @fix@Serial.println(myIMU.readAccelY(), 4);
  @fix@Serial.print(" Z = ");
  @fix@Serial.println(myIMU.readAccelZ(), 4);

  // Not supported by hardware, so will return NAN
  @fix@Serial.print("\nGyroscope:\n");
  @fix@Serial.print(" X = ");
  @fix@Serial.println(myIMU.readGyroX(), 4);
  @fix@Serial.print(" Y = ");
  @fix@Serial.println(myIMU.readGyroY(), 4);
  @fix@Serial.print(" Z = ");
  @fix@Serial.println(myIMU.readGyroZ(), 4);

  @fix@Serial.print("\nMagnetometer:\n");
  @fix@Serial.print(" X = ");
  @fix@Serial.println(myIMU.readMagX(), 4);
  @fix@Serial.print(" Y = ");
  @fix@Serial.println(myIMU.readMagY(), 4);
  @fix@Serial.print(" Z = ");
  @fix@Serial.println(myIMU.readMagZ(), 4);

  @fix@Serial.print("\nThermometer:\n");
  @fix@Serial.print(" Degrees C = ");
  @fix@Serial.println(myIMU.readTempC(), 4);
  @fix@Serial.print(" Degrees F = ");
  @fix@Serial.println(myIMU.readTempF(), 4);
  
  delay(1000);
}
