#ifndef HP_HEATING_SECTION_H
#define HP_HEATING_SECTION_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <mysql/mysql.h>
#include <map>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif
#ifndef HP_2HEATING_ZONE_H
#include "hp2heating_zone.h"
#endif

#define SENSOR_NON	 	0
#define SENSOR_REFERENCE 	1
#define SENSOR_ZONE 		2

using namespace std;

class hp_heating_section {
	public:
		hp_heating_section(XMLNode, float, int);
		string get_id() { return my_id; }
		float get_actual_temp() { return my_ref_temp_value; }
		float get_query_temp() { return my_query_temp; }
		float get_auto_query_temp() { return my_auto_query_temp; }
		int get_heater_state() { return my_heater_state;}
		int get_cooler_state() { return my_cooler_state;}
		bool has_main_vents() { return my_has_main_vents; }
		string get_section_state();
		vector<string> get_sensors();
		const vector<string> get_main_vents() { return my_main_vents;};
		void set_actual_temp (float temp) { my_ref_temp_value= temp;}
		void set_query_temp (float temp) { my_query_temp = temp<=0?my_def_temp_value:temp;}
		void set_auto_query_temp (float temp) { my_auto_query_temp= temp<=0?my_def_temp_value:temp;}
		void set_heater_state(int state) { my_heater_state = state;}
		void set_cooler_state(int state) { my_cooler_state= state;}
		void set_temp4section(float temp, string sensor);
		int has_sensor(string name);
		int check_tempering_start(int min, int heating_mode);
		int get_tempering_start () { return this->my_tempering_start; }
		int get_tempering_time() { return this->my_tempering_time; }
		bool is_tempering_enabled() { return this->my_tempering_enabled; }
		bool has_heater(string heater);
		bool is_active_tempering() {return my_tempering_actived; } 
		void set_tempering_active (bool temp) { my_tempering_actived = temp; }
		vector<hp_heating_data_t> process_section(int mode, int eco_temp, string sensor_name = "");//, bool heating_mode_changed = false);
		vector<hp_heating_data_t> get_tempering_data();
		vector<hp_heating_data_t> check_windows(string ident, int value);
		~hp_heating_section();
	private: 
		hp_xml_parser xml_parser;
		string my_id;
		string my_direction;
		float my_def_temp_value;
		float my_heating_treshold;
		bool my_tempering_enabled; // povolenie temperovania danej sekcie
		int my_tempering_time;
		int my_tempering_start;
		bool my_tempering_actived;

		string my_ref_temp; // ident referencnej teploty
		float my_ref_temp_value; // hodnota referencnej teploty

		float my_query_temp; // pozadovana teplota sekcie
		float my_auto_query_temp; /// pozadovana teplota sekcie pri auto mode

		int my_heater_state;
		int my_cooler_state;
		bool my_has_main_vents;

		vector<string> my_main_vents;
		vector<hp2heating_zone> my_heating_zones;
};

#endif
