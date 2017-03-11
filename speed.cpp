/* SmartSpeedo - This file is part of the firmware.
 * Copyright (C) 2014 Kolja Windeler
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

/* estimate number of pulses
 * given that there are x pulses per wheel rotation (ca 2m), at high speed (250km/h = 70m/s) we'll have
 * 70[m/s]/2[m/rotation]*x[pulse/rotation]=35*x[pulse/s]
 *
 * :: 250km/h ::
 * for a reed: 35 Hz
 * 8 pulses (T3): 280 Hz
 * 64 Pulses (Hornet): 2240 Hz (<- 8 Bit timer + overflow char?)
 *
 * :: 10 km/h ::
 * Reed: 1.38 Hz
 * 8 Pulses: 11.11 Hz
 * 64 Pulses: 89 Hz
 *
 * Possible ways to count:
 *
 * 1. On every Peak well jump in the interrupt and substact the time between two pulses ..
 * Pro: Easy to calclate, slow speed is very precise
 * Contra: very un-precise if time between pulses is short (<100ms), lots of interrupts!
 *
 * 2. Let a time run, grab timer value on interrupt
 * Pro: done in RPM calculation, working stable
 * Contra: lots of interrupts!
 *
 * 3. count peaks with a timer, ask on a fixed time base for values
 * Pro: very well suited for high pulse speed
 * contra: might be slow?
 * -> Read every 500ms + store millis of last read + only read if at least two pulses captured
 */
//*********************************************************
/* 					Bus TL
 * ********************************************************
 * Ermittlung der Geschwindgkeit im Bus (T3)
 * Verwendet wird ein speedpuls Sensor SP-T3-8P mit 8 Impulsen / Umdrehung
 * Radumfang bei 185/80R14 = 1,97m
 * Also ist 1km/h = 1000m x 8 Impulse / 1,97m/Impuls =  4060 Impule /h = 67 Impulse / min = 1,128 Impulse / sek
 * Berechnung ist also:
 * Anzahl Impulse / Sekunde /1,128 = Geschwindigkeit
 * Kontrolle: 120km/h=((120000*8/(1,97*1,128)/60/60)=120   passt!
 * ********************************************************

 */

speedo_speed::speedo_speed(){
	last_time_read=millis();
	reed_speed=0;
	last_calc_pulse_ts=0;
	last_pulse_ts=0;

};

speedo_speed::~speedo_speed(){
};

void speedo_speed::init (){
	// timer
	TCNT5=0x00; // reset value
	TIMSK5=(1<<TOIE5); // <- activate Timer overflow interrupt
	TCCR5B=(1<<CS52) | (1<<CS51) | (1<<CS50); // selects external clock on rising edge
	TCCR5A=0; // WGM=0 -> normal mode

	EIMSK |= (1<<INT5); // Enable Interrupt
	EICRB |= (1<<ISC50) ; // rising edge on INT5
	EICRB |= (1<<ISC51) ; // rising edge on INT5

	last_time_read=millis(); //prevent calculation of rpm

	//pDebug->sprintlnp(PSTR("Speed init done"));
};

void speedo_speed::shutdown(){
	TCCR5B=(0<<CS52) | (0<<CS51) | (0<<CS50); // off
}


ISR(TIMER3_OVF_vect){
	Serial.println("-----------------überlauf!--------------------");
}

/* output: frequency of reed input, >> SCALED BY 8.192 << !
 * to get the km/h with
 * e.g. one pulse per 2.5m, you have to get_mag_speed()/8.192*2.5m*3.6[km/h]/[m/s]
 * => get_mag_speed() * 1.098632813 = km/h
 * * TL => get_mag_speed()/1,1280315 = km/h
 * */

int speedo_speed::get_mag_speed(){

	if(millis()-last_time_read>500){
		uint8_t sreg;
		uint16_t timerValue=0;
		sreg = SREG;		/* Save global interrupt flag */
		cli();				/* Disable interrupts */
		if(TCNT5>1){
			timerValue = TCNT5;	/* Read TCNTn into i */
			TCNT5=0;			/* Reset Timer value */
		}
		SREG = sreg;		/* Restore global interrupt flag */
		if(timerValue>1){
			int32_t differ=last_pulse_ts-last_calc_pulse_ts; // value around 2000ms to 100ms
			if(differ<0){
				differ=((uint32_t)-1)-differ;
			}
			last_calc_pulse_ts=last_pulse_ts;
			if(timerValue>(1<<8)){
				reed_speed=(((uint32_t)timerValue)<<16)/(differ>>7); // this could be 2 to 240
			} else {
				reed_speed=(((uint32_t)timerValue)<<23)/(differ); // this could be 2 to 240
			}
#ifdef TEMP_DEBUG
						Serial.println("Tempokontrolle: ");
						Serial.print(differ);
						Serial.print(" | ");
						Serial.print(timerValue);
						Serial.print(" | ");
						Serial.print(reed_speed);

						Serial.print(" | ");
						Serial.println(reed_speed/8.192);
			 //in 576 hz, 1256376, 764
			// in 12 hz,  1274432, 14
#endif
			last_time_read=millis();
		} else {
			// if we haven't even received a pulse in one sec than we are driving
			// reed sensor: less then 2m in one sec, less than 7.2 km/h
			// T3 8 pulses: less then 2m/8 in one sec, less than 0,9 km/h
			// hornet:  less then 2m/64 in one sec, less than 0,1125 km/h
			if(millis()-last_time_read>1000 && timerValue==0){
				reed_speed=0;
				last_time_read=millis();
			}
		}

	}
	return reed_speed/8.192;
};


ISR(INT5_vect ){

	geschwindigkeitssensor->last_pulse_ts = micros();

}
