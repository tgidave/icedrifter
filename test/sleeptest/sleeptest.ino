
#include "icedrifter.h"
#include "gps.h"
#include <avr/wdt.h>

#include <TinyGPS++.h> // NMEA parsing: http://arduiniana.org
#include <PString.h> // String buffer formatting: http://arduiniana.org

#define CONSOLE_BAUD 115200

bool gotFullFix;  // First time switch for getting a full GPS fix.

int noFixFoundCount;  // Number of times the GPS device could not get a fix.

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
  /*
  disablePower(POWER_ADC);
  disablePower(POWER_SPI); 
  disablePower(POWER_WIRE); 
  disablePower(POWER_TIMER0); 
  disablePower(POWER_TIMER1); 
  disablePower(POWER_TIMER3); 
  disablePower(POWER_SERIAL0); 
  disablePower(POWER_SERIAL1); 
  */ 
  ADCSRA &= ~(1 << ADEN);
  wdt_enable(SLEEP_8S);
  WDTCSR |= (1 << WDIE);	
  sleepMode(SLEEP_POWER_SAVE);
  sleep();
  noSleep();
//  enablePower(POWER_ALL);
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

//! setup - This is an arduino defined routine that is called only once after the processor is booted.
//!
void setup() {
  pinMode(GPS_POWER_PIN, OUTPUT);
  digitalWrite(GPS_POWER_PIN, LOW);

#ifdef SERIAL_DEBUG
  //! Start the serial ports
  DEBUG_SERIAL.begin(CONSOLE_BAUD);
#endif

  gotFullFix = false; //! Clear the GPS full fix switch so the first call to the loop function requests a full fix.

  DEBUG_SERIAL.print("Setup done\r\n"); //! Let the user know we are done with the setup function.

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

  int fixFound; //! indicates weather the last call to the GPS system returned a fix.

  int sleepSecs;  //! Number of seconds to sleep before the processor is woken up.
  int sleepMins;  //! Number of minutes to sleep before the processor is woken up.

  noFixFoundCount = 0;  //! clear the no fix found count.

  //! Check to see if a full fix was received.  If not, try to get a full fix.
  //! If so, just get a time fix.
  if (gotFullFix) { 
    fixFound = getGPSFix(FIX_TIME); 
  } else { 
    fixFound = getGPSFix(FIX_FULL);
  }

  //! If a GPS fix was received, set the gotFullFix switch and clear the noFixFound count.
  //! Otherwise add one to the noFixFoundCount.
  if (fixFound) {
    gotFullFix = true;
    noFixFoundCount = 0;
  } else {
    ++noFixFoundCount;
  }
  
  //! If a GPS fix was found
  if (fixFound) {
  /*
    //! Calculate the minutes until the next half hour,
    sleepMins = 90 - gpsGetMinute();
    //! If it less than 15 minutes until the nex half hour, 
    if (sleepMins >= 75) {
      sleepMins -= 60;
    }
*/
    sleepMins = 10;
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print("sleep ");
    DEBUG_SERIAL.print(sleepMins);
    DEBUG_SERIAL.print(" minutes\r\n");
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.end();
#endif
    sleepSecs = sleepMins * 60; 
  } else {
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print("sleep 60 minutes\r\n");
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.end();
#endif
//    sleepSecs = 3600;
    sleepSecs = 600;
  }

/*
#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print("sleep 5 minutes\r\n");
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.end();
#endif

  sleepSecs = 300;
*/
        
  do {
    powerDown();

    sleepSecs -= 8;

  } while (sleepSecs > 0);
#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.begin(CONSOLE_BAUD);
  DEBUG_SERIAL.print("wake up\r\n");
  DEBUG_SERIAL.flush();
#endif
}
