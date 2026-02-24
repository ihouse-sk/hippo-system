#include "../include/hp_advanced_scenario.h"

hp_advanced_scenario::hp_advanced_scenario(XMLNode node, string scen_ident) 
{
	my_scen_ident = scen_ident;
	for(int j=0; j<node.getChildNode("actions").nChildNode("ident"); j++){
		hp_scen_action_t tmp;
		tmp.ident = xml_parser.get_node_value(node.getChildNode("actions"), "ident", "value",j);
		tmp.to_value = xml_parser.get_node_value(node.getChildNode("actions"), "ident", "to_value",j);
		tmp.timer_time = xml_parser.get_node_value(node.getChildNode("actions"), "ident", "timer_hold",j);
		tmp.on_value= xml_parser.get_node_value(node.getChildNode("actions"), "ident", "on_value",j);
		tmp.off_value= xml_parser.get_node_value(node.getChildNode("actions"), "ident", "off_value",j);
		my_action.push_back(tmp);
	}
	for(int j=0; j<node.getChildNode("services").nChildNode("service"); j++){
		string gui_ident, section, command, to_value;
		gui_ident = xml_parser.get_node_value(node.getChildNode("services"), "service", "gui_ident",j);
		section = xml_parser.get_node_value(node.getChildNode("services"), "service", "section_id",j);
		command = xml_parser.get_node_value(node.getChildNode("services"), "service", "command",j);
		to_value = xml_parser.get_node_value(node.getChildNode("services"), "service", "to_value",j);
		if(gui_ident == "secur"){
			if(command == ""){
				my_service_mess.push_back(gui_ident+"_"+section+"_"+to_value);
			}
		}
		if(gui_ident == "temp"){
			if(command == ""){
				my_service_mess.push_back(gui_ident+"_"+section+"_"+to_value);
			} else {
				my_service_mess.push_back(gui_ident+"_"+section+"_"+command+"_"+to_value);
			}
		}
	}
	for(int j=0; j<node.getChildNode("shutters").nChildNode("shutter"); j++){
		string ident, command;
		ident = xml_parser.get_node_value(node.getChildNode("shutters"), "shutter", "ident",j);
		command = xml_parser.get_node_value(node.getChildNode("shutters"), "shutter", "direction",j);
		if(command.find("all") != std::string::npos){
			my_shutters_mess.push_back("shut_"+ident+"_"+command+"_1");
		} else{
			my_shutters_mess.push_back("shut_"+ident+"_"+command+"_5");
		}
	}
	//cout << "Scen ident: " << my_scen_ident << endl;

	//print_actions();
}


void hp_advanced_scenario::print_actions()
{
	for(unsigned int i=0; i<my_action.size(); i++){
		cout << " ident: " << my_action[i].ident << " to_value: " << my_action[i].to_value << " timer hold: " << my_action[i].timer_time << endl;
	}
	for(unsigned int i=0; i<my_service_mess.size(); i++){
		cout <<"servise mess: " << my_service_mess[i] << endl;
	}
	for(unsigned int i=0; i<my_shutters_mess.size(); i++){
		cout <<"shutter mess: " << my_shutters_mess[i] << endl;
	}
}
hp_advanced_scenario::~hp_advanced_scenario() {}
