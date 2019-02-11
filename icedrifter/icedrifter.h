#ifndef _ICEDRIFTER_H
#define _ICEDRIFTER_H

#ifdef ARDUINO
  #include <TimeLib.h>
  #include <Time.h>
#endif

// The TEST_ALL switch will will collect and send data at bootup 
// and then every hour on the half hour after that.  Comment out 
// the next line to run normally.

//#define TEST_ALL  // test as much code a possible at bootup.

//To turn off the debugging messages, comment out the next line.

//#define SERIAL_DEBUG

//The following defines are used to control what data is transmitted during debugging.
//If "SERIAL_DEBUG" is not defined they have no effect.

#ifdef SERIAL_DEBUG
#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200

#define SERIAL_DEBUG_GPS
#define SERIAL_DEBUG_BMP280
#define SERIAL_DEBUG_DS18B20
#define SERIAL_DEBUG_ROCKBLOCK
#endif

//icedrifter data record definition.
typedef struct icedrifterData {
//  uint16_t idSeqNbr;
//  uint16_t idRecordType;
  char idRecordType[4];
#ifdef ARDUINO
  time_t idLastBootTime;
  time_t idGPSTime;
#else
  int idLastBootTime;
  int idGPSTime;
#endif
  float idLatitude;
  float idLongitude;
  float idAltitude;
  float idSpeed;
  float idCourse;
  float idTemperature;
  float idPressure;
  float idRemoteTemp;
} icedrifterData; 

#endif // _ICEDRIFTER_H
