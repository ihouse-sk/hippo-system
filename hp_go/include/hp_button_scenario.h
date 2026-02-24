#ifndef HP_BUTTON_SCENARIO_H
#define HP_BUTTON_SCENARIO_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_ADVANCED_SCENARIO_H
#include "hp_advanced_scenario.h"
#endif

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp_button_scenario : public hp_advanced_scenario
{
	public:
		hp_button_scenario(XMLNode node, string scen_ident, string scen_label);
		~hp_button_scenario();
	private: 
		string my_scen_label;
};

#endif
