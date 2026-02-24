#ifndef HP_LIGHTNING_H
#define HP_LIGHTNING_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <utility>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_LIGHTNING_RULE_H
#include "hp_lightning_rule.h"
#endif

using namespace std;

class hp_lightning {
	public:
		hp_lightning(XMLNode);
		hp_lightning_rule *find_rule(string actor, int pushed, string hold, string on_value = "");
		vector<bool> get_rules4pin(string desc, string mode = "normal");
		string get_dn_ident() { return my_gui_id; }
		string get_actual_mode() { return my_actual_mode; }
		string get_dn_type() { return this->my_dn_type; }
		int get_actual_int_mode();
		void set_actual_mode(string mode); //{ my_actual_mode = mode;} 
		std::pair<string,string> check_dn_change(string ident ="", int value = 0);
		~hp_lightning();
	private: 
		hp_xml_parser xml_parser;
		vector<hp_lightning_rule> my_normal_rules;
		vector<hp_lightning_rule> my_day_rules;
		vector<hp_lightning_rule> my_night_rules;
		string my_id;
		string my_gui_id;
		string my_actual_mode;
		bool my_dn_active;
		string my_dn_type;
		string my_dn_actor_ident;
		int my_night_start;
		int my_night_end;
		int my_day_start;
		int my_day_end;
		int my_day_value;
		int my_night_value;
		int my_dn_array[3600];
};

#endif
