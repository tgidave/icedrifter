
#include <IridiumSBD.h>
#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "rockblock.h"


SoftwareSerial rbSerial(ROCKBLOCK_RX_PIN, ROCKBLOCK_TX_PIN);

IridiumSBD isbd(rbSerial, ROCKBLOCK_SLEEP_PIN);

iceDrifterChunk idcChunk;

void rbTransmitIcedrifterData(icedrifterData* idPtr, int idLen) {

#ifdef NEVER_TRANSMIT

#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.print(F("Transmission disabled by NEVER_TRANSMIT switch.\r\n"));
#endif

#else // NEVER_TRANSMIT

  int rc;
  int recCount;
  int dataLen;
  int chunkLen;
  int i;
  uint8_t* dataPtr;
  uint8_t* chunkPtr;
  uint8_t* wkPtr;

  // Setup the RockBLOCK
#ifdef SERIAL_DEBUG_ROCKBLOCK
  isbd.attachConsole(DEBUG_SERIAL);
  isbd.attachDiags(DEBUG_SERIAL);
#endif // SERIAL_DEBUG_ROCKBLOCK
  isbd.setPowerProfile(1);

#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.println(F("Powering up RockBLOCK\r\n"));
  DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

  digitalWrite(ROCKBLOCK_POWER_PIN, HIGH);
  delay(1000);
  rbSerial.begin(ROCKBLOCK_BAUD);
  // Step 3: Start talking to the RockBLOCK and power it up
#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.println(F("RockBLOCK begin\r\n"));
  DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK
  rbSerial.listen();

  if ((rc = isbd.begin()) == ISBD_SUCCESS) {
#ifdef SERIAL_DEBUG_ROCKBLOCK
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.print(F("Transmitting address="));
    DEBUG_SERIAL.print((long)idPtr, HEX);
    DEBUG_SERIAL.print(F(" length="));
    DEBUG_SERIAL.print(idLen);
    DEBUG_SERIAL.print(F("\r\n"));
    DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

    recCount = 0;
    dataPtr = (uint8_t*)idPtr;
    chunkPtr = (uint8_t*)&idcChunk.idcBuffer;
    dataLen = idLen;

    while (dataLen > 0) {
      idcChunk.idcSendTime = idPtr->idGPSTime;
      idcChunk.idcRecordType[0] = 'I';
      idcChunk.idcRecordType[1] = 'D';
      idcChunk.idcRecordNumber = recCount;

      if (dataLen > MAX_CHUNK_DATA_LENGTH) {
        chunkLen = MAX_CHUNK_LENGTH;
        dataLen -= MAX_CHUNK_DATA_LENGTH;
      } else {
        chunkLen = (dataLen + CHUNK_HEADER_SIZE);
        dataLen = 0;
      }

      memmove(chunkPtr, dataPtr, chunkLen);
      dataPtr += MAX_CHUNK_DATA_LENGTH;
      ++recCount;

#ifdef SERIAL_DEBUG_ROCKBLOCK
      DEBUG_SERIAL.flush();
      DEBUG_SERIAL.print(F("Chunk address="));
      DEBUG_SERIAL.print((long)chunkPtr, HEX);
      DEBUG_SERIAL.print(F(" Chunk length="));
      DEBUG_SERIAL.print(chunkLen);
      DEBUG_SERIAL.print(F("\r\n"));
      wkPtr = (uint8_t *)&idcChunk;

      for (i = 0; i < chunkLen; i++) {
        printHexChar((uint8_t)*wkPtr);
        ++wkPtr;
      }

      DEBUG_SERIAL.print(F("\r\n"));
      DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK

      rc = isbd.sendSBDBinary((uint8_t*)&idcChunk, chunkLen);

#ifdef SERIAL_DEBUG_ROCKBLOCK
      DEBUG_SERIAL.flush();
      if (rc == 0) {
        DEBUG_SERIAL.print(F("Good return code from send!\r\n"));
        DEBUG_SERIAL.flush();
      } else {
        DEBUG_SERIAL.print(F("Bad return code from send = "));
        DEBUG_SERIAL.print(rc);
        DEBUG_SERIAL.print(F("\r\n"));
        DEBUG_SERIAL.flush();
      }
#endif // SERIAL_DEBUG_ROCKBLOCK
    }

#ifdef SERIAL_DEBUG_ROCKBLOCK
  } else {
    DEBUG_SERIAL.print("Bad return code from begin = ");
    DEBUG_SERIAL.print(rc);
    DEBUG_SERIAL.print("\r\n");
    DEBUG_SERIAL.flush();
#endif // SERIAL_DEBUG_ROCKBLOCK
  }

  isbd.sleep();
  rbSerial.end();
  digitalWrite(ROCKBLOCK_POWER_PIN, LOW);

#endif // NEVER_TRANSMIT

}


