#include "../include/hp2heating_zone.h"

hp2heating_zone::hp2heating_zone(XMLNode node, float def_temp)
{
	my_sensor_ident= xml_parser.get_node_value(node, "sensor","ident");
	for(int i=0; i< node.nChildNode("actor"); i++){
		this->my_heaters.push_back(xml_parser.get_node_value(node,"actor","ident",i));
	}
	for(int i=0; i<node.nChildNode("cooler"); i++){
		my_coolers.push_back(xml_parser.get_node_value(node,"cooler","ident",i));
	}
	for(int i=0; i<node.nChildNode("window"); i++){
		my_windows.emplace(std::make_pair<string,int>(xml_parser.get_node_value(node,"window","ident",i),patch::string2int(xml_parser.get_node_value(node,"window","closed",i))));
	}
	my_direction = "up";
	my_heater_state = 0;
	my_cooler_state = 0;
	my_blocked = false;
	this->my_actual_temp = def_temp;
}

bool hp2heating_zone::set_blocked(string ident, int state)
{
	bool res = false;
	auto it = my_windows.find(ident);
	if(it != my_windows.end()){
		bool tmp_block = my_blocked;
		my_blocked = state == it->second?false:true;
		if(tmp_block != my_blocked){
			res = true;
		}
	}
	return res;
}

hp2heating_zone::~hp2heating_zone() {}
