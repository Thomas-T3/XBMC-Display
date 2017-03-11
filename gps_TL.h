/*
 * gps.h
 *
 *  Created on: 01.06.2011
 *      Author: kolja
 */

#ifndef GPS_H_
#define GPS_H_

#define    SERIAL_BUFFER_SIZE 75   // grÃ¶Ãe des char buffers fÃ¼r die seriellen gps daten

/**************** gps *******************/
class speedo_gps{
public:
	speedo_gps();
	~speedo_gps();
	void init();

	void recv_data(uint8_t input);

	unsigned long nmea_to_dec(unsigned long nmea);

	// die letzen 30 infos
	unsigned long gps_lati,gps_long;
	unsigned int gps_sats_temp,gps_course,gps_speed, gps_speed_km,gps_speed_GPVTG,die_geschwindigkeit;
	long gps_time,gps_time2;
	long gps_date;
	int valid;
	long gps_alt_temp,gps_alt;
private:
	uint8_t checksum;
	// die letzen 30 infos
	unsigned int gps_fix_temp,gps_speed_temp;
	//long gps_alt_temp;
	//long gps_alt;

	int gps_state;
	int ringbuf_counter; // position im ring-empfangs-buffer
	char gps_buffer1[SERIAL_BUFFER_SIZE]; // buffer zum entgegennehmen der seriellen daten
	char gps_buffer2[SERIAL_BUFFER_SIZE]; // buffer zum entgegennehmen der seriellen daten
	char gps_buffer3[SERIAL_BUFFER_SIZE]; // buffer zum entgegennehmen der seriellen daten
	void parse(char linea[SERIAL_BUFFER_SIZE],int datensatz);
	unsigned long mod(unsigned long zahl,unsigned long teiler);
};
/**************** gps *******************/
extern speedo_gps* gps;

#endif /* GPS_H_ */
