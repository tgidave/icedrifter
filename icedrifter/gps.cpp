#include <time.h>
#include <TinyGPS++.h>  // NMEA parsing: http://arduiniana.org
#include <PString.h>    // String buffer formatting: http://arduiniana.org

#include "icedrifter.h"
#include "gps.h"



TinyGPSPlus tinygps;

static int fixfnd;
static struct tm timeStru;

int gpsGetFix(fixType typeFix, icedrifterData * idData) {

unsigned long now;

#ifdef  SERIAL_DEBUG_GPS
  char outBuffer[OUTBUFFER_SIZE];
#endif

  digitalWrite(GPS_POWER_PIN, HIGH);
  GPS_SERIAL.begin(GPS_BAUD);
  fixfnd = false;

  // Step 1: Reset TinyGPS++ and begin listening to the GPS
#ifdef SERIAL_DEBUG_GPS
  DEBUG_SERIAL.println(F("Beginning GPS"));
#endif
  tinygps = TinyGPSPlus();

  // Step 2: Look for GPS signal for up to 7 minutes
  for (now = millis(); !fixfnd && ((millis() - now) < (5UL * 60UL * 1000UL));) {
    if (GPS_SERIAL.available()) {
      tinygps.encode(GPS_SERIAL.read());
      fixfnd = tinygps.location.isValid() && tinygps.date.isValid() &&
               tinygps.time.isValid();
    }
  }

  if (fixfnd) {
//#ifdef SERIAL_DEBUG_GPS
//    DEBUG_SERIAL.print(tinygps.date.year());
//    DEBUG_SERIAL.print(F("\n"));
//#endif
    timeStru.tm_year = tinygps.date.year() - 1900;
    timeStru.tm_mon = tinygps.date.month() - 1;
    timeStru.tm_mday = tinygps.date.day();
    timeStru.tm_hour = tinygps.time.hour();
    timeStru.tm_min = tinygps.time.minute();
    timeStru.tm_sec = tinygps.time.second();
    idData->idGPSTime = mktime(&timeStru);
    idData->idLatitude = tinygps.location.lat();
    idData->idLongitude = tinygps.location.lng();

#ifdef SERIAL_DEBUG_GPS
    *outBuffer = 0;
    PString str(outBuffer, OUTBUFFER_SIZE);
    str.print(F("fix found!\n"));
    str.print(timeStru.tm_year + 1900);
    str.print(F("/"));
    str.print(timeStru.tm_mon + 1);
    str.print(F("/"));
    str.print(timeStru.tm_mday);
    str.print(F(" "));
    str.print(timeStru.tm_hour);
    str.print(F(":"));
    str.print(timeStru.tm_min);
    str.print(F(":"));
    str.print(timeStru.tm_sec);
    str.print(F(" "));
    str.print(idData->idLatitude, 6);
    str.print(F(","));
    str.print(idData->idLongitude, 6);
    str.print(F("\n"));
    DEBUG_SERIAL.print(outBuffer);
#endif

  }

#ifdef SERIAL_DEBUG_GPS
  else {
    DEBUG_SERIAL.print(F("No fix found.\n"));
  }
#endif

  GPS_SERIAL.end();
  digitalWrite(GPS_POWER_PIN, LOW);
  digitalWrite(GPS_RX_PIN, LOW); 
  digitalWrite(GPS_TX_PIN, LOW); 
  return (fixfnd);
}

int8_t gpsGetHour(void) {

  if (fixfnd) {
#ifdef SERIAL_DEBUG_GPS
    DEBUG_SERIAL.print(F("Fix Found - gpsGetHour returning "));
    DEBUG_SERIAL.print(timeStru.tm_min);
    DEBUG_SERIAL.print(F("\n"));
#endif
    return (timeStru.tm_hour);
  }

#ifdef SERIAL_DEBUG_GPS
    DEBUG_SERIAL.print(F("Fix Not Found - gpsGetHour returning -1!"));
#endif
  return (-1);
}

int8_t gpsGetMinute(void) {

  if (fixfnd) {
#ifdef SERIAL_DEBUG_GPS
    DEBUG_SERIAL.print(F("Fix Found - gpsGetMinute returning "));
    DEBUG_SERIAL.print(timeStru.tm_min);
    DEBUG_SERIAL.print(F("\n"));
#endif
    return (timeStru.tm_min);
  }

#ifdef SERIAL_DEBUG_GPS
    DEBUG_SERIAL.print(F("Fix Not Found - gpsGetMinute returning -1!"));
#endif
  return (-1);
}
