/*
 * speed.h
 *
 *  Created on: 01.06.2011
 *      Author: kolja
 */

#ifndef SPEED_H_
#define SPEED_H_

class speedo_speed{
#define SPEED_PIN PE5
#define SPEED_REED_OK 0
#define SPEED_REED_ERROR 1
public:
	speedo_speed();
	~speedo_speed();
	void init();
	void shutdown();
	int get_mag_speed();
	volatile unsigned long last_pulse_ts;

private:


	int           reed_speed;  // speichere speed darin, wenn valid_time noch < 1000 ist gib das hier aus
	unsigned long last_time_read;  // wenn dieser Zeitpunkt über 1000 msec her ist stehen wir
	unsigned long last_calc_pulse_ts;
};

extern speedo_speed*   geschwindigkeitssensor;

#endif /* SPEED_H_ */
