#ifndef _GPS_H
  #define _GPS_H

#define OUTBUFFER_SIZE  340

#define GPS_SERIAL Serial1
//#define GPS_RX_PIN 9 //Pin marked TX on GPS board
//#define GPS_TX_PIN 10 //Pin marked RX on GPS board
#define GPS_POWER_PIN 14
#define GPS_BAUD 9600


typedef enum ft {
  FIX_FULL,
  FIX_TIME
} fixType;

int getGPSFix(fixType typeFix);
int gpsGetYear(void);  
int gpsGetMonth(void);
int gpsGetDay(void);   
int gpsGetHour(void);  
int gpsGetMinute(void);
int gpsGetSecond(void);
float gpsGetLatitude(void);
float gpsGetLongitude(void);
float gpsGetAltitude(void);
float gpsGetSpeed(void);
float gpsGetCourse(void);

#endif



