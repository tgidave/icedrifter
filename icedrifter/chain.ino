
#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "chain.h"

SoftwareSerial schain(CHAIN_RX, CHAIN_TX); 

int processChainData(uint8_t *tempDataPtr, uint8_t *lightDataPtr) {

  int i = 0;

  int chainByteCount;
  int lightByteCount;
  int totalByteCount;

  uint8_t* tmpPtr;
  uint16_t* wordPtr;

  uint32_t startTime;
  uint32_t tempTime;

  uint16_t waitSeconds;

  int chainError;

  uint8_t tmp;

  float ltClear;

  uint8_t rgbRed;
  uint8_t rgbGreen;
  uint8_t rgbBlue;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("\r\nPowering up chain.\r\n"));
#endif // SERIAL_DEBUG

  digitalWrite(CHAIN_POWER_PIN, HIGH);
  delay(1000);

  schain.begin(9600);
  schain.listen();

  tmpPtr = tempDataPtr;
  chainError = 0;

  schain.print(F("+1::chain\n"));
  startTime = millis();
  waitSeconds = 0;
  chainByteCount = 0;

  while (chainByteCount < (TEMP_DATA_SIZE)) {
    if (schain.available()) {
      *tmpPtr = schain.read();
      ++chainByteCount;
      ++tmpPtr;
    }

    if ((millis() - startTime) > (2UL * 60UL * 1000UL)) {
      chainError |= TEMP_CHAIN_TIMEOUT_ERROR;
      break;
    }
  }

  totalByteCount = chainByteCount;

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Received "));
  DEBUG_SERIAL.print(chainByteCount);
  DEBUG_SERIAL.print(F(" bytes of chain -data.\r\n"));

  if (chainError != 0) {
    DEBUG_SERIAL.print(F("\r\nTimeout on temp chain!!!\r\n"));
    DEBUG_SERIAL.print((TEMP_DATA_SIZE));
    DEBUG_SERIAL.print(F(" bytes requested but only "));
    DEBUG_SERIAL.print(chainByteCount);
    DEBUG_SERIAL.print(F(" bytes received\r\n"));
  }
#endif // SERIAL_DEBUG

  if (schain.available()) {
    chainError |= TEMP_CHAIN_OVERRUN_ERROR;

#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("\r\nToo much chain data received!!!\r\n"));
    DEBUG_SERIAL.print(F("\r\nReturning with chainError = "));
    DEBUG_SERIAL.print(chainError);
    DEBUG_SERIAL.print(F("\r\n"));
#endif // SERIAL_DEBUG

    return (chainError);
  }

  tmpPtr = lightDataPtr;

  schain.print(F("+1::light\n"));
  startTime = millis();
  waitSeconds = 0;
  lightByteCount = 0;

  while (lightByteCount < (LIGHT_DATA_SIZE)) {
    if (schain.available()) {
      *tmpPtr = schain.read();
      ++lightByteCount;
      ++tmpPtr;
    }

    if ((millis() - startTime) > (2UL * 60UL * 1000UL)) {
      chainError |= LIGHT_CHAIN_TIMEOUT_ERROR;
      break;
    }
  }

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("Received "));
  DEBUG_SERIAL.print(lightByteCount);
  DEBUG_SERIAL.print(F(" bytes of light data.\r\n"));

  if (chainError != 0) {
    DEBUG_SERIAL.print(F("\r\nTimeout on light chain!!!\r\n"));
    DEBUG_SERIAL.print((TEMP_DATA_SIZE + LIGHT_DATA_SIZE));
    DEBUG_SERIAL.print(F(" bytes requested but only "));
    DEBUG_SERIAL.print(chainByteCount);
    DEBUG_SERIAL.print(F(" bytes received\r\n"));
  }
#endif // SERIAL_DEBUG

  if (schain.available()) {
    chainError |= LIGHT_CHAIN_OVERRUN_ERROR;
#ifdef SERIAL_DEBUG
    DEBUG_SERIAL.print(F("\r\nToo much light data received!!!\r\n"));
#endif // SERIAL_DEBUG
  }

#ifdef SERIAL_DEBUG
  DEBUG_SERIAL.print(F("\r\nReturning with chainError = "));
  DEBUG_SERIAL.print(chainError);
  DEBUG_SERIAL.print(F("\r\n"));
#endif // SERIAL_DEBUG

  digitalWrite(CHAIN_POWER_PIN, LOW);

  return (chainError);
}


