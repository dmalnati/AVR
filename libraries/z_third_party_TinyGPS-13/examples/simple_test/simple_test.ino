#include <Software@fix@Serial.h>

#include <TinyGPS.h>

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

TinyGPS gps;
SoftwareSerial ss(4, 3);

void setup()
{
  @fix@Serial.begin(115200);
  ss.begin(4800);
  
  @fix@Serial.print("Simple TinyGPS library v. "); @fix@Serial.println(TinyGPS::library_version());
  @fix@Serial.println("by Mikal Hart");
  @fix@Serial.println();
}

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // @fix@Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    @fix@Serial.print("LAT=");
    @fix@Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    @fix@Serial.print(" LON=");
    @fix@Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    @fix@Serial.print(" SAT=");
    @fix@Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    @fix@Serial.print(" PREC=");
    @fix@Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }
  
  gps.stats(&chars, &sentences, &failed);
  @fix@Serial.print(" CHARS=");
  @fix@Serial.print(chars);
  @fix@Serial.print(" SENTENCES=");
  @fix@Serial.print(sentences);
  @fix@Serial.print(" CSUM ERR=");
  @fix@Serial.println(failed);
  if (chars == 0)
    @fix@Serial.println("** No characters received from GPS: check wiring **");
}