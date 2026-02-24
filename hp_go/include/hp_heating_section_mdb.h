#ifndef HP_HEATING_SECTION_MDB_H
#define HP_HEATING_SECTION_MDB_H

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

using namespace std;

class hp_heating_section_mdb {
	public:
		hp_heating_section_mdb(XMLNode);
		string get_id() { return my_id; }
		string get_master_name() { return my_master_name; }
		float get_actual_temp() { return my_ref_temp_value; }
		float get_query_temp() { return my_query_temp; }
		float get_auto_query_temp() { return my_auto_query_temp; }
		int get_heater_state() { return my_heater_state;}
		int get_cooler_state() { return my_cooler_state;}
		string get_section_state();
		void set_actual_temp (float temp) { my_ref_temp_value= temp;}
		void set_query_temp (float temp) { my_query_temp = temp<=0?my_def_temp_value:temp;}
		void set_auto_query_temp (float temp) { my_auto_query_temp= temp<=0?my_def_temp_value:temp;}
		void set_temp4section(float temp);
		void set_master_pos(int pos) { my_master_pos = pos;}
		int get_master_pos() { return my_master_pos;}
		uint16_t get_read_address() { return my_read_address; }
		int get_funct_code() { return my_funct_code; }
		int get_coils_num() { return my_read_coils_num; }
		//vector<hp_heating_data_t> process_section(int mode, int eco_temp, string sensor_name = "");//, bool heating_mode_changed = false);
		//vector<hp_heating_data_t> get_tempering_data();
		~hp_heating_section_mdb();
	private: 
		hp_xml_parser xml_parser;
		
		string my_master_name;
		int my_master_pos;


		string my_id;
		float my_def_temp_value;

		float my_ref_temp_value; // hodnota referencnej teploty

		float my_query_temp; // pozadovana teplota sekcie
		float my_auto_query_temp; /// pozadovana teplota sekcie pri auto mode

		int my_heater_state;
		int my_cooler_state;
		
		//////read registers
		uint16_t my_read_address;
		int my_funct_code;
		int my_read_coils_num;
};

#endif
