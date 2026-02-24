#include "../include/hp_pin_status_light.h"


hp_pin_status_light::hp_pin_status_light(XMLNode node, int hbx_pos) : hp_output_pin(node, hbx_pos)
{
	//cout << "constructor hp_pin_status_light" << endl;
}


hp_mess2send_t hp_pin_status_light::create_send_mess(string value, bool timer_message)
{
	//cout <<"Pozadovana hodnota pri vytvarani spravy: " << value << " a dlzka value: " << value.length() <<  endl;
	//vector<string> res;
	string str_value = "";
	hp_mess2send_t res;
	res.timer_hold = -1;

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
		int i_value = patch::string2int(value);
		if(i_value > 10 && i_value < 250){
			str_value = value;
			res.to_value = "1";
		} else {
			return res;
		}
	}

	if(my_pin_timer != -1 && timer_message){
		if(!is_inverz() && str_value == "009"){
			if(my_pin_timer <= 2){
				str_value = patch::to_string(my_pin_timer*10+10);
			} else {
				if(my_pin_timer <= 71){
					str_value = patch::to_string(my_pin_timer+29);
				} else if(my_pin_timer <= 600){
					str_value = patch::to_string(my_pin_timer+88);
				} else {
					str_value = patch::to_string(my_pin_timer);
				}
			}
			while(str_value.length() < 3){
				str_value = add_zero(str_value);
			}
		}
		res.timer_hold = my_pin_timer;
	}

	if(my_repeat_timer != -1){
		res.service_type = "repeat_mess";
	} else {
		res.service_type = "";
	}
	res.repeat_timer = my_repeat_timer;
	res.max_triggers = my_max_triggers;
	

	res.xbee_id =patch::to_string(my_hbx_pos);
	res.uc_mess_type = "1";
	res.mess = my_id+str_value;
	res.ident = this->get_desc();

	//cout << "xbee: " << res.xbee_id << " mess: " << res.mess << " timer: " << res.timer_hold << " pre pin: " << this->get_desc() << endl;

	return res;
}

string hp_pin_status_light::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_pin_status_light::set_status(string status)
{
	if(status == "009"){
		my_status = patch::string2int(my_active_value);
	} else {
		int tmp = patch::string2int(status);
		if(tmp!= -1){
			if(tmp> 9 && tmp< 250){
				my_status = 1;
			} else {
				if(my_active_value == "1"){
					my_status = 0;
				} else {
					my_status = 1;
				}
			}
		}
		my_priorty_on = false;
	}
	my_changed = true;

	return patch::to_string(my_status);
}


hp_pin_status_light::~hp_pin_status_light() {}
