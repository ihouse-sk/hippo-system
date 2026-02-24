#ifndef HP_JABLOTRON_ZONE_H
#define HP_JABLOTRON_ZONE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp_jablotron_zone {
	public:
		hp_jablotron_zone(XMLNode);
		string get_ident() { return my_ident;}
		string get_actor_ident() { return my_actor_ident;}
		string get_state_ident() { return my_state_ident;}
		string get_on_ident() { return my_on_ident;}
		string get_off_ident() { return my_off_ident;}
		bool has_impulz_control() { return my_impulz_control;}
		int get_wait_time() { return my_wait_time; }
		int get_zone_status() { return my_sec_state; }
		void set_zone_status(int sec_state) { my_sec_state = sec_state; }
		~hp_jablotron_zone();
	private: 
		hp_xml_parser xml_parser;
		string my_ident;
		string my_actor_ident;
		string my_on_ident;
		string my_off_ident;
		string my_state_ident;
		int my_sec_state;
		int my_wait_time;
		bool my_impulz_control;
};

#endif
