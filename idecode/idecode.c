#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../icedrifter/icedrifter.h"

icedrifterData idData;

#define BUFFSIZE 2048

char buff[BUFFSIZE];

uint32_t ltClear;

uint8_t rgbRed;
uint8_t rgbGreen;
uint8_t rgbBlue;

//void convertStringToStruct(char* charPtr);
char convertCharToHex(char);
void convertBigEndianToLittleEndian(char* sPtr, int size);
float convertTempToC(short temp);


int main(int argc, char** argv) {
  char* cunkPtr;
  char* wkPtr;
  int argIx;

  int i;
  char hb1;
  char hb2;
  char buffHold;
//  int dataLen;
  struct tm* timeInfo;
  time_t tempTime;

  if (argc != 2)  {
    printf("Invalid number of arguments received!\r\n");
    exit(1);
  }

  argIx = 0;
  wkPtr = buff;
  
  while (1) {
    hb1 = hb2 = 0;

    if (!((argv[1][argIx] == 0) ||
          (argv[1][argIx] == '\r') ||
          (argv[1][argIx] == '\n'))) {
      if ((hb1 = convertCharToHex(argv[1][argIx])) == 0xFF) {
        printf("\r\nInvalid hex charactor found at location %d!!!\r\n", i);
        exit(1);
      }
    } else {
      break;
    }
   
    if (!((argv[1][argIx + 1] == 0) ||
          (argv[1][argIx + 1] == '\r') ||
          (argv[1][argIx + 1] == '\n'))) {
      if ((hb2 = convertCharToHex(argv[1][argIx + 1])) == 0xFF) {
        printf("\r\nInvalid hex charactor found at location %d!!!\r\n", (i + 1));
        exit(1);
      }
    }

    *wkPtr = (hb1 << 4) | hb2;
    ++wkPtr;
    argIx += 2;
  }

  if (argIx < (BASE_RECORD_LENGTH * 2)) {
    printf("\r\nNot enough data for base record!!!\r\n");
    exit(1);
  }
  
  wkPtr = buff;

  if ((buff[4] == 'I') && (buff[5] == 'D')) {
    wkPtr += 8;
  }

    memmove((char*)&idData, wkPtr, BASE_RECORD_LENGTH);

    wkPtr += BASE_RECORD_LENGTH;

    if (idData.idSwitches & PROCESS_CHAIN_DATA_SWITCH) {
      if (idData.idTempByteCount > 0) {
        memmove((char*)&idData.idChainData.cdTempData, wkPtr, idData.idTempByteCount);
        wkPtr = (char *)&idData.idChainData.cdTempData;
      }

      wkPtr += idData.idTempByteCount;

      if (idData.idLightByteCount != 0) {
        memmove((char*)&idData.idChainData.cdLightData, wkPtr, idData.idLightByteCount);
      }
    }

    convertBigEndianToLittleEndian((char*)&idData.idChainData, sizeof(idData.idChainData));

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

void convertBigEndianToLittleEndian(char* sPtr, int size) {
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
