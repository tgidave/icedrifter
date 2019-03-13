
#include <IridiumSBD.h>
#include <SoftwareSerial.h>

#include "icedrifter.h"
#include "rockblock.h"


SoftwareSerial rbSerial(ROCKBLOCK_RX_PIN, ROCKBLOCK_TX_PIN);

IridiumSBD isbd(rbSerial, ROCKBLOCK_SLEEP_PIN);

iceDrifterChunk idcChunk;

void rbTransmitIcedrifterData(icedrifterData* idPtr, int idLen) {

  int rc;
  int recCount;
  int dataLen;
  int chunkLen;
  int i;
  uint8_t* dataPtr;
  uint8_t* chunkPtr;
  uint8_t* wkPtr; 

//  dataLen = idLen;
  // Setup the RockBLOCK
#ifdef SERIAL_DEBUG_ROCKBLOCK
  isbd.attachConsole(DEBUG_SERIAL);
  isbd.attachDiags(DEBUG_SERIAL);
#endif
  isbd.setPowerProfile(1);

#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.println(F("Powering up RockBLOCK\r\n"));
  DEBUG_SERIAL.flush();
#endif

  digitalWrite(ROCKBLOCK_POWER_PIN, HIGH);
  delay(1000);
  rbSerial.begin(ROCKBLOCK_BAUD);
  // Step 3: Start talking to the RockBLOCK and power it up
#ifdef SERIAL_DEBUG_ROCKBLOCK
  DEBUG_SERIAL.flush();
  DEBUG_SERIAL.println(F("RockBLOCK begin\r\n"));
  DEBUG_SERIAL.flush();
#endif
  rbSerial.listen();

  if ((rc = isbd.begin()) == ISBD_SUCCESS) {
#ifdef SERIAL_DEBUG_ROCKBLOCK
    DEBUG_SERIAL.flush();
    DEBUG_SERIAL.print(F("Transmitting address="));
    DEBUG_SERIAL.print((long)idPtr);
    DEBUG_SERIAL.print(F(" length="));
    DEBUG_SERIAL.print(idLen);
    DEBUG_SERIAL.print(F("\r\n"));

    DEBUG_SERIAL.flush();
#endif

    recCount = 0;
    dataPtr = (uint8_t *)idPtr;
    chunkPtr = (uint8_t *)&idcChunk.idcBuffer;
    dataLen = idLen;

    while (dataLen > 0) {
      idcChunk.idcSendTime = idPtr->idGPSTime;
      idcChunk.idcRecordType[0] = 'I';
      idcChunk.idcRecordType[1] = 'D';
      idcChunk.idcRecordNumber = recCount;

      if (dataLen > MAX_CHUNK_LENGTH) {
        chunkLen = (MAX_CHUNK_LENGTH + CHUNK_HEADER_SIZE);
        dataLen -= MAX_CHUNK_LENGTH;
        dataPtr += MAX_CHUNK_LENGTH;
      } else {
        chunkLen = (dataLen + CHUNK_HEADER_SIZE);
        dataLen = 0;
      }

      memmove(chunkPtr, idPtr, chunkLen);
      ++recCount;

#ifdef SERIAL_DEBUG_ROCKBLOCK
      DEBUG_SERIAL.flush();
      DEBUG_SERIAL.print(F("Chunk address="));
      DEBUG_SERIAL.print((long)chunkPtr);
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

#ifdef NEVER_TRANSMIT
      DEBUG_SERIAL.print(F("Transmission disabled by NEVER_TRANSMIT switch.\r\n"));
#else
      rc = isbd.sendSBDBinary((uint8_t *)&idcChunk, chunkLen);
#endif // NEVER_TRANSMIT

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
#endif
    }

#ifdef SERIAL_DEBUG_ROCKBLOCK
  } else {
    DEBUG_SERIAL.print("Bad return code from begin = ");
    DEBUG_SERIAL.print(rc);
    DEBUG_SERIAL.print("\r\n");
    DEBUG_SERIAL.flush();
#endif
  }

  isbd.sleep();
  rbSerial.end();
  digitalWrite(ROCKBLOCK_POWER_PIN, LOW);
}



