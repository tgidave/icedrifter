/// ///////////////////////////////////////////////////////////
/// icedrifter.ino 
///  
/// This program implements the functionality neede to run the 
/// icedrifter sensor array. 
/// 
/// There are four sensors and a communication devices
/// controlled by this code:
/// 
/// 1. a GPS module
/// 2. a BMP280 module to collect temperature and air pressure.
/// 3. a remote temperature sensor driven by a DS18B23.
/// 4. an optional temperature and light chain.
/// 5. an Iridium network comminication device,
/// 
/// The system sleeps for 24 hours and then wakes up and
/// collects data.  After all data is collected it is
/// transmitted through the Iridium network to the user.
/// 
/// ///////////////////////////////////////////////////////////


#include <TimeLib.h>
#include <Time.h>

//#include <arduino.h>
#include <avr/wdt.h>

#include <TinyGPS++.h> // NMEA parsing: http://arduiniana.org
#include <PString.h> // String buffer formatting: http://arduiniana.org
//#include <time.h>

#include "icedrifter.h"
#include "gps.h"
#include "bmp280.h"
#include "ds18b20.h"
#include "rockblock.h"

#define CONSOLE_BAUD 115200

bool firstTime;
bool gotFullFix;  // First time switch for getting a full GPS fix.

int noFixFoundCount;  // Number of times the GPS device could not get a fix.

int fixFound; //! indicates weather the last call to the GPS system returned a fix.

icedrifterData idData;

time_t lbTime;

enum period_t {
  SLEEP_15MS,
  SLEEP_30MS,
  SLEEP_60MS,
  SLEEP_120MS,
  SLEEP_250MS,
  SLEEP_500MS,
  SLEEP_1S,
  SLEEP_2S,
  SLEEP_4S,
  SLEEP_8S,
  SLEEP_FOREVER
};

//! powerDown - Put processor into low power mode.
//! 
//! This function first set up the watchdog timer to go of after the maxiuum interval 
//! of 8 seconds and then puts the processor into low power sleep node.  After 
//! approximately 8 seconds the interval time will expire and wake up the processor
//! and the program continues.
//!
//! \param void 

void powerDown(void) {
  ADCSRA &= ~(1 << ADEN);
  wdt_enable(SLEEP_8S);
  WDTCSR |= (1 << WDIE);	
  sleepMode(SLEEP_POWER_SAVE);
  sleep();
  noSleep();
}

//! ISR - Interrupt Service Routine for handeling the watchdog timer interupt.  This routine
//! disables the WDT interupt and then returns.
//!
//! \param WDT_vect 
ISR (WDT_vect)
{
	//! WDIE & WDIF is cleared in hardware upon entering this ISR
	wdt_disable();
}

void accumulateAndSendData(void) {

  idData.idRecordType[0] = 'I';
  idData.idRecordType[1] = 'D';
  idData.idRecordType[2] = '0';
  idData.idRecordType[3] = '0';

  idData.idLastBootTime = lbTime;

  if ((fixFound = gpsGetFix(FIX_FULL, &idData)) == false) {
    idData.idGPSTime = 0;
    idData.idLatitude = 0;
    idData.idLongitude = 0;
    idData.idAltitude = 0;
    idData.idSpeed = 0;
    idData.idCourse = 0;
  }

  getBMP280Data(&idData);
  getRemoteTemp(&idData);

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Sending this data:\r\n"));
  DEBUG_SERIAL.print(F("idData.idRecordType   = "));
  DEBUG_SERIAL.print(idData.idRecordType[0]); 
  DEBUG_SERIAL.print(idData.idRecordType[1]); 
  DEBUG_SERIAL.print(idData.idRecordType[2]); 
  DEBUG_SERIAL.print(idData.idRecordType[3]);
  DEBUG_SERIAL.print(F("\r\nidData.idLastBootTime = "));
  DEBUG_SERIAL.print(idData.idLastBootTime);
  DEBUG_SERIAL.print(F("\r\nidData.idGPSTime      = "));
  DEBUG_SERIAL.print(idData.idGPSTime);
  DEBUG_SERIAL.print(F("\r\nidData.Latitude       = "));
  DEBUG_SERIAL.print(idData.idLatitude);
  DEBUG_SERIAL.print(F("\r\nidData.idLongitude    = "));
  DEBUG_SERIAL.print(idData.idLongitude);
  DEBUG_SERIAL.print(F("\r\nidData.idAltitude     = "));
  DEBUG_SERIAL.print(idData.idAltitude); 
  DEBUG_SERIAL.print(F("\r\nidData.idSpeed        = "));
  DEBUG_SERIAL.print(idData.idSpeed);
  DEBUG_SERIAL.print(F("\r\nidData.idCourse       = "));
  DEBUG_SERIAL.print(idData.idCourse);
  DEBUG_SERIAL.print(F("\r\nidData.idTemperature  = "));
  DEBUG_SERIAL.print(idData.idTemperature);
  DEBUG_SERIAL.print(F("\r\nidData.idPressure     = "));
  DEBUG_SERIAL.print(idData.idPressure);
  DEBUG_SERIAL.print(F("\r\nidData.idRemoteTemp   = ")); 
  DEBUG_SERIAL.print(idData.idRemoteTemp);
  DEBUG_SERIAL.print(F("\r\n"));
#endif

  transmitIcedrifterData(&idData, sizeof(idData));
}

