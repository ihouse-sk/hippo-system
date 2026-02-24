#include "../include/hp_push_button.h"


hp_push_button::hp_push_button(XMLNode node, int hbx_pos) : hp_input_pin(node, hbx_pos)
{
	//cout << "constructor hp_push_button" << endl;
}


string hp_push_button::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_push_button::set_status(string status)
{
	if(status == my_active_value){
		my_status = 1;
	} else {
		my_status = 0;
	} 
	my_changed = true;
	return patch::to_string(my_status);
}


hp_push_button::~hp_push_button() {}
