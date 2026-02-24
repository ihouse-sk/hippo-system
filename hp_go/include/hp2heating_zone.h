#ifndef HP_2HEATING_ZONE_H
#define HP_2HEATING_ZONE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#include <map>
#include <iostream>
#include <fstream>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_HEATING_DATA_S
#define HP_HEATING_DATA_S
typedef struct {
	string ident;
	string value;
	int type;
	int off_delay;
	string debug_data;
} hp_heating_data_t;
#endif

#define HEATING_OFF 		0
#define HEATING_MANUAL 		1
#define HEATING_ECO		2
#define HEATING_AUTO		3
#define HEATING_COOLING		4
#define HEATING_DIFF_STEP 	0.05
#define HEATING_THRESH_TEMPERING 0.5

#define HEATING_OSC_PERIOD 	30// v minutach
#define HEATING_MAINTAIN_TIME	1

#define NORMAL_HEATING	0
#define TUNED_HEATING	1	

#define ACTOR_HEATER 0
#define ACTOR_COOLER 1

using namespace std;

class hp2heating_zone {
	public:
		hp2heating_zone(XMLNode, float def_temp);
		const vector<string> get_heaters() { return my_heaters;}
		const vector<string> get_collers() { return my_coolers;}
		const map<string,int> get_windows() { return my_windows;}
		void set_actual_temp (float temp) { my_actual_temp = temp;}
		float get_actual_temp() { return my_actual_temp; }
		string get_direction() { return my_direction; }
		string get_sensor_ident() { return my_sensor_ident; }
		void set_direction(string dir) { my_direction = dir;}
		void print_zone();
		int get_heater_state() { return my_heater_state;}
		int get_cooler_state() { return my_cooler_state;}
		void set_heater_state(int state) { my_heater_state = state;}
		void set_cooler_state(int state) { my_cooler_state= state;}
		bool is_blocked() { return my_blocked;}
		bool set_blocked(string ident, int state);
		~hp2heating_zone();
	private: 
		hp_xml_parser xml_parser;
		string my_sensor_ident;
		vector<string> my_heaters;
		vector<string> my_coolers;
		map<string,int> my_windows;
		float my_actual_temp;
		string my_direction;
		int my_cooler_state;
		int my_heater_state;
		bool my_blocked;
};

#endif
