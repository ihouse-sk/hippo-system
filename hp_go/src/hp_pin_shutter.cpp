#include "../include/hp_pin_shutter.h"


hp_pin_shutter::hp_pin_shutter(XMLNode node, int hbx_pos) : hp_output_pin(node, hbx_pos)
{
	//cout << "constructor hp_pin_shutter" << endl;
}


hp_mess2send_t hp_pin_shutter::create_send_mess(string value, bool timer_message)
{
	//vector<string> res;
	string str_value = "";
	hp_mess2send_t res;

	 if(value == "1"){
		str_value = my_active_value=="1"?"009":"255";
	} else if(value == "0"){
		str_value = my_active_value=="1"?"255":"009";
	} else if(value == "down"){
		str_value = my_active_value=="1"?"255":"009";
		//str_value = "255";
	} else if(value == "up"){
		str_value = my_active_value=="1"?"009":"255";
		//str_value = "009";
	} else {
		if(this->get_desc() == "shuttPracovnaOnOff" ||this->get_desc() == "shuttHostOnOff" ||this->get_desc() == "shuttPracOnOff") {
			value = "150";
		}
		int ivalue = patch::string2int(value);
		if(ivalue != -1){
			str_value = patch::to_string(10+ivalue/10);
			str_value = add_zero(str_value);
		}
	}

	res.xbee_id =patch::to_string(my_hbx_pos);
	res.uc_mess_type = "1";
	res.mess = my_id+str_value;
	res.service_type = "shutter";
	res.timer_hold = my_pin_timer;
	res.ident = this->get_desc();

	return res;
}

string hp_pin_shutter::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_pin_shutter::set_status(string status)
{
	//cout << "Set status to : " << status << endl;
	if(status == "255" ){
		my_status = 0;
	} else if (status == "009"){
		my_status = 1;
	}
	my_changed = true;
	return patch::to_string(my_status);
}


hp_pin_shutter::~hp_pin_shutter() {}
