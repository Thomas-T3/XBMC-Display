/*
 * speedo.h
 *
 *  Created on: 01.06.2011
 *      Author: kolja
 */

#ifndef SPEEDO_H_
#define SPEEDO_H_

typedef struct {
	int8_t x;
	int8_t y;
	int8_t font;
	bool symbol;
} widget;

//109982 - 4737
//110308 - 4899 :( --struct
//110414 - 4953 :(( -- kmhchars
//109088 - 4467 :] -- die elseif geschichte gelöscht -> nur ein test, keine reale variante
//110432 - 4245 :D --string in flash
//111696 - 3715 :D --string in flash


class speedo_speedo{


#define			GPS_VALUE 11

public:
	void loop(unsigned long previousMillis);
	void reset_bak();
	void clear_vars();
	void check_vars();
	int 		  disp_zeile_bak[12];// backup bestimmter werte um abzuschätzen ob die Zeile geupdated werden sollte
	int           max_speed[9];      // array of the max speed values sollte dem speedo_speedo gehören
	unsigned long avg_timebase[9];

	int 		  refresh_cycle;


	private:
	bool check_no_collision_with_addinfo2(int current_widget_y);
	bool addinfo2_currently_shown;

};
extern speedo_speedo* pSpeedo;

#endif /* SPEEDO_H_ */
