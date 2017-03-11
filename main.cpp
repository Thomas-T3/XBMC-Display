#include <global.h>
debugging*				pDebug=new debugging();

// define global vars
ssd0323 OLED;
speedo_temperature*	pTemp=new speedo_temperature();
speedo_speed*   geschwindigkeitssensor=new speedo_speed();// original speedo_speedo* usw
speedo_dz* drehzahlsignal=new speedo_dz();
speedo_gps* gps=new speedo_gps();
speedo_Kw* Mkl=new speedo_Kw();
//speedo_SD_Karte* km_Stand=new speedo_SD_Karte();
//speedo_SD_Karte* km_speichern= new speedo_SD_Karte();


char disp_sha[4][21];

// debug
#undef DEBUG

// speicherplätze, wo du die im projekt hast ist egal (kann sogar mitten im code sein)
// es ist nur einfacher wenn man sie alle oben im kopf hat
// kannst du auch auf 0, 1 ,2 ... oder was auch immer ändern
// das sollten halt nur EINDEUTIGE werte sein, sonst vergleichst du volt mit wasser ;)

#define KUEHLWASSER_WERT 4 // ach ja, natürlich keine umlaute, nech
#define OELTEMPERATUR 5
#define BATTERIE 6
#define TEMPO 7
#define DREHZAHL 8
#define GANG 9
#define INNENTEMPERATUR 10
#define AUSSENTEMP 11
#define ZEIT 12
#define DATE 13
#define KM_STAMD 14
#define SEITE 15
#define HOEHE 15
//+++++++++++++++++++++++ Tasten fuer Menue +++++++++++++++++++++++++ //
const int button_right=41;
const int button_left=43;
int button_state_right= HIGH;
int button_state_left= HIGH;
int button_counter;
char buffer[60];
// start all corresponding server in initialize the vars
void setup()
{
	Serial.begin(115200);
	OLED.init(0xA8,0x28);
	OLED.filled_rect(0,0,128,64,0x00);
	//pSD->init();
}

