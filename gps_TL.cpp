/*
 * gps_TL.cpp
 *
 *  Created on: 11.05.2014
 *      Author: Thomas
 */

#include "global.h"
//#define GPS_DEBUG

speedo_gps::speedo_gps(){
	gps_state=0;
	gps_alt_temp=0;
	gps_course=0;
	gps_date=0;
	gps_lati=0;
	gps_long=0;
	gps_sats_temp=0;
	gps_speed=0;
	gps_time=0;
	gps_time2=0;
	valid=false;
	checksum=0;
	gps_fix_temp=0;
	ringbuf_counter=0;
	gps_speed_GPVTG=0;
	gps_speed_km=0;
	gps_speed_temp=0;
	die_geschwindigkeit=0;

}

speedo_gps::~speedo_gps(){}


void speedo_gps::recv_data(uint8_t byteGPS){

	char speicher[1];
	sprintf(speicher,"%c",byteGPS);
	Serial.print(char(byteGPS));
	switch(gps_state){
	case 0:  // hier sitzen wir und warten auf das startzeichen
		if(byteGPS=='$'){
			gps_state=1;
			checksum=0x00;
		};
		break;
	case 1:  // das wird dann ein "G" sein
		gps_state=2;
		checksum^=byteGPS;
		break;
	case 2: // ein "P"
		gps_state=3;
		checksum^=byteGPS;
		break;
	case 3: // hier wirds interessant: isses ein R oder G oder V?
		if(byteGPS=='R'){
			gps_state=4;
			checksum^=byteGPS;
		} else if(byteGPS=='G'){
			gps_state=14; // hier mal einfach um 10 weiterspringen
			checksum^=byteGPS;
		} else if (byteGPS=='V'){ // TL: mit GPVTG kann ich auch noch was machen
			//Serial.print ("gpVtg gefunden");
			gps_state=18; // also springen wir un 14 weiter
			checksum^=byteGPS;
		};
		break;
	case 4: // auch noch interessant, hier müsste auf das R ein M folgen
		if(byteGPS=='M'){ // jaha
			gps_state=5;
			checksum^=byteGPS;
		} else {			// schade, doch nüscht
			gps_state=0;
		}
		break;
	case 5: // soweit gut $GPRM fehlt das C
		if(byteGPS=='C'){// jaha, ab in den "lese modus"
			ringbuf_counter=0; // position in die geschrieben werden soll
			gps_state=6;
			checksum^=byteGPS;
		} else			// schade, doch nüscht
			gps_state=0;
		break;
	case 6:
		if(ringbuf_counter>=SERIAL_BUFFER_SIZE-1){ gps_state=0; break; }    // überlauf, kein gps wert ist länger als 75 Byte. Mist
		gps_buffer1[ringbuf_counter]=byteGPS;   // in den buffer schmei�?en
		if(byteGPS=='*'){                // * das ist mein end signal dann is ende im gelände und abmarsch
			gps_state=7;
		} else if(byteGPS=='$'){ // damn da haben wir was verpasst
			checksum=0x00;
			gps_state=1; // jump start
		} else { // es scheint also ein ganz normales zeichen zu sein, dann könnte nur noch der hier schief gehen
			ringbuf_counter++;
			checksum^=byteGPS;
		}
		break;
	case 7: // checksum
		if(byteGPS>='A' && byteGPS<='F'){
			byteGPS=byteGPS-'A'+10;
		} else {
			byteGPS=byteGPS-'0';
		}
		checksum-=(byteGPS<<4);
		gps_state=8;
		break;
	case 8:
		if(byteGPS>='A' && byteGPS<='F'){
			byteGPS=byteGPS-'A'+10;
		} else {
			byteGPS=byteGPS-'0';
		}

		if(byteGPS==checksum){
			parse(gps_buffer1,1);
		}
		gps_state=0;
		break;
	case 14: // auch noch interessant, hier müsste auf das GPG ein G und dann ein A folgen
		if(byteGPS=='G'){ // jaha
			gps_state=15;
			checksum^=byteGPS;
		} else			// schade, doch nüscht
			gps_state=0;
		break;
	case 15: // jetzt fehlt nur noch das A
		if(byteGPS=='A'){ // jaha
			ringbuf_counter=0; // position in die geschrieben werden soll
			gps_state=16;
			checksum^=byteGPS;
		} else			// schade, doch nüscht
			gps_state=0;
		break;
	case 16:
		if(ringbuf_counter>=SERIAL_BUFFER_SIZE){ gps_state=0; break; }    // überlauf, kein gps wert ist länger als 75 Byte. Mist
		gps_buffer2[ringbuf_counter]=byteGPS;   // in den buffer schmei�?en
		if(byteGPS=='*'){                // * das ist mein end signal dann is ende im gelände und abmarsch
			gps_state=17;
		} else if(byteGPS=='$'){ // damn da haben wir was verpasst
			gps_state=1; // jump start
			checksum=0x00;
		} else { // es scheint also ein ganz normales zeichen zu sein, dann könnte nur noch der hier schief gehen
			ringbuf_counter++;
			checksum^=byteGPS;
		}
		break;
	case 17: // checksum
		if(byteGPS>='A' && byteGPS<='F'){
			byteGPS=byteGPS-'A'+10;
		} else {
			byteGPS=byteGPS-'0';
		}
		checksum-=(byteGPS<<4);
		gps_state=21;
		break;
	case 18: // auch noch interessant, hier muesste auf das V ein T folgen
		if(byteGPS=='T'){ // jaha

			gps_state=19;
			checksum^=byteGPS;
		} else {			// schade, doch nix
			gps_state=0;
		}
		break;
	case 19: // auch noch interessant, hier müsste auf das T ein G folgen
		if(byteGPS=='G'){ // jaha
			ringbuf_counter=0; // position in die geschrieben werden soll
			gps_state=20;
			checksum^=byteGPS;
		} else {			// schade, doch nüscht
			gps_state=0;
		}
		break;
	case 20:
		if(ringbuf_counter>=SERIAL_BUFFER_SIZE-1){ gps_state=0; break; }    // �berlauf, kein gps wert ist l�nger als 75 Byte. Mist
		gps_buffer3[ringbuf_counter]=byteGPS;   // in den buffer schmeissen
		if(byteGPS=='*'){        // * das ist mein end signal dann is ende im gel�nde und abmarsch
			gps_state=22;
		} else if(byteGPS=='$'){ // damn da haben wir was verpasst
			checksum=0x00;
			gps_state=1; // jump start
		} else { // es scheint also ein ganz normales zeichen zu sein, dann k�nnte nur noch der hier schief gehen
			ringbuf_counter++;
			checksum^=byteGPS;
		}
		break;
	case 21:

		if(byteGPS>='A' && byteGPS<='F'){
			byteGPS=byteGPS-'A'+10;
		} else {
			byteGPS=byteGPS-'0';
		}

		if(byteGPS==checksum){
			parse(gps_buffer2,2);
		}
		gps_state=0;
		break;
	case 22:
		if(byteGPS>='A' && byteGPS<='F'){
			byteGPS=byteGPS-'A'+10;
		} else {
			byteGPS=byteGPS-'0';
		}
		checksum-=(byteGPS<<4);
		gps_state=23;
		break;
	case 23:

		if(byteGPS>='A' && byteGPS<='F'){
			byteGPS=byteGPS-'A'+10;
			} else {
			byteGPS=byteGPS-'0';
		}

		if(byteGPS==checksum){
			parse(gps_buffer3,3);
		}
		gps_state=0;
		break;
		// end case
	}
}


