#include "../include/hp_pin_heater.h"


hp_pin_heater::hp_pin_heater(XMLNode node, int hbx_pos) : hp_output_pin(node, hbx_pos)
{
	//cout << "constructor hp_pin_heater" << endl;
}


hp_mess2send_t hp_pin_heater::create_send_mess(string value, bool timer_message)
{
	//cout <<"Pozadovana hodnota pri vytvarani spravy: " << value << " a dlzka value: " << value.length() <<  endl;
	//vector<string> res;
	string str_value = "";
	hp_mess2send_t res;
	res.timer_hold = -1;
	res.repeat_timer = -1;

	if(value == "change"){
		if(my_status == 0){
			res.to_value = "1";
			str_value = my_active_value=="1"?"009":"255";
			//str_value = "009";
		} else {
			res.to_value = "0";
			str_value = my_active_value=="1"?"255":"009";
		}
	} else if(value == "1"){
		str_value = my_active_value=="1"?"009":"255";
		res.to_value = "1";
	} else if(value == "0"){
		str_value = my_active_value=="1"?"255":"009";
		res.to_value = "0";
	} else { 
		return res;
	}

	if(my_pin_timer != -1 && timer_message && my_pin_timer != 0){
		res.timer_hold = my_pin_timer;
	}

	res.xbee_id =patch::to_string(my_hbx_pos);
	res.uc_mess_type = "1";
	res.mess = my_id+str_value;
	res.ident = this->get_desc();
	res.service_type = "";

	//cout << "xbee: " << res.xbee_id << " mess: " << res.mess << " timer: " << res.timer_hold << endl;

	return res;
}

string hp_pin_heater::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_pin_heater::set_status(string status)
{
	//cout << "Set status to : " << status << endl;
	if(status == "009"){
		my_status = patch::string2int(my_active_value);
	} else {
		if(my_active_value == "1"){
			my_status = 0;
		} else {
			my_status = 1;
		}
	}
	my_changed = true;

	return patch::to_string(my_status);
}


hp_pin_heater::~hp_pin_heater() {}
