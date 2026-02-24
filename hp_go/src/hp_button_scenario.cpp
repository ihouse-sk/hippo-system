#include "../include/hp_button_scenario.h"

hp_button_scenario::hp_button_scenario(XMLNode node, string scen_ident, string scen_label): hp_advanced_scenario(node, scen_ident)
{
	my_scen_label = scen_label;
	//print_actions();
}

hp_button_scenario::~hp_button_scenario() {}
