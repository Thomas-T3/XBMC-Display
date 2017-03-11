/*
 * KW1281.h
 *
 *  Created on: 01.03.2017
 *      Author: Tom
 */
#include "global.h"
#include <iostream.h>
#include "pgmspace.h"
#include "Arduino.h"
#ifndef KW1281_H_
#define KW1281_H_

#define ADR_Engine 0x01
#define pinKLineRX 15
#define pinKLineTX 14

class speedo_Kw {
public:
	speedo_Kw();
		~speedo_Kw();


	void disconnect() ;
	void obdWrite(uint8_t data);
	uint8_t obdRead();
	uint16_t Errorcode;
	int Grund;
	void readFromProgmemArray(int pos,int pos2);
	void send5baud(uint8_t data) ;
	bool KWP5BaudInit(uint8_t addr);
	bool KWPSendBlock(char *s, int sizeb) ;
	bool KWPReceiveBlock(char s[], int maxsize, int &sizeb) ;
	bool KWPSendAckBlock();
	bool connect(uint8_t addr, int baudrate) ;
	bool readConnectBlocks();
	bool readError();
	bool readSensors(int group);
	void updateDisplay();
	void OBD_setup();
	void OBD_loop();
	int get_Error_Temp();
	int get_Pointer_for_Array();
	int get_Reason_Temp();
	int get_Reason_Pointer();
	char printBuffer[50];
private:
	};

extern speedo_Kw* Mkl;

#endif /* KW1281_H_ */

