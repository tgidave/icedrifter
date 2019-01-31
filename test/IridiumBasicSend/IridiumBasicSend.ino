#include <IridiumSBD.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

#define CONSOLE_BAUD 115200

SoftwareSerial nss(12, 13);
IridiumSBD isbd(nss, 44);

static const int ROCKBLOCK_POWER = 15;

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
};

//! ISR - Interrupt Service Routine for handeling the watchdog timer interupt.  This routine
//! disables the WDT interupt and then returns.
//!
//! \param WDT_vect
ISR(WDT_vect) {
  //! WDIE & WDIF is cleared in hardware upon entering this ISR
  wdt_disable();
}

void setup() {

  int signalQuality = -1;

  pinMode(ROCKBLOCK_POWER, OUTPUT);
  digitalWrite(ROCKBLOCK_POWER, HIGH);
  delay(1000);


  Serial.begin(CONSOLE_BAUD);
  nss.begin(19200);

  Serial.print("Starting rockblock test\r\n");

  isbd.attachConsole(Serial);
  isbd.setPowerProfile(1);
  isbd.begin();

  int err = isbd.getSignalQuality(signalQuality);
  if (err != 0) {
    Serial.print("SignalQuality failed: error ");
    Serial.println(err);
    return;
  }

  Serial.print("Signal quality is ");
  Serial.println(signalQuality);

  err = isbd.sendSBDText("Hello, world!");
  if (err != 0) {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    return;
  }

  Serial.println("Hey, it worked!");
  Serial.print("Messages left: ");
  Serial.println(isbd.getWaitingMessageCount());

  digitalWrite(ROCKBLOCK_POWER, LOW);
}

void loop() {

  int sleepSecs;  //! Number of seconds to sleep before the processor is woken up.
  int sleepMins;  //! Number of minutes to sleep before the processor is woken up.

  Serial.print("sleep 60 minutes\r\n");
  Serial.flush();
  Serial.end();

  sleepMins = 60;
  sleepSecs = sleepMins * 60;

  do {
    powerDown();

    sleepSecs -= 8;

  } while (sleepSecs > 0);

  Serial.begin(CONSOLE_BAUD);
  Serial.print("wake up\r\n");
  Serial.flush();
}

bool ISBDCallback() {
  return true;
}
