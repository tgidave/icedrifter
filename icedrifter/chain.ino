
#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "chain.h"

SoftwareSerial schain(CHAIN_RX, CHAIN_TX); 

int processChainData(uint8_t *chainPtr) {

  int i = 0;

  int chainByteCount;

  uint8_t* tmpPtr;
  uint16_t* wordPtr;

  uint32_t startTime;
  uint32_t tempTime;

  uint16_t waitSeconds;

  int timeoutError;

  uint8_t tmp;

  float ltClear;

  uint8_t rgbRed;
  uint8_t rgbGreen;
  uint8_t rgbBlue;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("\r\nPowering up chain.\r\n"));
#endif

  digitalWrite(CHAIN_POWER_PIN, HIGH);
  delay(1000);

  schain.begin(9600);
  schain.listen(); 

  schain.print(F("+1::measure\n"));
  startTime = millis();
  timeoutError = 0;
  waitSeconds = 0;
  chainByteCount = 0;
  tmpPtr = chainPtr;

  while (chainByteCount < (TEMP_DATA_SIZE + LIGHT_DATA_SIZE)) {
    if (schain.available()) {
      *tmpPtr = schain.read();
      ++chainByteCount;
      ++tmpPtr;
    }

    if ((millis() - startTime) > (2UL * 60UL * 1000UL)) {
      timeoutError = 1;
      break;
    }
  }

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Received "));
  DEBUG_SERIAL.print(chainByteCount);
  DEBUG_SERIAL.print(F(" bytes of data.\r\n"));

  if (timeoutError != 0) {
    DEBUG_SERIAL.print(F("\r\nTimeout on chain!!!\r\n"));
    DEBUG_SERIAL.print((TEMP_DATA_SIZE + LIGHT_DATA_SIZE));
    DEBUG_SERIAL.print(F(" bytes requested but only "));
    DEBUG_SERIAL.print(chainByteCount);
    DEBUG_SERIAL.print(F(" bytes received\r\n"));
  }
#endif

  if (schain.available()) {
    timeoutError += 2; 
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("\r\nToo much chain data received!!!\r\n"));
#endif
  }

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("\r\nReturning with timeoutError = "));
  DEBUG_SERIAL.print(timeoutError);
  DEBUG_SERIAL.print(F("\r\n"));
#endif

  return (timeoutError);
}