/* parsen von nmea daten zu gps daten
 * im dümmsten fall haben wir die beiden Datensätze direkt hinter einander
 * dann haben wir durch die Statemashine 2 Byte für die Prüfsumme und 6 Byte für die anfangssequnz bis wir
 * in den gleichen serial_buffer reinschreiben
 * bei 4800 Baud sind das 1,6 ms für den Spass hier unten .. nicht gerade üppig
 * datensatz=1 => $GPRMC
 */
void speedo_gps::parse(char linea[SERIAL_BUFFER_SIZE],int datensatz){
	int cont=0;
	char status;
	int indices[16]; // positionsmarker für die kommata
	// gibt kein mod f�r ul ? son m�ll!

	//debug
	#ifdef GPS_DEBUG
	Serial.print("GPS wurde fuer Datensatz ");
	Serial.print(datensatz);
	Serial.println(" aufgerufen");
	Serial.println("get_gps hat bekommen: ");
	for(int i=0;i<SERIAL_BUFFER_SIZE;i++){
		Serial.print(linea[i]);
	};
	Serial.println("");
	#endif
	//debug
	// seperatorstellen suchen
	for (int i=0;i<SERIAL_BUFFER_SIZE;i++){
		if (linea[i]==','){    // check for the position of the  "," separator
			indices[cont]=i;
			cont=(cont+1)%20; // to prevent buffer overrun
		}
	}
	// modus 1, $gprmc empfangen

		if(datensatz==1){ // lat,long,fix,time,Course,date
		/*
	        $GPRMC,191410,A,4735.5634,N,00739.3538,E,0.0,0.0,181102,0.4,E,A*19
				   ^      ^ ^           ^            ^   ^   ^      ^     ^
				   |      | |           |            |   |   |      |     |
				   |      | |           |            |   |   |      |     Neu in NMEA 2.3:
				   |      | |           |            |   |   |      |     Art der Bestimmung
				   |      | |           |            |   |   |      |     A=autonomous (selbst)
				   |      | |           |            |   |   |      |     D=differential
				   |      | |           |            |   |   |      |     E=estimated (geschätzt)
				   |      | |           |            |   |   |      |     N=not valid (ungültig)
				   |      | |           |            |   |   |      |     S=simulator
				   |      | |           |            |   |   |      |
				   |      | |           |            |   |   |      Missweisung (mit Richtung)
				   |      | |           |            |   |   |
				   |      | |           |            |   |   Datum: 18.11.2002
				   |      | |           |            |   |
				   |      | |           |            |   Bewegungsrichtung in Grad (wahr)
				   |      | |           |            |
				   |      | |           |            Geschwindigkeit über Grund (Knoten)
				   |      | |           |
				   |      | |           Längengrad mit (Vorzeichen)-Richtung (E=Ost, W=West)
				   |      | |           007° 39.3538' Ost
				   |      | |
				   |      | Breitengrad mit (Vorzeichen)-Richtung (N=Nord, S=Süd)
				   |      | 46° 35.5634' Nord
				   |      |
				   |      Status der Bestimmung: A=Active (gültig); V=void (ungültig)
				   |
				   Uhrzeit der Bestimmung: 19:14:10 (UTC-Zeit)
		 *
		 */
		status=linea[indices[1]+1];

		// zeit in jedem fall berechnen, auch wenn das sample nicht valid ist, das ist der RTC
		long temp_gps_time=0;
		long temp_gps_date=0;
		for (int j=indices[0]+1;j<indices[1] && linea[j]>='0' && linea[j]<='9';j++){ // format 234500.000 für füddelvorzwölf
			if(linea[j]>='0' && linea[j]<='9'){
				temp_gps_time=temp_gps_time*10+(linea[j]-'0');
			}
		}

		for (int j=indices[8]+1;j<(indices[9]) && linea[j]>='0' && linea[j]<='9';j++){
			temp_gps_date=temp_gps_date*10+(linea[j]-'0'); //181102 für 18.Nov 2002

		}

		if(status=='A'){ // only evaluate if valid (A means valid)
			//valid=0;
			gps_time=temp_gps_time;
			//Serial.print("***************************GPS TIME UPDATE:");
			//Serial.println(gps_time);
			gps_date=temp_gps_date;
			//Serial.print("***************************GPS DATE UPDATE:");
			//Serial.println(gps_date);
			for (int j=indices[7]+1;j<(indices[8])-2;j++){ // keine Nachkommastelle mehr
				if(linea[j]!=46){
					gps_course=gps_course*10+(linea[j]-48);
				};
			}
			for (int j=indices[2]+1;j<(indices[3]);j++){ // keine Nachkommastelle mehr, punktkiller format 1-3 vorkomma und immer 6(!) nachkomma
				if(linea[j]!=46){
					gps_lati=gps_lati*10+(linea[j]-48);
				};
			}

			for (int j=indices[4]+1;j<(indices[5]);j++){ // keine Nachkommastelle mehr, punktkiller format 1-3 vorkomma und immer 6(!) nachkomma
				if(linea[j]!=46){
					gps_long=gps_long*10+(linea[j]-48);
				};
			}

				gps_speed=0;
			for (int j=indices[6]+1;j<(indices[7]);j++){
				if(linea[j]!=46){
					gps_speed=gps_speed*10+(linea[j]-48);
				};
				//Serial.println("gps_speed_TL:");
				//Serial.print(gps_speed);
			}
		}
		else{
			Serial.println("******GPS****ALARM****************");
		}

		// debug
#ifdef GPS_DEBUG
		//Serial.print("Time in UTC (HhMmSs): ");
		//Serial.println(gps_time);
		//Serial.print("Date UTC (DdMmAa) ");
		//Serial.println(gps_date);
		//Serial.print("Heading in degrees:(*10)");
		//Serial.println(gps_course);
		//Serial.print("Latitude: ");
		//Serial.println(gps_lati);
		//Serial.print("Longitude: ");
		//Serial.println(gps_long);
		//Serial.print("Speed kmh: ");
		//Serial.println(gps_speed);

		//Serial.print("alt: ");
		//Serial.println(gps_alt_temp);
		//Serial.print("sats: ");
		//Serial.println(gps_sats_temp);
		//Serial.print("fix? 1==ja: ");
		//Serial.println(gps_fix_temp);
#endif

		//Debug
		// anderer modus, gpgga empfangen
		}else if(datensatz==2){ // altitude, fix ok?,sats,
		/* GPS Datensatz 2:
        $GPGGA,191410,4735.5634,N,00739.3538,E,1,04,4.4,351.5,M,48.0,M,,*45
			   ^      ^           ^            ^ ^  ^   ^       ^
			   |      |           |            | |  |   |       |
			   |      |           |            | |  |   |       Höhe Geoid minus
			   |      |           |            | |  |   |       Höhe Ellipsoid (WGS84)
			   |      |           |            | |  |   |       in Metern (48.0,M)
			   |      |           |            | |  |   |
			   |      |           |            | |  |   Höhe über Meer (über Geoid)
			   |      |           |            | |  |   in Metern (351.5,M)
			   |      |           |            | |  |
			   |      |           |            | |  HDOP (horizontal dilution
			   |      |           |            | |  of precision) Genauigkeit
			   |      |           |            | |
			   |      |           |            | Anzahl der erfassten Satelliten
			   |      |           |            |
			   |      |           |            Qualität der Messung
			   |      |           |            (0 = ungültig)
			   |      |           |            (1 = GPS)
			   |      |           |            (2 = DGPS)
			   |      |           |            (6 = geschätzt nur NMEA-0183 2.3)
			   |      |           |
			   |      |           Längengrad
			   |      |
			   |      Breitengrad
			   |
			   Uhrzeit
		 *
		 *	fix=Qualität der Messung (zwischen dem 5. und 6. ",")
		 *	sats=Anzahl der erfassten Satelliten (zwischen dem 6. und 7. ",")
		 *
		 */
		gps_sats_temp=0;
		for (int j=indices[5]+1;j<(indices[6]);j++){
			gps_fix_temp=(linea[j]-48);
		};
		if(gps_fix_temp>0){ // fix>0
			for (int j=indices[6]+1;j<(indices[7]);j++){
				gps_sats_temp=gps_sats_temp*10+(linea[j]-48);
			}
			for (int j=indices[8]+1;j<(indices[9]);j++){
				if(linea[j]!=46){
					gps_alt_temp=gps_alt_temp*10+(linea[j]-48);
				};
			}
			if(gps_alt_temp>100000){ // sind wir höher als 10 km?
				gps_alt_temp=0;
			}
			gps_alt=gps_alt_temp;
			long temp_gps_time2 =0;
			for (int j=indices[0]+1;j<indices[1] && linea[j]>='0' && linea[j]<='9';j++){ // format 234500.000 für füddelvorzwölf
						if(linea[j]>='0' && linea[j]<='9'){
							temp_gps_time2=temp_gps_time2*10+(linea[j]-'0');
						}
			gps_time2=temp_gps_time2;
#ifdef GPS_DEBUG
						Serial.print("***********GPS TIME 2 UPDATE:************");
						Serial.println(gps_time2);
#endif
			}
			};

	//debug
	//#ifdef GPS_DEBUG
	//		Serial.print("altitude: ");
	//		Serial.println(gps_alt_temp);
	//		Serial.print("sats: ");
	//		Serial.println(gps_sats_temp);
	//		Serial.print("fix? 1==ja: ");
	//	Serial.println(gps_fix_temp);
	//#endif
	//debug

		//TL Variante 3 $GPVTG
		}else if(datensatz==3){
		//Serial.print ("$GPVTG gefunden:");//
		/* GPS Datensatz 3:
	        $GPVTG

Track Made Good and Ground Speed.

eg1. $GPVTG,360.0,T,348.7,M,000.0,N,000.0,K*43
eg2. $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K

            054.7,T      True track made good
                    034.4,M      Magnetic track made good
                            005.5,N      Ground speed, knots
                                     010.2,K      Ground speed, Kilometers per hour
		 */
		/*TL
 GPVTG liefert bei Geschwindigkeit in km/h z.B. ..,125.5,,..*43
 Ich lese:
 - linea[j]=1
 - gps_speed_temp= 0
 - gps_speed_temp = 0*10+(1-'0')=1
 linea[j]=2
 -gps_sped_GPVTG = 1*10+(2-'0')=12
 linea[j]=5
 -gps_speed_GPVTG = 12*10+(5-'0')=125
 linea[j]=. Bedingung nicht erf�llt, also machen wir nichts
 linea[j]=5
 -gps_speed_GPVTG = 125*10+(5-'0')=1255
		 */
	gps_speed_GPVTG=0;
	gps_speed_temp=0;
		for (int j=indices[6]+1;j<(indices[7]);j++){
			if(linea[j]!=46){
				gps_speed_temp=gps_speed_temp*10+(linea[j]-48);
				//Serial.println("Kontrolle gps_speed_GPVTG:");
				//Serial.print (gps_speed_temp);
			};
		}
		gps_speed_GPVTG=gps_speed_temp;
	//debug
#ifdef GPS_DEBUG
		Serial.print ("speed GPVTG: gps_speed_temp/gps_speed_GPVTG:");
		Serial.print(gps_speed_temp);
		Serial.print("/");
		Serial.println(gps_speed_GPVTG);

#endif
		//debug
		}
};


