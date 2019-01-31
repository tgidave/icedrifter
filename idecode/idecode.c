#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../icedrifter/icedrifter.h"

#define BUFFSIZE 512

char buff[BUFFSIZE];

icedrifterData idData;

void convertStringToStruct(char* charPtr, char* binPtr);
uint16_t convertCharToHex(char* ptr);

int main(int argc, char** argv) {
  char* ptr = buff;
  int i;
  char buffHold;
  int dataLen;
  struct tm *timeInfo;
  time_t tempTime;

  if (argc != 2)  {
    printf("Invalid number of arguments received!\r\n");
    exit(1);
  }

  if (strncmp(argv[1], "ID00", 4)) {

    if ((dataLen = strlen(argv[1])) != 88) {
      printf("\r\nData invalid lentgh ");
      printf("%d", dataLen);
      printf(" should be 88\r\n");
      exit(1);
    }

    convertStringToStruct(argv[1], (char*)&idData);

    printf("Record type ID00\r\n");
    tempTime = (time_t)idData.idLastBootTime;
    timeInfo = gmtime(&tempTime);
    printf("Last Boot:   %s", asctime(timeInfo));
    tempTime = (time_t)idData.idGPSTime;
    timeInfo = gmtime(&tempTime);
    printf("GPS time:    %s", asctime(timeInfo));
    printf("latitude:    %f\r\n", idData.idLatitude); 
    printf("longitude:   %f\r\n", idData.idLongitude);
    printf("altitude:    %f\r\n", idData.idAltitude);
    printf("speed:       %f\r\n", idData.idSpeed);
    printf("course:      %f\r\n", idData.idCourse);
    printf("temperature: %f C\r\n", idData.idTemperature);
    printf("pressure:    %f Pa\r\n", idData.idPressure);
    printf("remote temp: %f C\r\n", idData.idRemoteTemp);

  } else {
    printf("Invalid record type!!!\r\n");
  }
}

void convertStringToStruct(char* charPtr, char* binPtr) {
  char hByte0;
  char hByte1;
  int i = 0;
  int j = 0;

  while (*charPtr != 0) {
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

  printf("Invalid hex char found at position %ld\r\n", ((ptr - (char*)&buff) + 1));
  exit(1);
}

