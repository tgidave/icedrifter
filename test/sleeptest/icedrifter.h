#ifndef ICEDRIFTER_H
#define ICEDRIFTER_H

//To turn off the debugging messages, comment out the next line.

#define SERIAL_DEBUG

//The following defines are used to control what data is transmitted during debugging.
//If "SERIAL_DEBUG" is not defined they have no effect.

#ifdef SERIAL_DEBUG
#define SERIAL_DEBUG_TIME
#define SERIAL_DEBUG_GPS
#define SERIAL_DEBUG_TEMP
#define SERIAL_DEBUG_IMU
#define SERIAL_DEBUG_ROCKBLOCK

#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200
#endif

#endif // ICEDRIFTER_H