void ram_info()
{
	int size = 8192; // Use 2048 with ATmega328
	byte *buf;
	while ((buf = (byte *) malloc(--size)) == NULL);
	free(buf);
  //#ifdef TEMP_DEBUG
	Serial.print(size);
	Serial.println(" Byte heap free");
//#endif
}
int main(void)
{
	init();
	setup();
	Serial.begin(115200);
	Serial.print("losgehts");
	OLED.string(VISITOR_SMALL_2X_FONT,"Toms",6,1);
	OLED.string(VISITOR_SMALL_2X_FONT,"Bus-Tacho",2,4);
	delay(5000);
	OLED.clear_screen();
	OLED.draw_linie(0,20);
	OLED.draw_clock(3,0);
	OLED.draw_oil(32,10);
	OLED.draw_water(34,0);
	OLED.draw_batterie (27,47);
	OLED.draw_linie(0,42);
	OLED.draw_sat(60,47);
	OLED.string(VISITOR_SMALL_1X_FONT,"H/m",0,6);
	OLED.string(VISITOR_SMALL_1X_FONT,"OUT",4,6);
	OLED.string(VISITOR_SMALL_1X_FONT,"DAT.",0,1);

	//*********************************km_Stand von SD Karte****************
	//			long km_first= km_Stand->speicherwert;
	//
	//*********************************km_Stand von SD Karte****************

	pTemp->init(); // brauchst du eigentlich nicht aber gibt einen status aus
	pTemp->check_vars(); // da werden "notfalls" standard werte eingetragen
	geschwindigkeitssensor->init();
	drehzahlsignal->init();
	//km_Stand->init();

	while(1)
	{
		//*********************************Taster********************************
		int button_right_temp=digitalRead(button_right);
		int button_right_temp2;
		if (button_right_temp==LOW){ // war da was?
			delay(100);
			button_right_temp2=digitalRead(button_right);
			if ((button_right_temp==LOW)&&(button_right_temp2==LOW)&& (button_counter!=1)){ // ja da war was, dann los
				button_counter=1;
				delay(200);
				OLED.clear_screen();
		// ANFANG **************************MOTORKONTROLLEUCHTE + STEUERGERÄTEDATEN******************
				OLED.draw_Mil(0,6);
				OLED.string(VISITOR_SMALL_1X_FONT,"Fehlerspeicher",4,1);
				OLED.draw_linie(0,15);
				Mkl->readError();
				int Fehlercode= Mkl->get_Error_Temp();
				char Fehler1[7];
				sprintf(Fehler1,"%i",Fehlercode);
				OLED.string(VISITOR_SMALL_1X_FONT,Fehler1,0,2);
				int pointer_for_array=Mkl->get_Pointer_for_Array();
				char Pointer1[3];
				sprintf(Pointer1,"%i",pointer_for_array);
				int reason=Mkl->get_Reason_Temp();
				char Reason1[4];
				sprintf(Reason1,"%i",reason);
				int reason_Pointer=Mkl->get_Reason_Pointer();
				char Pointer2[4];
				sprintf(Pointer2,"%i",reason_Pointer);

		#ifdef TEMP_DEBUG
				Serial.print("Fehlercode / Pointer: ");
				Serial.print(Fehler1);
				Serial.print(" / ");
				Serial.println(Pointer1);
				Serial.print("Fehlergrund / Pointer: ");
				Serial.print(Reason1);
				Serial.print(" / ");
				Serial.println(Pointer2);
				Serial.print("Taster rechts low:");
				Serial.print(button_counter);
		#endif
			}
			else {
				Serial.print("Taster rechts high:");
				Serial.print(button_counter);
			}
		}



		int button_left_temp=digitalRead(button_left);
		int button_left_temp2;
		if (button_left_temp==LOW){
			delay(100);
			button_left_temp2=digitalRead(button_left);
			if ((button_left_temp==LOW)&&(button_left_temp2==LOW)&& (button_counter!=2)){
				button_counter=2;
				delay(200);
				OLED.clear_screen();
				OLED.string(VISITOR_SMALL_2X_FONT,"Seite 2",1,1);
				delay(2000);
				OLED.clear_screen();
				Serial.print("Taster links low:");
				Serial.print(button_counter);
			}
		}
// ENDE **************************MOTORKONTROLLEUCHTE + STEUERGERÄTEDATEN******************
		if (button_counter==0){
			ram_info();
			int disp_zeile_bak[21];// backup bestimmter werte um abzuschÃ¤tzen ob die Zeile geupdated werden sollte

		//***************************Anfang**Kühlwasser****************************

			pTemp->read_water_temp(); // <- das hier ruhig einmal pro sek aufrufen
			int wasser_temperatur;
			wasser_temperatur = pTemp->get_water_temp();
			if( (abs(disp_zeile_bak[KUEHLWASSER_WERT]-(wasser_temperatur+1))>1) ||
				(abs(disp_zeile_bak[KUEHLWASSER_WERT]-(wasser_temperatur+1))==1)) {
				disp_zeile_bak[KUEHLWASSER_WERT]=(wasser_temperatur+1);
				char mein_string1[3];
				sprintf(mein_string1,"%03i{C",wasser_temperatur);
#ifdef TEMP_DEBUG
			Serial.print("Wassertemperatur: ");
			Serial.print(mein_string1);
#endif
				if(wasser_temperatur>35 && wasser_temperatur<130){
					OLED.string(VISITOR_SMALL_1X_FONT,mein_string1,16,0);
				}
				else{
					OLED.string(VISITOR_SMALL_1X_FONT,"--{C",16,0);
				}
			}
		//***************************Ende Kühlwasser****************************

		//***************************Anfang Öltemperatur****************************
			pTemp->read_oil_temp();
			int oil_temperatur;
			oil_temperatur = pTemp->get_oil_temp();
			//oil_temperatur=round(oil_temperatur/10);
			if( (abs(disp_zeile_bak[OELTEMPERATUR]-(oil_temperatur+1))>1) || (abs(disp_zeile_bak[OELTEMPERATUR]-(oil_temperatur+1))==1 ) ) {
				disp_zeile_bak[OELTEMPERATUR]= (oil_temperatur+1);
				char mein_string2[2];
				sprintf(mein_string2,"%03i{C",oil_temperatur);
#ifdef TEMP_DEBUG
			Serial.print("Oeltemperatur: ");
			Serial.print(mein_string2);
#endif
				if(oil_temperatur>20 && oil_temperatur<150){
					OLED.string(VISITOR_SMALL_1X_FONT,mein_string2,16,1);
				}
				else{
				OLED.string(VISITOR_SMALL_1X_FONT,"--{C",16,1);
				}

			}
		//***************************Ende Öltemperatur****************************

		//*************************Anfang Batteriespannung****************************
			int analog_value;
			//	Serial.print("Spannung: ");
			//	Serial.print("alter Wert:");
			analog_value = analogRead(0);
			Serial.print("analog:");
			Serial.println(analog_value);
			uint32_t milli_volt = (analog_value*4.88); //vorher 4.01!! 400*5000/1024 ~=400*5 = 2000
			milli_volt = (milli_volt*5.18);
			Serial.print("milli_volt:");
			Serial.println(milli_volt);
			int ganze_volt=(milli_volt/1000); // 2000/1000 = 2
			Serial.print("ganze_volt:");
			Serial.println(ganze_volt);
			int nachkomma=(milli_volt%1000)/10; // (2000%1000)/10 = 0/10 = 0 oder (2120%1000)/10=120/10=12
			if( (abs(disp_zeile_bak[BATTERIE]-(nachkomma+1))>50)  ) {
					Serial.print("nachkomma:");
					Serial.println(nachkomma);
					disp_zeile_bak[BATTERIE]=(nachkomma+1);
					char mein_string[3];
					sprintf(mein_string,"%i,%1i V ",ganze_volt,nachkomma); // "in volt 2.0"
					if (ganze_volt>0 && ganze_volt<16){
						OLED.string(VISITOR_SMALL_1X_FONT,mein_string,8,7);
					}
					else{
						OLED.string(VISITOR_SMALL_1X_FONT,"--,-- V",8,7);
					}
#ifdef TEMP_DEBUG
					Serial.println("Ausgabe Spannung: ");
					Serial.print(mein_string);
#endif
			}
		//****************************Ende Batteriespannung****************************

		//**************************Anfang Geschwindigkeit****************
		//********Anzahl Satelliten per $GPGGA
			int Satelliten=gps->gps_sats_temp;
			char AnzahlSat[3];
			sprintf(AnzahlSat,"%2i",Satelliten);
		//******* Geschwindigkeit über Satellit ******************
			int Speed1=gps->gps_speed_GPVTG;
			Speed1=round(Speed1/100);
			char Tempo1[4];
			sprintf(Tempo1,"%3i",Speed1);
		// ************* Geschwindigkeit über Impulsgeber ********
			long die_geschwindigkeit=geschwindigkeitssensor->get_mag_speed();
		/*
		 *  * Fortschreibung km Stand / Berechnung gefahrene Strecke
		 * ********************************************************
		 * 8 Impule = 1,97m
		 * 100m = 100m / 1,97m = 50.761421 * 8 Impulse = 406 Impulse auf 100 m
		 * Also ca. alle 406 Impulse bin ich 100 m gefahren. Dann km Stand + 0,1km
 *
		 */

	/*	int reed_speed_trip1= (die_geschwindigkeit*8,192); //=Anzahl Impulse
		int reed_speed_trip2= (reed_speed_trip2+reed_speed_trip1);// Anzahl Impulse plus neue Anzahl Impulse
			if(reed_speed_trip2>406){
				trip_meter=trip_meter+1;
				reed_speed_trip2=0;

			}
			float km_second= ((km_first+trip_meter)/10);
			char km_Stand[8];
			sprintf(km_Stand,"%07i{C",km_second);
#ifdef TEMP_DEBUG
		Serial.print("km Stand von SD: ");
		Serial.print(km_Stand);
#endif
	if ((die_geschwindigkeit==0)&&(km_second*10>=km_first+10)){
			long km_neu = km_speichern ->naechster();
			char km_Stand_neu[9];
					sprintf(km_Stand,"%07i{C",km_neu);
		#ifdef TEMP_DEBUG
				Serial.print("km Stand neu auf SD: ");
				Serial.print(km_neu);
		#endif
		}
*/
			die_geschwindigkeit = (die_geschwindigkeit/1.1280315);
			char Geschwindigkeit[4];
			if( (abs(disp_zeile_bak[TEMPO]-(die_geschwindigkeit+1))>1) ) {
				disp_zeile_bak[TEMPO]=(die_geschwindigkeit+1);
				sprintf(Geschwindigkeit,"%3i\n",die_geschwindigkeit);
				OLED.string(VISITOR_SMALL_2X_FONT,Geschwindigkeit,0,3);
				OLED.string(VISITOR_SMALL_1X_FONT,"km/h",7,4);
				OLED.string(VISITOR_SMALL_1X_FONT,"   ",8,3);
				Serial.print("Geschwindigkeit: ");
				Serial.println(Geschwindigkeit);
			}

	//*********************Ausgabe: >70 =GPS;<70 speedpuls *****************
			if ((Satelliten>=4)&&(Speed1>70)) {
				OLED.string(VISITOR_SMALL_2X_FONT,Tempo1,0,3);
				OLED.string(VISITOR_SMALL_1X_FONT,"km/h",7,4);
				OLED.string(VISITOR_SMALL_1X_FONT,"GPS",8,3);
				Serial.print ("Tempo1: ");
				Serial.println(Tempo1);
			}
			else{
				if( (abs(disp_zeile_bak[TEMPO]-(die_geschwindigkeit+1))>1) ) {
					disp_zeile_bak[TEMPO]=(die_geschwindigkeit+1);
					sprintf(Geschwindigkeit,"%3i\n",die_geschwindigkeit);
					OLED.string(VISITOR_SMALL_2X_FONT,Geschwindigkeit,0,3);
					OLED.string(VISITOR_SMALL_1X_FONT,"km/h",7,4);
					OLED.string(VISITOR_SMALL_1X_FONT,"   ",8,3);
					//Serial.print ("Geschwindigkeit<70: ");
					//Serial.println(Geschwindigkeit);
				}
			}
		//*************************** Ende Geschwindigkeit*****************************

		//****************************Anfang Drehzahl*****************************

			long RPM;
			RPM=drehzahlsignal->get_dz();
#ifdef TEMP_DEBUG
			Serial.print("Drehzahl");
			Serial.println(RPM);
#endif
			int RPM_Gang=RPM;
			RPM=(RPM/100);
			RPM=(RPM*100);
			if( (abs(disp_zeile_bak[DREHZAHL]-(RPM+1))>100) || (abs(disp_zeile_bak[DREHZAHL]-(RPM+1))==100) ) {
				disp_zeile_bak[DREHZAHL]=(RPM+1);
				char Umdrehungen[5];
				sprintf(Umdrehungen,"%4i\n",RPM);
				OLED.string(VISITOR_SMALL_2X_FONT,Umdrehungen,12,3);
			}
			else{
				char Umdrehungen[5];
				sprintf(Umdrehungen,"%4i\n",RPM);
				OLED.string(VISITOR_SMALL_2X_FONT,Umdrehungen,12,3);
			}
		//***************************Ende Drehzahl*****************************

		//***************************Anfang Gang*****************************
			int gang=0;
			//long die_geschwindigkeit;
			die_geschwindigkeit=geschwindigkeitssensor->get_mag_speed();
			die_geschwindigkeit = (die_geschwindigkeit/1.1280315);
			long wert_gang = ((die_geschwindigkeit*1000)/RPM_Gang); // Gangwert berechnen
#ifdef TEMP_DEBUG
			Serial.println("berechneter Gang:");
			Serial.print(wert_gang);
#endif
			if (wert_gang >1 && wert_gang< 8){				// Gangwert mit Festwert vergleichen
				gang=1;							//1= 0,598
			}
			if (wert_gang >=8 && wert_gang<12){
				gang=2;							// 2=1,055
			}
			if (wert_gang>=12 && wert_gang<19){
				gang=3;							// 3=1,666
			}
			if (wert_gang>=19 && wert_gang<26){
				gang=4;							// 4= 2,404
			}
			if (wert_gang>=26){
				gang=5;							//5=2,999
			}
			if (wert_gang>=33){
				gang=0;
			}
			if ((die_geschwindigkeit==0)&& (RPM_Gang==0)){
				gang=0;
			}
			if ( (abs(disp_zeile_bak[GANG]-(gang+1))>1 )) {
				disp_zeile_bak[GANG]=(gang+1);
				if (gang>0){
					char Gang[2];
					sprintf(Gang,"%i",gang%10);
					OLED.string(VISITOR_SMALL_2X_FONT,Gang,17,6);
				}
				else{
					OLED.string(VISITOR_SMALL_2X_FONT,"N",17,6);
				}
				if (gang>0 && gang<5 && RPM_Gang>2800){
					OLED.draw_Pfeil(44,45);
				}
				else{
					OLED.draw_no_Pfeil(44,45);
				}
			}
		//****************************Ende Gang*****************************

		//*************************Anfang Temperaturen per LM73*************

			pTemp->read_air_temp();
			int InnenTemperaturwert;
			InnenTemperaturwert=pTemp->get_air_temp();
			if( (abs(disp_zeile_bak[INNENTEMPERATUR]-(InnenTemperaturwert+1))>1) ||
				(abs(disp_zeile_bak[INNENTEMPERATUR]-(InnenTemperaturwert+1))==1 ) ) {
				disp_zeile_bak[INNENTEMPERATUR]=(InnenTemperaturwert+1);
				char mein_string5[3];
				sprintf(mein_string5,"%2i{",InnenTemperaturwert);//InnenTemperaturwert%100)
#ifdef TEMP_DEBUG
				Serial.print("Innentemperaturausgabe: ");
				Serial.println(mein_string5);
#endif
				OLED.string(VISITOR_SMALL_1X_FONT,mein_string5,0,7);
			}
			else{
				char mein_string5[3];
				sprintf(mein_string5,"%2i{C",InnenTemperaturwert);
#ifdef TEMP_DEBUG
				Serial.print("Innentemperaturausgabe: ");
				Serial.println(mein_string5);
#endif
			}

			pTemp->read_air_temp_out();
			int AussenTemperaturwert;
			AussenTemperaturwert=pTemp->get_air_temp_out();
			if( (abs(disp_zeile_bak[AUSSENTEMP]-(AussenTemperaturwert+1))>1) ||
				(abs(disp_zeile_bak[AUSSENTEMP]-(AussenTemperaturwert+1))==1 ) ) {
				disp_zeile_bak[AUSSENTEMP]=(AussenTemperaturwert+1);
				char mein_string6[3];
				sprintf(mein_string6,"%2i{",AussenTemperaturwert);
#ifdef TEMP_DEBUG
				Serial.print("Aussentemperaturausgabe: ");
				Serial.println(mein_string6);
#endif
				OLED.string(VISITOR_SMALL_1X_FONT,mein_string6,4,7);
			}
			else{
				char mein_string6[3];
				sprintf(mein_string6,"%2i{C",AussenTemperaturwert);
#ifdef TEMP_DEBUG
				Serial.print("Aussentemperaturausgabe: ");
				Serial.println(mein_string6);
#endif
			}

		//***************************Ende Temperaturen per LM73*************


		//**********Datum / Uhrzeit / Geschwindigkeit / Höhe in m per GPS*********
			Serial2.begin(38400);
			int winter = 0;
			while(Serial2.available()){
				gps->recv_data(Serial2.read());
			}

		//************************Datum per GPS
			long Date=gps->gps_date;
			Date=round(Date/100);
			int Tag= (Date/100);
			int Monat = (Date)%100;
			if ((Monat>03) && (Monat<=10)){
				winter =1;
			}
			char Datum[6];
			sprintf(Datum,"%02i.%02i",Tag,Monat);
#ifdef TEMP_DEBUG
			Serial.print("Datum");
			Serial.println(Datum);
#endif
			if (Date>0){
				OLED.string(VISITOR_SMALL_1X_FONT,Datum,4,1);
			}
			else{
				OLED.string(VISITOR_SMALL_1X_FONT,"--.--",4,1);
			}

		//*****************************************Zeit per GPS
			long Time=gps->gps_time;

			if(Time>0){
				Time=round(Time/100);
				int STD = (Time/100);
				if (winter==1){
					STD=STD+2;
				}
				else{ STD=STD+1;
				}
				if(STD>23){
					STD=(STD-24);
				}
				int MIN = (Time)%100;
				char Zeit[6];
				sprintf(Zeit,"%2i:%02i",STD,MIN);
#ifdef TEMP_DEBUG
		Serial.print("Zeit");
		Serial.println(Zeit);
#endif
				if (Time>0 && Time<2359){
					OLED.string(VISITOR_SMALL_1X_FONT,Zeit,4,0);
				}
			}
			else{
				long Time2=gps->gps_time2;
				if(Time2>0){
					Time2=round(Time2/100);
					int STD2 = (Time2/100);
					if (winter==1){
						STD2=STD2+2;
					}
					else{ STD2=STD2+1;
					}
					if(STD2>23){
						STD2=(STD2-24);
					}
					int MIN2 = (Time2)%100;
					char Zeit2[6];
					sprintf(Zeit2,"%2i:%02i",STD2,MIN2);
#ifdef TEMP_DEBUG
				Serial.print("Zeit2");
				Serial.println(Zeit2);
#endif
					if (Time2>0 && Time2<2359){
						OLED.string(VISITOR_SMALL_1X_FONT,Zeit2,4,0);
					}
				}
			}


		//**************************Anzahl Satelliten per $GPGGA
			int Sat=gps->gps_sats_temp;
			char AnzSat[3];
			sprintf(AnzSat,"%2i",Sat);
			//Serial.print("Anzahl Satelliten:");
			//Serial.println(AnzSat);
			OLED.string(VISITOR_SMALL_1X_FONT,AnzSat,19,7);
		//**************************Anzahl Satelliten per $GPGGA

		//**************************Höhe über Geoid in m
			int Hoehe=gps->gps_alt_temp;
			int HoeheInMeter = Hoehe/10;
#ifdef TEMP_DEBUG
			Serial.print("Höhe ");
			Serial.println(HoeheInMeter);
#endif
			char hoehe_in_m[4];
			sprintf(hoehe_in_m,"%3i",HoeheInMeter);
			if( (abs(disp_zeile_bak[HOEHE]-(HoeheInMeter+1))>3) ) {
					disp_zeile_bak[HOEHE]=(HoeheInMeter+1);
					Serial.print("Neue Höhe:");
					Serial.println(HoeheInMeter);
					if (HoeheInMeter<1000){
						OLED.string(VISITOR_SMALL_1X_FONT,hoehe_in_m,0,7);
					}
			}
		//**************************Höhe über Geoid in m

		//**************************Speed per GPS $GPVTG
		/*int Speed1=gps->gps_speed_GPVTG;
		Speed1=round(Speed1/100);
		char Tempo1[4];
		sprintf(Tempo1,"%3i",Speed1);
		Serial.print("Speed1:");
		Serial.print(Tempo1);

		int gps_speed_km=gps->gps_speed; //Speed per GPGGA
		gps_speed_km= round((gps_speed_km*1.852)/10);
		//if (gps_speed_km>=0 && gps_speed_km<200){
		char Tempo[4];
		sprintf(Tempo,"%3i",gps_speed_km);
		//Serial.print("Speed GPRMC:");
		//Serial.print(Tempo);
		//if ((Speed1>5)&&(Speed1 <201)){
		//	OLED.string(VISITOR_SMALL_1X_FONT,Tempo1,8,3);
		//}
		//else
		if((gps_speed_km>5)&&(gps_speed_km<201)){
			OLED.string(VISITOR_SMALL_1X_FONT,Tempo,8,3);
		}
		else{
			OLED.string(VISITOR_SMALL_1X_FONT,"---",8,3);
		}
	}

	//**********Datum / Uhrzeit / Geschwindigkeit per GPS*********
*/
		}
	}
};

