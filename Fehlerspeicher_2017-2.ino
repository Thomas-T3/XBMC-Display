#include <Wire.h>

//#include "NewSoftwareSerial.h"
#include "pgmspace.h"

void setup(){
  Serial.begin(19200);
  Serial1.begin(9600);
   //Serial1.write("Hallo Serial1");
  OBD_setup();
}
void loop(){
   
  
  OBD_loop();
}
