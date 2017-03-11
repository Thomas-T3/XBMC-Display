/*
 * KW1281.cpp
 *
 *  Created on: 01.03.2017
 *      Author: Tom
 */
/*
Arduino Nano OBD reader (OBD protocol KW1281,  Audi A4 B5 etc.)
adapted for VW T3 with AGG Engine with Simos ECU
Read the error list and several sensordata.

wiring:
Mega 2560: 15 (Serial3) --- OBD level shifter input (RX) (e.g. LM339)
Mega 2560: 14 (Serial3) --- OBD level shifter output (TX) (e.g. LM339)

NOTE: For the level shifting, I used a 'AutoDia K409 Profi USB adapter', disassembled it,
      and connected the Arduino to the level shifter chip (LM339) - the original FTDI chip TX line
      was removed (so it does not influence the communication)
*/
#include "global.h"
//#include "KW1281.h"
speedo_Kw::speedo_Kw(){
}
		bool status = 1;
			uint8_t currAddr = 0;
			uint8_t blockCounter = 0;
			uint8_t errorTimeout = 0;
			uint8_t errorData = 0;
			bool connected = false;
			int sensorCounter = 0;
			int pageUpdateCounter = 0;
			uint8_t currPage = 2;
			int8_t coolantTemp = 0;         // K�hlwassertemperatur
			float Lambda=0;                 // Lambdasondenspannung
			int8_t intakeAirTemp = 0;       // Ansauglufttemperatur
			float engineLoad = 0;           // Lastzustand
			int   engineSpeed = 0;          // Drehzahl
			float throttleValve = 0;        // Drosselklappenstellung
			float supplyVoltage = 0;        // Batteriespannung
			uint8_t vehicleSpeed = 0;       // Geschwindigkeit
			uint8_t fuelConsumption = 0;    // Verbrauch
			 int e; //Pointer fuer Fehlertext
			 int f; // Pointer fuer Fehlergrund

	String floatToString(float v){
		String res;
		char buf[16];
		dtostrf(v,4, 2, buf);
		res=String(buf);
		return res;
	}

  const char E282[] PROGMEM = " Drosselklappensteller";
  const char E513[] PROGMEM = " OT-Geber";
  const char E515[] PROGMEM = " Hallgeber";
  const char E516[] PROGMEM = " Leerlaufschalter";
  const char E518[] PROGMEM = " Drosselklappenpoti";
  const char E520[] PROGMEM = " Luftmassenmesser";
  const char E522[] PROGMEM = " Geber Kuehlmitteltemperatur";
  const char E523[] PROGMEM = " Geber Ansauglufttemperatur";
  const char E524[] PROGMEM = " Klopfsensor";
  const char E525[] PROGMEM = " Lambdasonde";
  const char E530[] PROGMEM = " Drosselklappensteller/-poti";
  const char E532[] PROGMEM = " Batteriespannung";
  const char E533[] PROGMEM = " Leerlaufregelung";
  const char E537[] PROGMEM = " Lambdaregelung";
  const char E546[] PROGMEM = " Datenleitung defekt";
  const char E625[] PROGMEM = " Geschwindigkeitssignal";
  const char E635[] PROGMEM = " Heizung Lambdasonde";
  const char E1247[] PROGMEM = " Ventil Aktivkohlebeh�lter";
  const char E1249[] PROGMEM = " Einspritzventil 1.Zyl";
  const char E1250[] PROGMEM = " Einspritzventil 2.Zyl";
  const char E1251[] PROGMEM = " Einspritzventil 3.Zyl";
  const char E1252[] PROGMEM = " Einspritzventil 4.Zyl";
  const char E1259[] PROGMEM = " Kraftstoffpumpenrelais";
  const char E17978[] PROGMEM = " Motorsteuergeraet gesperrt";
  const char E65335[] PROGMEM = " Steuergeraet defekt";
  const char Fehler[] PROGMEM = " unbekannter Fehler";
  const char C01[] PROGMEM = " Kurzschlu� nach Plus";
  const char C02[] PROGMEM =" Kurzschlu� nach Masse";
  const char C03[] PROGMEM =" kein Signal";
  const char C04[] PROGMEM =" mechanischer Fehler";
  const char C06[] PROGMEM =" Signal zu gro�";
  const char C07[] PROGMEM =" Signal zu klein";
  const char C08[] PROGMEM =" Regelgrenze ueberschritten";
  const char C16[] PROGMEM =" Signal au�erhalb der Toleranz";
  const char C17[] PROGMEM =" Regeldifferenz";
  const char C18[] PROGMEM =" oberer Anschlagwert";
  const char C19[] PROGMEM =" unterer Anschlagwert";
  const char C27[] PROGMEM =" unplausibles Signal";
  const char C28[] PROGMEM =" Kurzschluss nach Plus";
  const char C29[] PROGMEM =" Kurzschluss nach Masse";
  const char C30[] PROGMEM =" Unterbrechung / Kurzschluss nach Plus";
  const char C31[] PROGMEM =" Unterbrechung / Kurzschluss nach Masse";
  const char C36[] PROGMEM =" Unterbrechung";
  const char C37[] PROGMEM =" defekt";
  const char C44[] PROGMEM =" Kurzschluss";
  const char C45[] PROGMEM =" Steckverbindung";
  const char CFehler[] PROGMEM = " unbekannter Grund";
  const char* const ECListe[] PROGMEM = {E282,E513,E515,E516,E518,E520,E522,E523,E524,E525,E530,E532,E533,E537,E546,E625,E635,E1247,E1249,E1250,E1251,E1252,E1259,E17978,E65335, Fehler};
  const char* const CListe[] PROGMEM ={C01,C02,C03,C04,C06,C07,C08,C16,C17,C18,C19,C27,C28,C29,C30,C31,C36,C37,C44,C45,CFehler};


