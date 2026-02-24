
#include "../include/hp_onOffButton.h"


hp_onOffButton::hp_onOffButton(XMLNode node, int hbx_pos) : hp_input_pin(node, hbx_pos)
{
	//cout << "constructor hp_onOffButton" << endl;
}


string hp_onOffButton::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_onOffButton::set_status(string status)
{
	//cout <<"Setting status to: " << status << "pre pin: " << this->get_desc() << " active; " << this->get_active_value() << endl;
	if(status == my_active_value){
		my_status = 1;
	} else {
		my_status = 0;
	} 
	my_changed = true;
	return patch::to_string(my_status);
	/*
	my_status = patch::string2int(status);
	return patch::to_string(my_status);
	*/
}


hp_onOffButton::~hp_onOffButton() {}
