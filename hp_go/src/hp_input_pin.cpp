#include "../include/hp_input_pin.h"

hp_input_pin::hp_input_pin(XMLNode node, int hbx_pos) : hp_virtual_pin(node, hbx_pos)
{
	string tmp = xml_parser.get_node_value(node,"pin_type","periodic_check");
	if(tmp != ""){
		my_periodic_check = patch::string2int(tmp);
		my_periodic_counter = my_periodic_check-1;
	} else {
		my_periodic_check = -1;
		my_periodic_counter = 0;
	}
}

bool hp_input_pin::check_periodicity()
{
	if(my_periodic_counter++ > my_periodic_check){
		my_periodic_counter = 0;
		return true;
	} else {
		return false;
	}
}

bool hp_input_pin::check_input(string input_type)
{
	if(input_type == "normal") {
		if(!my_shord_pushed && !my_long_pushed && !my_double_pushed){
			return true;
		} else {
			return false;
		}
	}
	if(input_type == "short") {
		if(my_shord_pushed && !my_long_pushed){
			return true;
		} else {
			return false;
		}
	}
	if(input_type == "long"){
		return my_long_pushed;
	}
	if(input_type == "shutter"){
		if(my_shutter_control){
			return true;
		}
	}

	if(input_type == "double"){
		return my_double_pushed;
	}

	return false;
}

void hp_input_pin::setup_input_rules(bool short_pushed, bool long_pushed, bool doube_pushed, bool shutter_control )
{
	//cout << "Nastavujem rule pre : " << get_desc()<< ", : " << short_pushed << " " << long_pushed << " " << doube_pushed << endl;
	my_shord_pushed = short_pushed;
	my_long_pushed = long_pushed;
	my_double_pushed = doube_pushed;
	my_shutter_control = shutter_control;
}
		

hp_mess2send_t hp_input_pin::create_send_mess(string value, bool timer_message)
{
	hp_mess2send_t res;
	if(timer_message){
		value = "";
	}
	
	res.xbee_id =patch::to_string(my_hbx_pos);
	res.uc_mess_type = UC_CHECK_PORTS;
	res.mess = my_id+"0";
	res.ident = this->get_desc();
	return res;
}


hp_input_pin::~hp_input_pin() {}