void speedo_Kw::disconnect(){
    connected = false;
    currAddr = 0;
  }
  void speedo_Kw::obdWrite(uint8_t data){
  	  #ifdef DEBUG
	  	  Serial.print("uC:");
	  	  Serial.println(data, HEX);
  	  #endif
   //*********************  ADDED 4ms delay ****************************************************************
	  delay(4);
    Serial.write(data);                // sendChar() 28.04.2016 20:35
    delayMicroseconds(1300);            // Uart byte Tx duration at 9600 baud ~= 1040 us
    uint8_t dummy = Serial3.read();     // Read Echo from the line
    if (dummy != data ) {
        return;   // compare received and transmitted char
    }
    Serial.print(data);
  };

  void speedo_Kw::readFromProgmemArray(int pos,int pos2){
	  char printBuffer[50];
	  char printBuffer2[60];
	  strcpy_P(printBuffer, (char*)pgm_read_word(&(ECListe[pos])));
	  sprintf(printBuffer,"%s",ECListe[pos]);
	  Serial.print("Hier steht der Fehlertext: "); Serial.println(printBuffer);
	  strcpy_P(printBuffer2, (char*)pgm_read_word(&(CListe[pos2])));
	  sprintf(printBuffer2,"c%s",CListe[pos2]);
	  Serial.print("Hier steht der Fehlergrund: "); Serial.println(printBuffer2);
  }

  uint8_t speedo_Kw::obdRead(){
	  unsigned long timeout = millis() + 1000;
	  while (!Serial3.available()) {
		  if (millis() >= timeout) {
			  disconnect();
			  errorTimeout++;
			  return 0;
		  }
	    }
	    uint8_t data = Serial3.read();
	    while (data == 0xff) {
	    	data = Serial3.read();
	    }
	    return data;
	  }

  void speedo_Kw::send5baud(uint8_t data) {
    // // 1 start bit, 7 data bits, 1 parity, 1 stop bit
  #define bitcount 10
  byte bits[bitcount];
  byte even = 1;
  byte bit;
  pinMode(pinKLineTX, OUTPUT);
  digitalWrite(pinKLineTX, LOW);
  delay(500);
  for (int i = 0; i < bitcount; i++) {
      bit = 0;
      if (i == 0)  bit = 0;
      else if (i == 8) bit = even; // computes parity bit
      else if (i == 9) bit = 1;
      else {
        bit = (byte) ((data & (1 << (i - 1))) != 0);
        even = even ^ bit;
      }
      bits[i] = bit;
    }
    // now send bit stream
    for (int i = 0; i < bitcount + 1; i++) {
      if (i != 0) {
        // wait 200 ms (=5 baud), adjusted by latency correction
    	  delay(200);
    	  if (i == bitcount) break;
      }
      if (bits[i] == 1) {
        // high
    	  digitalWrite(pinKLineTX, LOW);
      } else {
        // low
    	  digitalWrite(pinKLineTX, HIGH);
      }
    }

    digitalWrite(pinKLineTX, LOW);
  };


  bool speedo_Kw::KWP5BaudInit(uint8_t addr){
    Serial.println("---KWP 5 baud init");
    //delay(3000);
    send5baud(addr);
    return true;
  }
  bool speedo_Kw::KWPSendBlock(char *s, int size){
    Serial.print("---KWPSend sz=");
    Serial.print(size);
    Serial.print(" blockCounter=");
    Serial.println(blockCounter);
    // show data
    Serial.print("OUT:");
    for (int i=0; i < size; i++){
      uint8_t data = s[i];
      Serial.print(data, HEX);
      Serial.print(" ");
    }
    Serial.println();
    for (int i=0; i < size; i++){
      uint8_t data = s[i];
      Serial.print(data);

      if (i < size-1){
    	  uint8_t complement = obdRead();
        if (complement != (data ^ 0xFF)){
          Serial.println("ERROR: invalid complement");
          disconnect();
          errorData++;
          return false;
        }
      }
    }
    blockCounter++;
    return true;
  }

  // count: if zero given, first received byte contains block length
  // 4800, 9600 oder 10400 Baud, 8N1
  bool speedo_Kw::KWPReceiveBlock(char s[], int maxsize, int &size){
    bool ackeachbyte = false;
    uint8_t data = 0;
    int recvcount = 0;
    if (size == 0) ackeachbyte = true;
    Serial.print("---KWPReceive sz=");
    Serial.print(size);
    Serial.print(" blockCounter=");
    Serial.println(blockCounter);
    if (size > maxsize) {
      Serial.println("ERROR: invalid maxsize");
      return false;
    }
    unsigned long timeout = millis() + 1000;
    while ((recvcount == 0) || (recvcount != size)) {
      while (Serial3.available()){
        data = obdRead();
        s[recvcount] = data;
        recvcount++;
        if ((size == 0) && (recvcount == 1)) {
          size = data + 1;
          if (size > maxsize) {
            Serial.println("ERROR: invalid maxsize");
            return false;
          }
        }
        if ((ackeachbyte) && (recvcount == 2)) {
          if (data != blockCounter){
            Serial.println("ERROR: invalid blockCounter");
            disconnect();
            errorData++;
            return false;
          }
        }
        if ( ((!ackeachbyte) && (recvcount == size)) ||  ((ackeachbyte) && (recvcount < size)) ){
          obdWrite(data ^ 0xFF);  // send complement ack
        }
        timeout = millis() + 1000;
      }
      if (millis() >= timeout){
        Serial.println("ERROR: timeout");
        disconnect();
        errorTimeout++;
        return false;
      }
    }
    // show data
    Serial.print("IN: sz=");
    Serial.print(size);
    Serial.print(" data=");
    for (int i=0; i < size; i++){
      uint8_t data = s[i];
      Serial.print(data, HEX);
      Serial.print(" ");
    }
    Serial.println();
    blockCounter++;
    return true;
  }


  bool speedo_Kw::KWPSendAckBlock(){
    Serial.print("---KWPSendAckBlock blockCounter=");
    Serial.println(blockCounter);
    char buf[32];
    sprintf(buf, "\x03%c\x09\x03", blockCounter);
    return (KWPSendBlock(buf, 4));
  }

  bool speedo_Kw::connect(uint8_t addr, int baudrate){
	    blockCounter = 0;
	    currAddr = 0;
	    KWP5BaudInit(addr);
	    while (Serial3.available() > 0){
	            Serial3.read();
	          }
	   //PIOD->PIO_PDR = 0x10;
	  //  Serial.begin(baudrate);
	  //  while (!Serial) ;
	    // answer: 0x55, 0x01, 0x8A
	    char s[3];
	    int sizeb = 3;
	    if (!KWPReceiveBlock(s, 3, sizeb)) {

	      return false;
	    }
	    if (    (((uint8_t)s[0]) != 0x55)
	            ||   (((uint8_t)s[1]) != 0x01)
	            ||   (((uint8_t)s[2]) != 0x8A)   ) {
	      disconnect();
	      errorData++;
	      return false;
	  }
	    currAddr = addr;
	      connected = true;
	      if (!readConnectBlocks()) return false;
	      return true;
    Serial.print("------connect addr=");
    Serial.print(addr);
    Serial.print(" baud=");
    Serial.println(baudrate);

    blockCounter = 0;
    currAddr = 0;
    Serial.begin(baudrate);
    KWP5BaudInit(addr);
    // answer: 0x55, 0x01, 0x8A

    int size = 3;
    if (!KWPReceiveBlock(s, 3, size)) return false;
    if (    (((uint8_t)s[0]) != 0x55)
       ||   (((uint8_t)s[1]) != 0x01)
       ||   (((uint8_t)s[2]) != 0x8A)   ){
      Serial.println("ERROR: invalid magic");
      disconnect();
      errorData++;
      return false;
    }
    currAddr = addr;
    connected = true;
    if (!readConnectBlocks()) return false;
    return true;
  }

  bool speedo_Kw::readConnectBlocks(){
    // read connect blocks
    Serial.println("------readconnectblocks");
    String info;
    while (true){
      int size = 0;
      char s[64];
      if (!(KWPReceiveBlock(s, 64, size))) return false;
      if (size == 0) return false;
      if (s[2] == '\x09') break;
      if (s[2] != '\xF6') {
        Serial.println("ERROR: unexpected answer");
        disconnect();
        errorData++;
        return false;
      }
      String text = String(s);
      info += text.substring(3, size-2);
      if (!KWPSendAckBlock()) return false;
    }

    Serial.print("label=");
    sprintf(printBuffer,"%i",info);
    Serial.print(printBuffer);

    //lcd.setCursor(0, 1);
    //lcd.print(info);
    return true;
  }

  //*************************************Fehlerspeicher**************************************
  bool speedo_Kw::readError (){
	  int errorlist=0;
	  char s[64];
	  bool MKL=false;
	  if (currAddr != ADR_Engine) {
		  connect(ADR_Engine, 9600);
       } else {
    	   Serial.println("+++++++++++++++Fehlerspeicher+++++++++++++++++++++");
       };
	  sprintf(s,"\x03%c\x07\x03",blockCounter, errorlist);
	  if (!KWPSendBlock(s, 4)) return false;
	  int size = 0;
	  KWPReceiveBlock(s, 64, size);
	  int counter = (size-4) / 3;           // Anzahl der gespeicherten Fehler
   // Wenn kein Fehler gespeichert ist, dann counter auf 0 setzen
	  if (s[5]=='\x88'){ //Das Steuerger�t sendet, wenn kein Fehler gespeichert ist an 6. Stelle ein x88
		  Serial.println ("Kein Fehler gespeichert");
		  counter=0;
		  MKL=false;
	  }
  #ifdef DEBUG
    Serial.print("counter=");
    Serial.println(counter);
  #endif
  /* Fehlerbytes auslesen:
   *  Beispielstring: 12 B FC 2 A 1E 2 6 1D 2 12 1F 46 3A 23 46 3A 23 3 (>>HEX Daten!!<<)
   *  12 = L�nge des strings in HEX (12=0102=19!!)
   *  B = Blockcounter
   *  FC = Info, dass jetzt Fehler kommen
   *  2 / 6 Fehlercodebytes = 02 + 06.  1D ist der Fehlergrund
   *  Beim SIMOS Steuerger�t stehen die HEX Daten f�r den 1.Fehlercode an Stelle 4 und 5
   *  An Stelle 6 kommt der Fehlergrund
   *  An Stelle 7 und 8 kommt der 2. Fehler, an Stelle 9 der 2. Fehlergrund
   *  An Stelle 10 und 11 kommt der 3. Fehler, an Stelle 12 der 3. Fehlergrund
   *  An Stelle 13 und 14 kommt der 4. Fehler, an Stelle 15 der 4. Fehlergrund
   *  An Stelle 16 und 17 kommt der 5. Fehler, an Stelle 18 der 5. Fehlergrund
   *  Am Ende steht immer eine 3.
  */
  for (int idx=0; idx < counter; idx++){
    byte f1=s[3*idx+3];//[3 + idx*3];
    byte f2=s[3*idx+4];//[3 + idx*3+1];
    byte g=s[3*idx+5];//[3 + idx*3+2];
    Errorcode= ((256*f1)+f2);          //Umrechnung der Fehlercodes in Dezimalzahlen
    Grund=g;
    Serial.print("Errorcode: ");
    Serial.print(Errorcode);
    Serial.print("; Reason: ");
    Serial.println(Grund);
    switch (Errorcode){
        case 282: e=0; break;
        case 513: e=1; break;
        case 515: e=2; break;
        case 516: e=3; break;
        case 518: e=4; break;
        case 520: e=5; break;
        case 522: e=6; break;
        case 523: e=7; break;
        case 524: e=8; break;
        case 525: e=9; break;
        case 530: e=10; break;
        case 532: e=11; break;
        case 533: e=12; break;
        case 537: e=13; break;
        case 546: e=14; break;
        case 625: e=15; break;
        case 635: e=16; break;
        case 1247: e=17; break;
        case 1249: e=18; break;
        case 1250: e=19; break;
        case 1251: e=20; break;
        case 1252: e=21; break;
        case 1259: e=22; break;
        case 17978: e=23; break;
        case 65335: e=24; break;
        default: e=25; break;
        return e;
    }
    switch (Grund){
    	case 1: f=0; break;
    	case 2: f=1; break;
    	case 3: f=2; break;
    	case 4: f=3; break;
    	case 6: f=4; break;
    	case 7: f=5; break;
    	case 8: f=6; break;
    	case 16: f=7; break;
    	case 17: f=8; break;
    	case 18: f=9; break;
    	case 19: f=10; break;
    	case 27: f=11; break;
    	case 28: f=12; break;
    	case 29: f=13; break;
    	case 30: f=14; break;
    	case 31: f=15; break;
    	case 36: f=16; break;
    	case 37: f=17; break;
    	case 44: f=18; break;
    	case 45: f=19; break;
    	default: f=20; break;
    	return f;
    }

  #ifdef DEBUG
      Serial.print("Fehlerbyte1: ");
      Serial.print(f1);
      Serial.print("; Fehlerbyte2: ");
      Serial.print(f2);
      Serial.print("; Grund: ");
      Serial.println('C'&g);
  #endif

  //******************************************Neu********************
      readFromProgmemArray(e,f);
      //readFromProgmemArray(e,f);

  //**********************************Ende neu*********************
    }
     if (size>28) {
    //  KWPSendAckBlock;
    //  KWPReceiveBlock(s, 64, size);
    }
    if (s[6] != '\x88') {
    	MKL=true;
      Serial.println("Fehler gefunden:");
      //return false;
    }
     // show data
     if (s[5]=='\x88'){
      Serial.println ("Kein Fehler gespeichert");
      MKL=false;
     }
     //else{
     // if (!KWPSendAckBlock()) return false;
    // Serial.println();
    //blockCounter++;
    //return true;
    // }
   //int count = (size-4) / 3;
   // Serial.print(F("count="));
   // Serial.println(count);
   return (KWPSendBlock(s, 4));
  }

  bool speedo_Kw::readSensors(int group){
   Serial.print("------readSensors ");
   Serial.println(group);
    char s[64];
    sprintf(s, "\x04%c\x29%c\x03", blockCounter, group);
    if (!KWPSendBlock(s, 5)) return false;
    int size = 0;
    KWPReceiveBlock(s, 64, size);
    if (s[2] != '\xe7') {
      Serial.println("ERROR: invalid answer");
      disconnect();
      errorData++;
      return false;
    }
    int count = (size-4) / 3;
    //Serial.print(F("count="));
    //Serial.println(count);
    for (int idx=0; idx < count; idx++){
      byte k=s[3 + idx*3];
      byte a=s[3 + idx*3+1];
      byte b=s[3 + idx*3+2];
      String n;
      float v = 0;
      Serial.print("type=");
      Serial.print(k);
      Serial.print("  a=");
      Serial.print(a);
      Serial.print("  b=");
      Serial.print(b);
      Serial.print("  text=");

      String t = "";
      String units = "";
      char buf[32];
      switch (k){
        case 1:  v=0.2*a*b;             units=("rpm"); break;
        case 2:  v=a*0.002*b;           units=("%%"); break;
        case 3:  v=0.002*a*b;           units=("Deg"); break;
        case 4:  v=abs(b-127)*0.01*a;   units=("ATDC"); break;
        case 5:  v=a*(b-100)*0.1;       units=("�C");break;
        case 6:  v=0.001*a*b;           units=("V");break;
        case 7:  v=0.01*a*b;            units=("km/h");break;
        case 8:  v=0.1*a*b;             units=(" ");break;
        case 9:  v=(b-127)*0.02*a;      units=("Deg");break;
        case 10: if (b == 0) t=("COLD"); else t=("WARM");break;
        case 11: v=0.0001*a*(b-128)+1;  units = (" ");break;
        case 12: v=0.001*a*b;           units =("Ohm");break;
        case 13: v=(b-127)*0.001*a;     units =("mm");break;
        case 14: v=0.005*a*b;           units=("bar");break;
        case 15: v=0.01*a*b;            units=("ms");break;
        case 18: v=0.04*a*b;            units=("mbar");break;
        case 19: v=a*b*0.01;            units=("l");break;
        case 20: v=a*(b-128)/128;       units=("%%");break;
        case 21: v=0.001*a*b;           units=("V");break;
        case 22: v=0.001*a*b;           units=("ms");break;
        case 23: v=b/256*a;             units=("%%");break;
        case 24: v=0.001*a*b;           units=("A");break;
        case 25: v=(b*1.421)+(a/182);   units=("g/s");break;
        case 26: v=float(b-a);          units=("C");break;
        case 27: v=abs(b-128)*0.01*a;   units=("�");break;
        case 28: v=float(b-a);          units=(" ");break;
        case 30: v=b/12*a;              units=("Deg k/w");break;
        case 31: v=b/2560*a;            units=("�C");break;
        case 33: v=100*b/a;             units=("%%");break;
        case 34: v=(b-128)*0.01*a;      units=("kW");break;
        case 35: v=0.01*a*b;            units=("l/h");break;
        case 36: v=((unsigned long)a)*2560+((unsigned long)b)*10;  units=("km");break;
        case 37: v=b; break; // oil pressure ?!
        // ADP: FIXME!
        /*case 37: switch(b){
               case 0: sprintf(buf, F("ADP OK (%d,%d)"), a,b); t=String(buf); break;
               case 1: sprintf(buf, F("ADP RUN (%d,%d)"), a,b); t=String(buf); break;
               case 0x10: sprintf(buf, F("ADP ERR (%d,%d)"), a,b); t=String(buf); break;
               default: sprintf(buf, F("ADP (%d,%d)"), a,b); t=String(buf); break;
            }*/
        case 38: v=(b-128)*0.001*a;        units=("Deg k/w"); break;
        case 39: v=b/256*a;                units=("mg/h"); break;
        case 40: v=b*0.1+(25.5*a)-400;     units=("A"); break;
        case 41: v=b+a*255;                units=("Ah"); break;
        case 42: v=b*0.1+(25.5*a)-400;     units=("Kw"); break;
        case 43: v=b*0.1+(25.5*a);         units=("V"); break;
        case 44: sprintf(buf, "%2d:%2d", a,b); t=String(buf); break;
        case 45: v=0.1*a*b/100;            units=(" "); break;
        case 46: v=(a*b-3200)*0.0027;      units=("Deg k/w"); break;
        case 47: v=(b-128)*a;              units=("ms"); break;
        case 48: v=b+a*255;                units=(" "); break;
        case 49: v=(b/4)*a*0.1;            units=("mg/h"); break;
        case 50: v=(b-128)/(0.01*a);       units=("mbar"); break;
        case 51: v=((b-128)/255)*a;        units=("mg/h"); break;
        case 52: v=b*0.02*a-a;             units=("Nm"); break;
        case 53: v=(b-128)*1.4222+0.006*a;  units=("g/s"); break;
        case 54: v=a*256+b;                units=("count"); break;
        case 55: v=a*b/200;                units=("s"); break;
        case 56: v=a*256+b;                units=("WSC"); break;
        case 57: v=a*256+b+65536;          units=("WSC"); break;
        case 59: v=(a*256+b)/32768;        units=("g/s"); break;
        case 60: v=(a*256+b)*0.01;         units=("sec"); break;
        case 62: v=0.256*a*b;              units=("S"); break;
        case 64: v=float(a+b);             units=("Ohm"); break;
        case 65: v=0.01*a*(b-127);         units=("mm"); break;
        case 66: v=(a*b)/511.12;          units=("V"); break;
        case 67: v=(640*a)+b*2.5;         units=("Deg"); break;
        case 68: v=(256*a+b)/7.365;       units=("deg/s");break;
        case 69: v=(256*a +b)*0.3254;     units=("Bar");break;
        case 70: v=(256*a +b)*0.192;      units=("m/s^2");break;
        default: sprintf(buf, "%2x, %2x      ", a, b); break;

      }

      switch (currAddr){
        case ADR_Engine:
          switch(group){
           case 1:
              switch (idx){
                case 2: coolantTemp =v; break;
                case 3: Lambda=v; break;
                break;
              }
            break;
           case 2:
              switch (idx){
                case 3: supplyVoltage=v; break;
                case 4: intakeAirTemp=v;break;
                break;
              }
            break;
          case 5:
              switch(idx){
                case 3: fuelConsumption=v;break;
                break;
              }
            break;
          }
          break;
   }
    if (units.length() != 0){
        dtostrf(v,4, 2, buf);
        t=String(buf) + " " + units;
        char printBuffer4[10];
            sprintf(printBuffer4,"%i",t);
            Serial.print(printBuffer4);
    }
    }
    sensorCounter++;
    return true;
  }



  void speedo_Kw::updateDisplay(){
    if (!connected){
      if ( (errorTimeout != 0) || (errorData != 0) ){
        //lcd.setCursor(0,3);
        //lcd.print("err to=");
        //lcd.setCursor(7,3);
        //lcd.print( String(errorTimeout));
        //lcd.setCursor(10,3);
        //lcd.print(" da=");
        //lcd.setCursor(14,3);
        //lcd.print( String(errorData));
      }
    } else {
      switch (currPage){
        case 2:
          Serial.print("Wassertemperatur: ");
          Serial.println(coolantTemp,3);
          Serial.print("Lambdaspannung: ");
          Serial.println(Lambda);
          Serial.print("Bordspannung: ");
          Serial.println(supplyVoltage);
          Serial.print("Ansauglufttemperatur: ");
          Serial.println(intakeAirTemp);
          Serial.print("Verbrauch: ");
          Serial.println(fuelConsumption);
          break;
      }
    }
    pageUpdateCounter++;
  }
  char check[64];
  void speedo_Kw::OBD_setup(){

       Serial3.begin(19200);


    pinMode(pinKLineTX, OUTPUT);
    digitalWrite(pinKLineTX, HIGH);
    Serial.println("START");
    }


  void speedo_Kw::OBD_loop(){
    currPage=2;
    switch (currPage){
      case 2:
          if (currAddr != ADR_Engine) {
          connect(ADR_Engine, 9600);
        } else {
          Serial.println("+++++++++++++++Fehlerspeicher+++++++++++++++++++++");
          	  readError();

          if (currAddr = 0xFF){
          break;
          }
          	  readSensors(1);
          	  readSensors(2);
          	  readSensors(5);
        }
        break;
      case 3:
      if (currAddr = 0x00){
        break;
      }
      }

    updateDisplay();
     }
    int speedo_Kw::get_Error_Temp(){
  	return(Errorcode);
  }
    int speedo_Kw::get_Pointer_for_Array(){
    	return(e);
    }
    int speedo_Kw::get_Reason_Temp(){
    	return (Grund);
    }
    int speedo_Kw::get_Reason_Pointer(){
    	return (f);
    }
