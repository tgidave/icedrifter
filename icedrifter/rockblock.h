#ifndef _ROCKBLOCK_H
  #define _ROCKBLOCK_H

#include "icedrifter.h"

#define ROCKBLOCK_RX_PIN 12 // Pin marked RX on RockBlock
#define ROCKBLOCK_TX_PIN 13 // Pin marked TX on RockBlock
#define ROCKBLOCK_SLEEP_PIN 6
#define ROCKBLOCK_BAUD 19200
#define ROCKBLOCK_POWER_PIN 15

//#define SEND_2_RECORDS

//#ifdef SEND_2_RECORDS
//void transmitIcedrifterData(char *, int, char *, int);
//#else
void transmitIcedrifterData(icedrifterData *, int);
//#endif  //SEND_2_RECORDS

#endif  //_ROCKBLOCK_H
