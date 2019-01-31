
#include <TinyGPS++.h>  // NMEA parsing: http://arduiniana.org
#include <PString.h>    // String buffer formatting: http://arduiniana.org

#include "icedrifter.h"
#include "gps.h"

TinyGPSPlus tinygps;

int getGPSFix(fixType typeFix) {

//  int i;
  int fixfnd = false;
  unsigned long now;
//  time_t GPSTime;
//  int notAvailableCount;
  char outBuffer[OUTBUFFER_SIZE];

//  loopStartTime = millis();

  digitalWrite(GPS_POWER_PIN, HIGH);
  GPS_SERIAL.begin(GPS_BAUD);

  // Step 1: Reset TinyGPS++ and begin listening to the GPS
#ifdef SERIAL_DEBUG_GPS
  DEBUG_SERIAL.println("Beginning GPS");
#endif
  tinygps = TinyGPSPlus();

  // Step 2: Look for GPS signal for up to 7 minutes
  for (now = millis(); !fixfnd && ((millis() - now) < (5UL * 60UL * 1000UL));) {

    if (GPS_SERIAL.available()) {
      tinygps.encode(GPS_SERIAL.read());

      if (typeFix == FIX_FULL) {
        fixfnd = tinygps.location.isValid() && tinygps.date.isValid() &&
            tinygps.time.isValid() && tinygps.altitude.isValid();

      } else {
        fixfnd = tinygps.date.isValid() && tinygps.time.isValid();
      }
    }
  }

#ifdef SERIAL_DEBUG_GPS
  if (fixfnd) {
    *outBuffer = 0;
    PString str(outBuffer, OUTBUFFER_SIZE);
    str.print("fix found!\r\n"); 
    str.print(tinygps.date.year());
    str.print("/");
    str.print(tinygps.date.month());
    str.print("/");
    str.print(tinygps.date.day());
    str.print(" ");
    str.print(tinygps.time.hour());
    str.print(":");
    str.print(tinygps.time.minute());
    str.print(":");
    str.print(tinygps.time.second());

    if (typeFix == FIX_FULL) {
      str.print(" ");
      str.print(tinygps.location.lat(), 6);
      str.print(",");
      str.print(tinygps.location.lng(), 6);
      str.print(",");
      str.print(tinygps.speed.knots(), 1);
      str.print(",");
      str.print(tinygps.altitude.meters());
      str.print(",");
      str.print(tinygps.course.value() / 100);
    }

    str.print("\r\n"); 

    DEBUG_SERIAL.print(outBuffer);
  } else {
    DEBUG_SERIAL.print("No fix found.\r\n");
  }
#endif

  GPS_SERIAL.end();
  digitalWrite(GPS_POWER_PIN, LOW);
  return (fixfnd);
}

int gpsGetYear(void) {
  return (tinygps.date.year());
}

int gpsGetMonth(void) {
  return (tinygps.date.month());
}

int gpsGetDay(void) {
  return (tinygps.date.day());
}

int gpsGetHour(void) {
  return (tinygps.time.hour());
}

int gpsGetMinute(void) {
  return (tinygps.time.minute());
}

int gpsGetSecond(void) {
  return (tinygps.time.second());
}

float gpsGetLatitude(void) {
  return (tinygps.location.lat());
}

float gpsGetLongitude(void) {
  return (tinygps.location.lng());
}

float gpsGetAltitude(void) {
  return (tinygps.altitude.meters());
}

float gpsGetSpeed(void) {
  return (tinygps.speed.knots());
}

float gpsGetCourse(void) {
  return (tinygps.course.deg());
}



