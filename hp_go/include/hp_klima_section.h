#ifndef HP_KLIMA_SECTION_H
#define HP_KLIMA_SECTION_H

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

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

using namespace std;

class hp_klima_section {
	public:
		hp_klima_section(XMLNode);
		string get_ident() { return my_ident; }
		string get_klima_ident() { return my_klima_ident; }
		string get_modbus_ip() { return my_modbus_ip;}
		int get_modbus_port() { return my_modbus_port;}
		float get_actual_temp() { return my_actual_temp; }
		float get_query_temp() { return my_query_temp; }
		int get_state() { return my_state;}
		string get_section_state();
		string get_sensor() { return my_temp_ident;}
		void set_actual_temp (float temp) { my_actual_temp= temp;}
		void set_query_temp (float temp) { my_query_temp = temp<=0?my_def_temp_value:temp;}
		void set_state(int state) { my_state = state;}
		void set_fan_speed(int tmp) { my_fan_speed= tmp;}
		void set_mode(int tmp) { my_mode= tmp;}
		void set_louvre(int tmp) { my_louvre= tmp;}
		void set_force_thermo(int tmp) { my_force_thermo= tmp;}
		int has_sensor(string name);
		~hp_klima_section();
	private: 
		void push_db_query(string query, int type = DB_STATUSES_QUERY, int log_level = 0);
		vector<string> parse_response(string resp, string separator);

		string my_ident;
		string my_klima_ident;
		string my_temp_ident;
		float my_def_temp_value;
		string my_modbus_ip;
		float my_modbus_port;
		int my_running;
		int my_fan_speed;
		int my_mode;
		int my_louvre;
		int my_force_thermo;
		

		float my_query_temp; // pozadovana teplota sekcie
		float my_actual_temp;

		int my_state;

		hp_db_data_t *my_db_data;
		hp_xml_parser xml_parser;
};

#endif
