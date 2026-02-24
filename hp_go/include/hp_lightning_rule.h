#ifndef HP_LIGHTNING_RULE_H
#define HP_LIGHTNING_RULE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <stdio.h>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_RULE_LIGHT_S
#define HP_RULE_LIGHT_S
typedef struct {
	string ident;
	string to_value;
	string timer_time;
	string timer_type;
	string my_pwm_on;
	string my_pwm_off;
} hp_rule_light_t;
#endif

using namespace std;

class hp_lightning_rule {
	public:
		hp_lightning_rule(XMLNode);
		string get_actor() { return my_actor; }
		string get_hold() { return my_hold; }
		string get_on_value() { return my_on_value; }
		int get_pushed() { return my_pushed; }
		bool has_time_rest() { return my_time_rest; }
		bool get_rule_priority() { return my_rule_priority; }
		bool is_valid();
		vector<hp_rule_light_t> get_outputs () { return my_outputs;}
		~hp_lightning_rule();
	private: 
		string my_actor;
		int my_actor_pos;
		int my_pushed;
		int my_start_time_rest;
		int my_end_time_rest;
		bool my_time_rest;
		bool my_rule_priority;
		string my_hold;
		string my_on_value;
		vector<hp_rule_light_t> my_outputs;
		hp_xml_parser xml_parser;
};

#endif
