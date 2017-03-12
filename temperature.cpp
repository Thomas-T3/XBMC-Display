/* Speedoino - This file is part of the firmware.
 * Copyright (C) 2011 Kolja Windeler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
//#define TEMP_DEBUG();
speedo_temperature::speedo_temperature(){
	max_values=0;
	air_temp_value_out=0;
	oil_temp_value_counter=0;
	temperatur_air=0;
	oil_temp_value=0;
	oil_temp_fail_status=0; // 0 = no failed read, 1-5 = Number of shorts read, 6-9 = Number of open read

	water_temp_value_counter=0;
	water_temp_value=0;
	water_temp_fail_status=0; // 0 = no failed read, 1-5 = Number of shorts read, 6-9 = Number of open read

	air_temp_value=0;

	water_warning_temp=0; // 105 C
	oil_warning_temp=0; // 120 C
	// values for
	for(unsigned int j=0; j<sizeof(oil_r_werte)/sizeof(oil_r_werte[0]); j++){
		oil_r_werte[j]=0;
		oil_t_werte[j]=0;

		water_r_werte[j]=0;
		water_t_werte[0]=0;
	};
};

speedo_temperature::~speedo_temperature(){
};

int speedo_temperature::check_vars(){
	if(water_r_werte[0]==0 || water_t_werte[0]==0 || oil_t_werte[0]==0 || oil_r_werte[0]==0){
		// Temperatur und Widerstands LookUp

		// OIL*************************************************

		int r_werte[19]={1000,800,600,500,400,350,330,310,290,270,240,235,230,225,220,215,210,205,201}; // widerstandswerte
		//int r_werte[19]={1200,800,600,400,300,250,230,210,180,120,100,75,55,45,40,35,25,20,15};
		int t_werte[19]={  20, 30, 40, 45, 50, 60, 70, 80, 85, 90, 95,100,110,120,125,130,135,140,150}; // passender Temperaturwert

		for(unsigned int j=0; j<sizeof(oil_r_werte)/sizeof(oil_r_werte[0]); j++){
			oil_r_werte[j]=r_werte[j];
			oil_t_werte[j]=t_werte[j];
		};

		// Water****************************************************
		//TODO (Werte aus Diagramm VW AGG Motor..)
		int r_werte2[19]={1600,800,500,450,400,375,350,335,300,260,250,240,230,225,221,217,214,210,206};
		//int r_werte2[19]={2700,1600,1200,1150,1030,920,820,720,620,570,490,440,420,395,368,318,268,235,225}; // widerstandswerte
		int t_werte2[19]={  20, 40, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95,100,105,110,115,120,125,130}; // passender Temperaturwert
		for(unsigned int j=0; j<sizeof(water_r_werte)/sizeof(water_r_werte[0]); j++){
			water_r_werte[j]=r_werte2[j];
			water_t_werte[j]=t_werte2[j];
		};

		water_warning_temp=1050; // 105 C
		oil_warning_temp=1250; // 125 C

		//Serial.println(PSTR("temp failed"));
		return 1;
	};

	return 0;
}

void speedo_temperature::init(){
	I2c.begin();
	I2c.setSpeed(0); /// fast
	I2c.timeOut(100); // 100 ms to get Temperature
	//Serial.println(PSTR("Temp init done."));

}

uint16_t speedo_temperature::pulldown_of_oil_divider(uint32_t mV_oil_supply, uint16_t mV_oil_center, uint16_t pull_up){
	uint32_t uV_oil_center=((uint32_t)1000)*((uint32_t)mV_oil_center);		// e.g. uV_center=1,980*1000=1980
	uint32_t pull_up_oil_mV=mV_oil_supply-mV_oil_center; 	// e.g. pull_up_mV=12000-2500=9500 TL5000-1980=3020?
	uint32_t current_oil_uA=(pull_up_oil_mV*1000)/pull_up;	// e.g. current_uA=9.500.000/3000=3166
	return (uint16_t)(uV_oil_center/current_oil_uA); 	// e.g.	2.500.000/3166=789 Ohm
}

void speedo_temperature::read_oil_temp() {

	uint16_t mV_oil_center=analogRead(OIL_TEMP_PIN);//*4.88; // *5000/1024
	//Serial.println(mV_oil_center);
	if(mV_oil_center>0 && mV_oil_center<1020){
		mV_oil_center=mV_oil_center * 4.88;//4.25;
		uint16_t r_temp=pulldown_of_oil_divider(5000,mV_oil_center,1000);
#ifdef TEMP_DEBUG
		// debug //
		Serial.print("analogRead: ");
		Serial.println(analogRead(OIL_TEMP_PIN));

		Serial.print("mV_oil_center: ");
		Serial.println(mV_oil_center);

		Serial.print("pulldown_of_divider: ");
		Serial.println(r_temp);
		// debug //
#endif

		if(r_temp<oil_r_werte[18]){
			oil_temp_value=flatIt(10*oil_t_werte[18],&oil_temp_value_counter,60,oil_temp_value);
		} else {
			for(int i=0;i<19;i++){ // 0 .. 15 müssen durchsucht werden das sind die LUT positionen
				if(r_temp>=oil_r_werte[i]){ // den einen richtigen raussuchen
					int j=i-1;  if(j<0) j=0; // j=12
					int offset=r_temp-oil_r_werte[i]; // wieviel höher ist mein messwert als den, den ich nicht wollte => 102R-100R => 2R
					int differ_r=oil_r_werte[j]-oil_r_werte[i]; // wie weit sind die realen widerstands werte auseinander 10R
					int differ_t=oil_t_werte[i]-oil_t_werte[j]; // wie weit sind die realen temp werte auseinander 5°C
					int aktueller_oil_wert=round(10*(oil_t_werte[i]-offset*differ_t/differ_r));
					oil_temp_value=flatIt(aktueller_oil_wert,&oil_temp_value_counter,60,oil_temp_value);
#ifdef TEMP_DEBUG

					Serial.print(" direkte messung ");
					Serial.print(aktueller_oil_wert);
					Serial.print(" und geplaettet: ");
					Serial.println(int(round(oil_temp_value)));
#endif
					oil_temp_fail_status=0;
					break; // break the for loop
				};
			};
		};

	}  else if(mV_oil_center==0) { // kein Sensor  0=(1024-x)/10        x>=1015
		if(oil_temp_fail_status<6){
			oil_temp_fail_status=6;
		} else if(oil_temp_fail_status<9){
			oil_temp_fail_status++;
		}
	} else { // Kurzschluss nach masse: 102=(1024-x)/10      x<=4
		if(oil_temp_fail_status<5){ // nach sechs maligem fehler => ausgabe!
			oil_temp_fail_status++;
		};
#ifdef TEMP_DEBUG
		//Serial.print("Oil Wert Kurzschluss ggn Masse");
#endif
	}
};

uint16_t speedo_temperature::pulldown_of_divider(uint32_t mV_supply, uint16_t mV_center, uint16_t pull_up){
	uint32_t uV_center=((uint32_t)1000)*((uint32_t)mV_center);		// e.g. uV_center=1980*1000=1.980.000
	uint32_t pull_up_mV=mV_supply-mV_center; 	// e.g. pull_up_mV=5000-3020=1980
	uint32_t current_uA=(pull_up_mV*1000)/pull_up;	// e.g. current_uA=1.980.000/5100=388
	return (uint16_t)(uV_center/current_uA); 	// e.g.	1.980.000/388=5103 Ohm
}

void speedo_temperature::read_water_temp() {

	// werte in array speichern in °C*10 für Nachkommastelle


#ifdef TEMP_DEBUG
	Serial.println("\n\rTemp: Beginne Water zu lesen");
#endif
	// werte auslesen
	uint16_t mV_center=analogRead(WATER_TEMP_PIN);//*4.88; // *5000/1024
#ifdef TEMP_DEBUG
	Serial.print("analogread zwischen 0 und 1024?:");
	Serial.println(mV_center);
#endif
	if(mV_center>0 && mV_center<1020){
		mV_center=mV_center*4.88;
		uint16_t r_temp=pulldown_of_divider(5000,mV_center,1000);

#ifdef TEMP_DEBUG
		// debug //
		Serial.print("water analog Read: ");
		Serial.println(analogRead(WATER_TEMP_PIN));

		Serial.print("water mV_center: ");
		Serial.println(mV_center);

		Serial.print("water pulldown_of_divider: ");
		Serial.println(r_temp);
		// debug //
#endif

		if(r_temp<water_r_werte[18]){
			water_temp_value=flatIt(10*water_t_werte[18],&water_temp_value_counter,60,water_temp_value);
		} else {
			for(int i=0;i<19;i++){ // 0 .. 18 müssen durchsucht werden das sind die LUT positionen
				if(r_temp>=water_r_werte[i]){ // den einen richtigen raussuchen
					int j=i-1;  if(j<0) j=0; // j=12
					int offset=r_temp-water_r_werte[i]; // wieviel höher ist mein messwert als den, den ich nicht wollte => 102R-100R => 2R
					int differ_r=water_r_werte[j]-water_r_werte[i]; // wie weit sind die realen widerstands werte auseinander 10R
					int differ_t=water_t_werte[i]-water_t_werte[j]; // wie weit sind die realen temp werte auseinander 5°C
					int aktueller_wert=round(10*(water_t_werte[i]-offset*differ_t/differ_r));
					water_temp_value=flatIt(aktueller_wert,&water_temp_value_counter,60,water_temp_value);
#ifdef TEMP_DEBUG

					Serial.print(" water direkte messung ");
					Serial.print(aktueller_wert);
					Serial.print(" water und geplaettet: ");
					Serial.println(int(round(water_temp_value)));
#endif
					water_temp_fail_status=0;
					break; // break the for loop
				};
			};
		};

	}  else if(mV_center==0) { // kein Sensor  0=(1024-x)/10        x>=1015
		if(water_temp_fail_status<6){
			water_temp_fail_status=6;
		} else if(water_temp_fail_status<9){
			water_temp_fail_status++;
		}
	} else { // Kurzschluss nach masse: 102=(1024-x)/10      x<=4
		if(water_temp_fail_status<5){ // nach sechs maligem fehler => ausgabe!
			water_temp_fail_status++;
		};
#ifdef TEMP_DEBUG
		Serial.print("Water Wert Kurzschluss gegen Masse");
#endif
	}
};
void speedo_temperature::read_air_temp() {
	// get i2c tmp102 //
#ifdef TEMP_DEBUG
	Serial.println("beginne air read IN");
#endif
	int sensorAddressIn = 0b01001001;

	byte msb;
	byte lsb;
	I2c.read(sensorAddressIn,0x00,2);
#ifdef TEMP_DEBUG
	Serial.println("request 1 abgeschickt");

#endif
	if (I2c.available() >= 2){  // if two bytes were received
#ifdef TEMP_DEBUG
		Serial.println("2 Byte im Puffer");
#endif
		msb = I2c.receive();  // receive high byte (full degrees)
		lsb = I2c.receive();  // receive low byte (fraction degrees)
		char pruefstring1[10];
		char pruefstring2[10];
		sprintf(pruefstring1,"%i",msb);
		sprintf(pruefstring2,"%i",lsb);
#ifdef TEMP_DEBUG
		Serial.println("Wert Temp Innen:");
		Serial.println(pruefstring1);
		Serial.println(pruefstring2);
		//25� = 0000 1100 1000 0000 LM73 11bit
		// 9� = 0000 0100 1000 0000
		//1�  = 0000 0000 1000 0000
		//-1� = 1111 1111 1000 0000
#endif
		air_temp_value = ((msb) <<1);  // MSB

#ifdef TEMP_DEBUG
		Serial.print("air1: ");
		Serial.println(air_temp_value);
#endif
		air_temp_value|= (lsb >> 5);    // LSB
#ifdef TEMP_DEBUG
		Serial.print("air2: ");
		Serial.println(air_temp_value);
#endif
		air_temp_value = round(air_temp_value); // round and save
#ifdef TEMP_DEBUG
		Serial.print("Air value IN: "); Serial.println(air_temp_value);
#endif

	} else {
		air_temp_value = 999;
#ifdef TEMP_DEBUG
		Serial.println("ALARM keine Antwort vom Innentemperatursensor!!");
#endif
	};

}
void speedo_temperature::read_air_temp_out() {
#ifdef TEMP_DEBUG
	Serial.println("beginne air read OUT");
#endif
	int sensorAddressOut = 0b01001000;
	byte msb;
	byte lsb;
	I2c.read(sensorAddressOut,0x00,2);
#ifdef TEMP_DEBUG
	Serial.println("request 2 abgeschickt");
#endif
	if (I2c.available() >= 2){  // if two bytes were received
#ifdef TEMP_DEBUG
			Serial.println("2 Byte im Puffer");
#endif
		msb = I2c.receive();  // receive high byte (full degrees)
		lsb = I2c.receive();  // receive low byte (fraction degrees)

		air_temp_value_out = ((msb) << 1);  // MSB
		air_temp_value_out|= (lsb >> 5);    // LSB
		air_temp_value_out = round(air_temp_value_out); // round and save
#ifdef TEMP_DEBUG
		Serial.print("Air value OUT: "); Serial.println(air_temp_value_out);
#endif

	} else {
		air_temp_value_out = 999;
#ifdef TEMP_DEBUG
		Serial.println("ALARM keine Antwort vom Aussentemperatursensor!!");
#endif
	};
};
int speedo_temperature::get_air_temp(){
	if(air_temp_value>999) // notfall
		return 0;
	else
		return air_temp_value;
}
int speedo_temperature::get_air_temp_out(){
	if(air_temp_value_out>999) // notfall
		return 0;
	else
		return air_temp_value_out;
}
int speedo_temperature::get_oil_temp(){
	return int(round(oil_temp_value/10));
}

int speedo_temperature::get_water_temp(){
	return int(round(water_temp_value/10));
}

float speedo_temperature::flatIt(int actual, unsigned char *counter, char max_counter, float old_flat){
	if(*counter==max_counter){
		return (float)((old_flat*(max_counter-1)+actual)/(max_counter));
	} else if(*counter<max_counter && *counter>=0){
		*counter=*counter+1;
		return (float)((old_flat*(*counter-1)+actual)/(*counter));
	} else {
		*counter=1;
		return actual;
	}
	// hier besteht die gefahr das ein messwert nur [INTmax]/max_counter groß sein darf
	// bei 20 Werten also nur 3276,8
}

