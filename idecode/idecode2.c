#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "../icedrifter/icedrifter.h"
#include "../icedrifter/rockblock.h"

icedrifterData idData;

#define BUFFSIZE 2048

char buff[BUFFSIZE];

uint32_t ltClear;

uint8_t rgbRed;
uint8_t rgbGreen;
uint8_t rgbBlue;

//void convertStringToStruct(char* charPtr);

int decodeData(void);
int getDataByFile(char **);
int getDataByChar(char **, int);
int getDataByChunk(char **, int);
//int getDataInDirectory(char *);
char convertCharToHex(char);
void convertBigEndianToLittleEndian(char *sPtr, int size);
float convertTempToC(short temp);

int main(int argc, char **argv) {
//  char *cunkPtr;
//  char *wkPtr;
//  int argIx;
//  int i;
//  char hb1;
//  char hb2;
//  char buffHold;
//  struct tm *timeInfo;
//  time_t tempTime;

  if (argc == 1) {
    printf("\r\nError: No arguments specified!!!\r\n");
    printf("Help will appear here!!!\r\n");
    exit(1);
  }

  if (argv[1][0] == '-') {
    switch (argv[1][1]) {
    case 'c':
      if (getDataByChunk(&argv[2], argc - 2) != 0) {
        exit(1);
      }
      break;
    case 'f':
      if (getDataByFile(&argv[2]) != 0) {
        exit(1);
      }
      break;
    case 'h':
      printf("Help will appear here!!!\r\n");
      exit(0);
    default:
      printf("Invalid switch!!!\r\n");
      printf("Help will appear here!!!\r\n");
      exit(1);
    }
  } else {
    if (getDataByChar(&argv[1], argc - 1) != 0) {
      exit(1);
    }
  }
}

int getDataByChar(char **data, int cnt) {
  char *cunkPtr;
  char *wkPtr;
  iceDrifterChunk *idcPtr;
  char *recPtr;
  int recNum;
  int dataIx;
  int i;
  int recLen;
  char hb1;
  char hb2;
  char buffHold;
  struct tm *timeInfo;
  uint32_t tempTime;
  uint32_t timeHold;
  bool gotDate;

  wkPtr = buff;
  gotDate = false;

  for (recNum = 0; recNum < cnt; ++recNum) {

    dataIx = 0;
    recLen = 0;

    if (*data[recNum] == 0) {
      break;
    }

    while (1) {
      hb1 = hb2 = 0;

      if (!((data[recNum][dataIx] == 0) ||
            (data[recNum][dataIx] == '\r') ||
            (data[recNum][dataIx] == '\n'))) {
        if ((hb1 = convertCharToHex(data[recNum][dataIx])) == 0xFF) {
          printf("\r\nInvalid hex charactor found at location %d!!!\r\n", dataIx);
          exit(1);
        }
      } else {
        break;
      }

      if (!((data[recNum][dataIx + 1] == 0) ||
            (data[recNum][dataIx + 1] == '\r') ||
            (data[recNum][dataIx + 1] == '\n'))) {
        if ((hb2 = convertCharToHex(data[recNum][dataIx + 1])) == 0xFF) {
          printf("\r\nInvalid hex charactor found at location %d!!!\r\n", (i + 1));
          exit(1);
        }
      }

      *wkPtr = (hb1 << 4) | hb2;
      ++wkPtr;
      ++recLen;
      dataIx += 2;
    }

    idcPtr = (iceDrifterChunk *)buff;

    if (!((idcPtr->idcRecordType[0] == 'I') && (idcPtr->idcRecordType[1] == 'D'))) {
      printf("Record ID not = \"ID\"!!!\r\n");
      exit(1);
    }

    if (gotDate) {
      if (idcPtr->idcSendTime != timeHold) {
        printf("Time of record %d does not equal other record!!!", recNum);
        exit(1);
      }
    } else {
      timeHold = idcPtr->idcSendTime;
      gotDate = true;
    }

    recPtr = (char *)&idData;

    if (idcPtr->idcRecordNumber == 1) {
      recPtr += sizeof(idcPtr->idcBuffer);
    } else if (idcPtr->idcRecordNumber == 2) {
      recPtr += sizeof(idcPtr->idcBuffer) * 2;
    }

    wkPtr = idcPtr->idcBuffer;
    memmove(recPtr, wkPtr, recLen);
  }

  convertBigEndianToLittleEndian((char *)&idData.idChainData, sizeof(idData.idChainData));

  return (decodeData());
}

