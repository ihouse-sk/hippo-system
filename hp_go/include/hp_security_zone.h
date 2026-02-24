#ifndef HP_SECURITY_ZONE_H
#define HP_SECURITY_ZONE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp_security_zone {
	public:
		hp_security_zone(XMLNode);
		string get_id() { return my_ident;}
		int init_countdown() { return my_countdown_counter = my_wait_time; }
		int get_wait_time() { return my_wait_time; }
		int get_current_countdown() { return my_countdown_counter; }
		int get_sec_status() { return my_sec_status; }
		int decrease_counter();
		bool repeat_action();
		vector<string> get_actors() { return my_actors; }
		vector<string> get_sensors() { return my_sensors; }
		bool has_sensor(string sensor);
		void disable_repead(int sec) {
			my_next_repeat = time(NULL)+sec;
		}
		void set_sec_status(int status) { 
			my_sec_status = status; 
			if(my_sec_status == 0){
				my_repeated_counter = 0;
				my_last_check_time = time(NULL)-1000;
			}
			my_countdown_counter = -1;
		} 

		~hp_security_zone();
	private: 
		hp_xml_parser xml_parser;
		string my_ident;
		string my_actor;
		int my_countdown_counter;
		int my_wait_time;
		vector<string> my_actors;
		vector<string> my_sensors;
		int my_sec_status;
		bool my_repeated_action;
		int my_repeat_triggers;
		int my_repeated_counter;

		bool my_status;
		time_t my_last_check_time;
		time_t my_next_repeat;
};

#endif
