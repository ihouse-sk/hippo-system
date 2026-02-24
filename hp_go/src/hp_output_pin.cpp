#include "../include/hp_output_pin.h"

hp_output_pin::hp_output_pin(XMLNode node, int hbx_pos) : hp_virtual_pin(node, hbx_pos)
{
	my_periodic_check = -1;
}
bool hp_output_pin::check_input(string input_type)
{
	return false;
}

string hp_output_pin::add_zero(string str)
{
	string res ="";
	while(res.size() + str.size() < 3){
		res.append("0");	
	}	
	return res+str;
}


void hp_output_pin::setup_input_rules(bool short_pushed, bool long_pushed, bool doube_pushed, bool shutter_control)
{

}

hp_output_pin::~hp_output_pin() {}