int decodeData() {

  struct tm *timeInfo;
  time_t tempTime;
  int i;

  tempTime = (time_t)idData.idLastBootTime;
  timeInfo = gmtime(&tempTime);
  printf("Last Boot:   %s", asctime(timeInfo));
  tempTime = (time_t)idData.idGPSTime;
  timeInfo = gmtime(&tempTime);
  printf("GPS time:    %s", asctime(timeInfo));

  if (idData.idcdError == 0) {
    printf("No errors found.\r\n");
  } else {
    printf("\r\nError(s) found!!!\r\n");
    if (idData.idcdError & TEMP_CHAIN_TIMEOUT_ERROR) {
      printf("*** Temperature chain timeout.\r\n");
    }
    if (idData.idcdError & TEMP_CHAIN_OVERRUN_ERROR) {
      printf("*** Temperature chain overrun.\r\n");
    }
    if (idData.idcdError & LIGHT_CHAIN_TIMEOUT_ERROR) {
      printf("*** Light chain timeout.\r\n");
    }
    if (idData.idcdError & LIGHT_CHAIN_OVERRUN_ERROR) {
      printf("*** Light chain overrun.\r\n");
    }
    printf("\r\n");
  }

  printf("Temp chain bytes received %d\r\n", idData.idTempByteCount);
  printf("Light chain bytes received %d\r\n\r\n", idData.idLightByteCount);

  printf("latitude:    %f\r\n", idData.idLatitude);
  printf("longitude:   %f\r\n", idData.idLongitude);
  printf("temperature: %f C\r\n", idData.idTemperature);
  printf("pressure:    %f Pa\r\n", idData.idPressure);

  if (idData.idSwitches & PROCESS_REMOTE_TEMP_SWITCH) {
    printf("remote temp: %f C\r\n\r\n", idData.idRemoteTemp);
  }

  if (idData.idSwitches & PROCESS_CHAIN_DATA_SWITCH) {
    for (i = 0; i < TEMP_SENSOR_COUNT; ++i) {
      printf("Chain temperature sensor %3d = %f\r\n", i, convertTempToC(idData.idChainData.cdTempData[i]));
    }

    printf("\r\n");

    for (i = 0; i < LIGHT_SENSOR_COUNT; ++i) {
      if (idData.idChainData.cdLightData[i][0] == 0) {
        rgbRed = rgbGreen = rgbBlue = 0;
      } else {
        ltClear = (uint32_t)idData.idChainData.cdLightData[i][0];
        rgbRed = (float)idData.idChainData.cdLightData[i][1] / ltClear * 255.0;
        rgbGreen = (float)idData.idChainData.cdLightData[i][2] / ltClear * 255.0;
        rgbBlue = (float)idData.idChainData.cdLightData[i][3] / ltClear * 255.0;
      }

      printf("Chain light sensor %2d = %5hu %5hu %5hu %5hu  RGB %3d %3d %3d\r\n", i,
             idData.idChainData.cdLightData[i][0], idData.idChainData.cdLightData[i][1], idData.idChainData.cdLightData[i][2], idData.idChainData.cdLightData[i][3],
             rgbRed, rgbGreen, rgbBlue);
    }
  }
  return (0);
}

char convertCharToHex(char chr) {
  if ((chr >= '0') && (chr <= '9')) {
    return (chr - '0');
  } else if ((chr >= 'a') && (chr <= 'f')) {
    return ((chr - 'a') + 10);
  } else if ((chr >= 'A') && (chr <= 'F')) {
    return ((chr - 'A') + 10);
  }

  return (0xFF);
}

void convertBigEndianToLittleEndian(char *sPtr, int size) {
  char tmp;
  int i;

  for (i = 0; i < (size / 2); ++i) {
    tmp = *sPtr;
    *sPtr = *(sPtr + 1);
    *(sPtr + 1) = tmp;
    sPtr += 2;
  }
}

float convertTempToC(short temp) {
  if (temp & 0x8000) {
    return ((float)((temp & 0x7FFF) - 0x8000) / 128.0);
  }

  return ((float)temp / 128.0);
}

