
#include <SoftwareSerial.h>
#include <TinyGPS++.h>  // NMEA parsing: http://arduiniana.org
#include <PString.h>    // String buffer formatting: http://arduiniana.org

#include "gps.h"

//To turn off the debugging messages, comment out the next line.

#define SERIAL_DEBUG

//The following defines are used to control what data is transmitted during debugging.
//If "SERIAL_DEBUG" is not defined they have no effect.

#ifdef SERIAL_DEBUG
#define SERIAL_DEBUG_TIME
#define SERIAL_DEBUG_GPS
#define SERIAL_DEBUG_TEMP
#define SERIAL_DEBUG_IMU
#define SERIAL_DEBUG_ROCKBLOCK

#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200
#endif

#define GPS_RX_PIN 12
#define GPS_TX_PIN 10
#define GPS_BAUD 9600
#define GPS_POWER_PIN 14

SoftwareSerial ssGPS(GPS_RX_PIN, GPS_TX_PIN); 

TinyGPSPlus tinygps;

int fixfnd = false;
unsigned long now; 

char outBuffer[OUTBUFFER_SIZE]; 

void setup(void) {

#ifdef SERIAL_DEBUG
  // Start the serial ports
  DEBUG_SERIAL.begin(DEBUG_BAUD);
//  delay(1000);
  DEBUG_SERIAL.print("\r\n\r\nDebugging start.\r\n");
#endif

  pinMode(GPS_POWER_PIN, OUTPUT);
  digitalWrite(GPS_POWER_PIN, HIGH);
  delay(1000);
  ssGPS.begin(GPS_BAUD);
}

void loop(void) {

  // Step 1: Reset TinyGPS++ and begin listening to the GPS
#ifdef SERIAL_DEBUG_GPS
  DEBUG_SERIAL.println("Beginning GPS");
#endif
  tinygps = TinyGPSPlus();

  // Step 2: Look for GPS signal for up to 7 minutes
  for (now = millis(); !fixfnd && ((millis() - now) < (5UL * 60UL * 1000UL));) {
    if (ssGPS.available()) {
//      DEBUG_SERIAL.println("GPS available!");
      tinygps.encode(GPS_SERIAL.read());
      fixfnd = tinygps.location.isValid() && tinygps.date.isValid() &&
          tinygps.time.isValid() && tinygps.altitude.isValid();
    } else {
        DEBUG_SERIAL.println("GPS not available!");
        delay(1000);
    }
  }

#ifdef SERIAL_DEBUG_GPS
  while (1) {
    if (fixfnd) {
      *outBuffer = 0;
      PString str(outBuffer, OUTBUFFER_SIZE);
      str.print("fix found! ");
      str.print(tinygps.location.lat(), 6);
      str.print(",");
      str.print(tinygps.location.lng(), 6);
      str.print(",");
      str.print(tinygps.speed.knots(), 1);
      str.print(",");
      str.print(tinygps.altitude.meters());
      str.print(",");
      str.print(tinygps.course.value() / 100);
      str.print("\r\n");
      DEBUG_SERIAL.print(outBuffer);
    } else {
        DEBUG_SERIAL.print("No fix found.\r\n");
    }
    delay(1000);
  }
#endif

}

int gpsGetYear(void) { 
  return(tinygps.date.year());
}

int gpsGetMonth(void) {
  return(tinygps.date.month());
}

int gpsGetDay(void) {   
  return(tinygps.date.day());
}

int gpsGetHour(void) {  
  return(tinygps.time.hour());
}

int gpsGetMinute(void) {
  return(tinygps.time.minute());
}

int gpsGetSecond(void) {
  return(tinygps.time.second());
}

float gpsGetLatitude(void) {
  return(tinygps.location.lat());
}

float gpsGetLongitude(void) {
  return(tinygps.location.lng());
}

float gpsGetAltitude(void) {
  return(tinygps.altitude.meters());
}

float gpsGetSpeed(void) {
  return(tinygps.speed.knots());
}

float gpsGetCourse(void) {
  return(tinygps.course.deg());
}



