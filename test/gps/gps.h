#ifndef _GPS_H
  #define _GPS_H

#define GPS_SERIAL Serial1
#define GPS_RX_PIN 9 //Pin marked TX on GPS board
#define GPS_TX_PIN 10 //Pin marked RX on GPS board
#define GPS_BAUD 9600
#define GPS_POWER_PIN 14

int getGPSFix(void);
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

#define OUTBUFFER_SIZE 128

#endif