//! setup - This is an arduino defined routine that is called only once after the processor is booted.
//!
void setup() {
  pinMode(GPS_POWER_PIN, OUTPUT);
  digitalWrite(GPS_POWER_PIN, LOW);

  pinMode(BMP280_POWER_PIN, OUTPUT);
  digitalWrite(BMP280_POWER_PIN, LOW);

  pinMode(DS18B20_POWER_PIN, OUTPUT);
  digitalWrite(DS18B20_POWER_PIN, LOW);

  pinMode(ROCKBLOCK_POWER_PIN, OUTPUT);
  digitalWrite(ROCKBLOCK_POWER_PIN, LOW);

#ifdef SERIAL_DEBUG
  //! Start the serial ports
  DEBUG_SERIAL.begin(CONSOLE_BAUD);
#endif

  gotFullFix = false; //! Clear the GPS full fix switch so the first call to the loop function requests a full fix.
  firstTime = true;
  
#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Setup done\r\n")); //! Let the user know we are done with the setup function.
#endif

}

//! loop - This is the main processing function for the arduino system.
//!
//! The first time through this function a full GPS fix is requested.  If no fix is 
//! received the processor is put to sleep for 60 minutes and then a full GPS fix 
//! will be requested again.  This continues until a full fix is received.
//!
//! Upon receiving a full GPS fix. the minutes are calculated to wake up the processor 
//! at the next half hour and the processor is put to sleep.
//! 
//! Once a full GPS fix is received, only the current time is requested from the GPS.
//! That's all that is needed to calculate the minutes to the next wake up time.
void loop() {

  int sleepSecs;  //! Number of seconds to sleep before the processor is woken up.
  int sleepMins;  //! Number of minutes to sleep before the processor is woken up.

  noFixFoundCount = 0;  //! clear the no fix found count.

  //! Check to see if a full fix was received.  If not, try to get a full fix.
  //! If so, just get a time fix.
  if (gotFullFix) { 
    fixFound = gpsGetFix(FIX_TIME, &idData); 
  } else { 
    fixFound = gpsGetFix(FIX_FULL, &idData);
  }

  //! If a GPS fix was received, set the gotFullFix switch and clear the noFixFound count.
  //! Otherwise add one to the noFixFoundCount.
  if (fixFound) {
    gotFullFix = true;
    noFixFoundCount = 0;
    if (firstTime) {
      lbTime = idData.idLastBootTime = idData.idGPSTime;
    }
  } else {
    ++noFixFoundCount;
  }
  
#ifdef TEST_ALL
  accumulateAndSendData();
  fixFound = gpsGetFix(FIX_TIME, &idData);
#else
  if (!firstTime &&
      ((fixFound && ((hour(idData.idGPSTime) == 0) || (hour(idData.idGPSTime) == 12))) ||
      noFixFoundCount >= 24)) {
    noFixFoundCount = 0;
    accumulateAndSendData();

    // Accumulating and sending the data can take a while so update the time again.
    fixFound = gpsGetFix(FIX_TIME, &idData); 
  }
#endif
  
  firstTime = false;

  //! If a GPS fix was found
  if (fixFound) {
    //! Calculate the minutes until the next half hour,
    sleepMins = 90 - minute(idData.idGPSTime);
    //! If it less than 15 minutes until the nex half hour, 
    if (sleepMins >= 75) {
      sleepMins -= 60;
    }

#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("Fix found - sleep "));
    DEBUG_SERIAL.print(sleepMins);
    DEBUG_SERIAL.print(F(" minutes\r\n"));
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.end();
#endif
    sleepSecs = sleepMins * 60; 
  } else {
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("Fix not found - sleep 60 minutes\r\n"));
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.end();
#endif
    sleepSecs = 3600;
  }

  do {
    powerDown();

    sleepSecs -= 8;

  } while (sleepSecs > 0);
#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.begin(CONSOLE_BAUD);
  DEBUG_SERIAL.print(F("wake up\r\n"));
  DEBUG_SERIAL.flush();
#endif
}