int getDataByFile(char **fnl) {
  int recordSize;
  int i;
  char **argIx;
  iceDrifterChunk *idcPtr;
  FILE *fd;
  char *wkPtr;
  char fileBuffer[MAX_RECORD_LENGTH];

  wkPtr = (char *)&idData;
  argIx = fnl;

  for (i = 0; i < (BASE_RECORD_LENGTH + TEMP_DATA_SIZE + LIGHT_DATA_SIZE); ++i) {
    *wkPtr - 0;
  }

  if ((fd = fopen(argIx[0], "r")) == NULL) {
    printf("Error opening input file %s!\r\n", argIx[0]);
    printf("idecode terminating.\r\n");
    fclose(fd);
    exit(1);
  }

  if ((recordSize = fread(fileBuffer, sizeof(char), MAX_RECORD_LENGTH, fd) == 0)) {
    printf("Error reading data file %s!\r\n", argIx[0]);
    printf("idecode terminating.\r\n");
    fclose(fd);
    exit(1);
  }

  fclose(fd);

  idcPtr = (iceDrifterChunk *)&fileBuffer;

//      if (rcordSize != BASE_RECORD_LENGTH + CHUNK_HEADER_SIZE) {
//        printf("Invalid record size!  Expected %d bytes but recieved %d bytes!\r\n",
//               BASE_RECORD_LENGTH + CHUNK_HEADER_SIZE, recordLength);
//        printf("idecode terminating.\r\n");
//        exit(1);
//      }

  if (!((idcPtr->idcRecordType[0] == 'I') &&
        (idcPtr->idcRecordType[1] == 'D'))) {
    printf("Invalid record! Chnk header - not \"IDxx\"!\r\n");
    exit(1);
  }

  wkPtr = (char *)&idData;
  memmove(wkPtr, (char *)&idcPtr->idcBuffer, recordSize);
  decodeData();
}

int getDataByChunk(char **fnl, int cnt) {
  int recordSize;
  int i;
  char **argIx;
  iceDrifterChunk *idcPtr;
  FILE *fd;
  char *wkPtr;
  char fileBuffer[MAX_RECORD_LENGTH];

  wkPtr = (char *)&idData;
  argIx = fnl;

  for (i = 0; i < (BASE_RECORD_LENGTH + TEMP_DATA_SIZE + LIGHT_DATA_SIZE); ++i) {
    *wkPtr - 0;
  }

  for (i = 0; i < cnt; ++i) {

  if ((fd = fopen(argIx[i], "r")) == NULL) {
    printf("Error opening input file %s!\r\n", argIx[0]);
    printf("idecode terminating.\r\n");
    fclose(fd);
    exit(1);
  }

    if ((recordSize = fread(fileBuffer, sizeof(char), MAX_RECORD_LENGTH, fd) == 0)) {
      printf("Error reading data file %s!\r\n", argIx[0]);
      printf("idecode terminating.\r\n");
      fclose(fd);
      exit(1);
    }

    idcPtr = (iceDrifterChunk *)&fileBuffer;

    //      if (rcordSize != BASE_RECORD_LENGTH + CHUNK_HEADER_SIZE) {
    //        printf("Invalid record size!  Expected %d bytes but recieved %d bytes!\r\n",
    //               BASE_RECORD_LENGTH + CHUNK_HEADER_SIZE, recordLength);
    //        printf("idecode terminating.\r\n");
    //        exit(1);
    //      }

    if (!((idcPtr->idcRecordType[0] == 'I') &&
          (idcPtr->idcRecordType[1] == 'D'))) {
      printf("Invalid record! Chnk header - not \"IDxx\"!\r\n");
      exit(1);
    }

    if (idcPtr->idcRecordNumber == 0) {
      wkPtr = (char *)&idData;
      memmove(wkPtr, (char *)&idcPtr->idcBuffer, recordSize - CHUNK_HEADER_SIZE);
    } else if (idcPtr->idcRecordNumber == 1) {
      wkPtr = (char *)&idData;
      wkPtr += MAX_CHUNK_DATA_LENGTH;
      memmove(wkPtr, (char *)&idcPtr->idcBuffer, recordSize - CHUNK_HEADER_SIZE);
    } else if (idcPtr->idcRecordNumber == 2) {
      wkPtr = (char *)&idData;
      wkPtr += (MAX_CHUNK_DATA_LENGTH * 2);
      memmove(wkPtr, (char *)&idcPtr->idcBuffer, recordSize - CHUNK_HEADER_SIZE);
    } else {
      printf("Invalid record number!!! Record number = %d!!!\r\n", idcPtr->idcRecordNumber);
      fclose(fd);
      exit(1);
    }

    fclose(fd);
  }

  convertBigEndianToLittleEndian((char *)&idData.idChainData, sizeof(idData.idChainData));
  return (decodeData());

}
