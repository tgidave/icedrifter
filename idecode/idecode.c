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

void convertStringToStruct(char* charPtr);
uint16_t convertCharToHex(char* ptr);
void convertBigEndianToLittleEndian(char* sPtr, int size);
float convertTempToC(short temp);


int main(int argc, char** argv) {
  char* cunkPtr;
  char* wkPtr;
  int i;
  char buffHold;
  int dataLen;
  struct tm* timeInfo;
  time_t tempTime;

  if (argc != 2)  {
    printf("Invalid number of arguments received!\r\n");
    exit(1);
  }

  dataLen = 0;
  wkPtr = buff;

  for (i = 0; i < sizeof(buff); ++i) {
    *wkPtr = 0;
  }

  i = 0;

  while (!(argv[1][i] == 0) ||
         (argv[1][i] == '\r') ||
         (argv[1][i] == '\n')) {
    *wkPtr = argv[1][i];
    ++i;
    ++wkPtr;
    ++dataLen;

    if (dataLen > sizeof(buff)) {
      printf("too much data received!!!\r\n");
      exit(1);
    }
  }

  convertStringToStruct(buff);

  convertBigEndianToLittleEndian((char*)&idData.idChainData, sizeof(idData.idChainData));

  tempTime = (time_t)idData.idLastBootTime;
  timeInfo = gmtime(&tempTime);
  printf("Last Boot:   %s", asctime(timeInfo));
  tempTime = (time_t)idData.idGPSTime;
  timeInfo = gmtime(&tempTime);
  printf("GPS time:    %s", asctime(timeInfo));
  printf("latitude:    %f\r\n", idData.idLatitude);
  printf("longitude:   %f\r\n", idData.idLongitude);
  printf("temperature: %f C\r\n", idData.idTemperature);
  printf("pressure:    %f Pa\r\n", idData.idPressure);

  if (idData.idSwitches & PROCESS_REMOTE_TEMP_SWITCH) {
    printf("remote temp: %f C\r\n\r\n", idData.idRemoteTemp);
  }

  i = 0;

  while (idData.idTempSensorCount > 0) {
    printf("Chain temperature sensor %3d = %f\r\n", i, convertTempToC(idData.idChainData.cdTempData[i]));
    ++i;
    --idData.idTempSensorCount;
  }

  printf("\r\n");

  i = 0;

  while (idData.idLightSensorCount > 0) {
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

    ++i;
    --idData.idLightSensorCount;
  }
}

void convertStringToStruct(char* charPtr) {
  char* binPtr;
  int i = 0; 
  char hByte0;
  char hByte1;
//  int j = 0;
//  int len;

  binPtr = (char*)&idData;

  for (i = 0; i < BASE_RECORD_LENGTH; ++i) {
    if (charPtr == 0) {
      printf("Not enough data to complete base record!  Can not continue!!!\r\n");
      exit(1);
    }

    hByte0 = convertCharToHex(charPtr);
    ++charPtr;
    hByte1 = convertCharToHex(charPtr);
    ++charPtr;
    *binPtr = (hByte0 << 4) | hByte1;
    ++binPtr;
  }

  binPtr = (char*)&idData.idChainData.cdTempData;

  for (i = 0; i < (idData.idTempSensorCount * sizeof(short)); ++i) {
    if (charPtr == 0) {
      printf("Not enough data to complete temperature data structure!  Continuing but data may be corrupt!!!\r\n");
      break;
    }

    hByte0 = convertCharToHex(charPtr);
    ++charPtr;
    hByte1 = convertCharToHex(charPtr);
    ++charPtr;
    *binPtr = (hByte0 << 4) | hByte1;
    ++binPtr;
  }

  binPtr = (char*)&idData.idChainData.cdLightData;
  
  for (i = 0; i <((idData.idLightSensorCount * LIGHT_SENSOR_FIELDS) * sizeof(short)); ++i) {
    if (charPtr == 0) {
      printf("Not enough data to complete light data structure!  Continuing but data may be corrupt!!!\r\n");
      break;
    }

    hByte0 = convertCharToHex(charPtr);
    ++charPtr;
    hByte1 = convertCharToHex(charPtr);
    ++charPtr;
    *binPtr = (hByte0 << 4) | hByte1;
    ++binPtr;
  }
}

uint16_t convertCharToHex(char* ptr) {
  if ((*ptr >= '0') && (*ptr <= '9')) {
    return (*ptr - '0');
  } else if ((*ptr >= 'a') && (*ptr <= 'f')) {
    return ((*ptr - 'a') + 10);
  } else if ((*ptr >= 'A') && (*ptr <= 'F')) {
    return ((*ptr - 'A') + 10);
  }

  printf("Invalid hex char found at position %lx\r\n", (ptr - buff));
  exit(1);
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
