#ifndef HP_ADVANCED_SCENARIO_H
#define HP_ADVANCED_SCENARIO_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_SCEN_ACTION_S
#define HP_SCEN_ACTION_S
typedef struct {
	string ident;
	string to_value;
	string timer_time;
	string on_value;
	string off_value;
} hp_scen_action_t;
#endif

using namespace std;

class hp_advanced_scenario {
	public:
		hp_advanced_scenario(XMLNode node, string scen_ident);
		string get_scen_ident() { return my_scen_ident; }
		vector<hp_scen_action_t> get_actions() { return my_action; }
		vector<string> get_services() { return my_service_mess; }
		vector<string> get_shutters() { return my_shutters_mess; }
		void print_actions();
		~hp_advanced_scenario();
	protected: 
		hp_xml_parser xml_parser;
		string my_scen_ident;
		vector<hp_scen_action_t> my_action;
		vector<string> my_service_mess;
		vector<string> my_shutters_mess;
};

#endif
