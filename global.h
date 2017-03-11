#ifndef GLOBAL_H_
#define GLOBAL_H_
#include "stdint.h"
#include "iostream.h"

typedef struct {
uint32_t longitude;
uint32_t latitude;
} simple_coordinate;
//int trip_meter;
// sensoren
//#include "clock_me.h"
//#include "dz.h"
//#include "gps.h"

#include "temperature.h"
#include "reset.h"
#include "ssd0323.h"
#include "debug.h"
#include "gps_TL.h"
#include "RTClib.h"

#ifdef __cplusplus
    #undef PROGMEM
    #define PROGMEM __attribute__(( section(".progmem.data") ))
    #undef P_STR
    #define P_STR(s) (__extension__({static prog_char __c[] PROGMEM = (s);&__c[0];}))
#else
    #define P_STR(s) P_STR(s)
#endif


/**********************************  working settings ********************************/
// development settings //
#define       GPS_SPEED_ONLY  false   // to ignore the magnetic value  -> hmm das ist doof, aber muss erstmal alleine arbeiten, mal sehen ob der gang angezeigt wird -> wenn ja => mag rennt
#define       BUTTONS_OFF     false   // disable all buttons
#define       WELCOME         true    // die frau am start
/**********************************  working settings ********************************/

#define BMP(a,b,c,d,e,f,g) (a*1000000L+b*100000L+c*10000L+d*1000+e*100+f*10+g)
//Useful macros for accessing single bytes of int and long variables
#define BYTE1(var) *((unsigned char *) &var+1)
#define BYTE0(var) *((unsigned char *) &var)

//#include <WProgram.h>
#include "wiring.h"
#include <avr/eeprom.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/delay.h> // timing
#include <stdlib.h>
#include "speedo.h"
#include "inc/wiring.h"
#include "inc/I2C.h"
#include "inc/HardwareSerial.h"
#include "Wprogram.h"
#include "pgmspace.h"
#include "inc/wiring.h"
#include "inc/mcp2515_defs.h"
#include <avr/interrupt.h>
#include "speed.h"
#include "dz.h"
#include "KW1281.h"

void setup();
void init_speedo();



#endif /* FKT_H_ */
