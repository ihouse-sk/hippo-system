
#include "../include/hp_pin_cons.h"


hp_pin_cons::hp_pin_cons(XMLNode node, int hbx_pos) : hp_input_pin(node, hbx_pos)
{
	my_impulz_factor = patch::string2float(xml_parser.get_node_value(node, "pin_type", "impulz_1"));
	if(my_impulz_factor == -1 || my_impulz_factor == 0){
		my_impulz_factor = 1;
	}
}


string hp_pin_cons::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_pin_cons::set_status(string status)
{
	int value = patch::string2int(status);
	//int act_value = value*(3600/CONS_INTERVAL);
	my_status = value*my_impulz_factor*(3600/CONS_INTERVAL);

	string res =  patch::to_string(my_status);
	my_changed = true;
	//cout << "status: " << status << ", res: " << res << ", factor: " << my_impulz_factor <<  endl; 

	return res;
}


hp_pin_cons::~hp_pin_cons() {}
