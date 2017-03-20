#include <Wire.h>

//#include "NewSoftwareSerial.h"
#include "pgmspace.h"

void setup(){
  Serial.begin(19200);
  
  OBD_setup();
}
void loop(){
   
  
  OBD_loop();
}
