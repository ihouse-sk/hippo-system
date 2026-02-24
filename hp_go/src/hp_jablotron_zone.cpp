#include "../include/hp_jablotron_zone.h"

hp_jablotron_zone::hp_jablotron_zone(XMLNode node)
{
	my_impulz_control = false;
	my_ident= xml_parser.get_node_value(node,"ident");
	my_actor_ident= xml_parser.get_node_value(node,"actor");
	my_state_ident = xml_parser.get_node_value(node,"state");
	my_on_ident= xml_parser.get_node_value(node,"actor_on");
	my_off_ident= xml_parser.get_node_value(node,"actor_off");
	my_impulz_control = xml_parser.get_node_value(node,"impulz_control")=="yes"?true:false;
	my_sec_state = 0;
	my_wait_time = 10;
}

hp_jablotron_zone::~hp_jablotron_zone() {}
