#include "../include/hp_pin_pwm.h"


hp_pin_pwm::hp_pin_pwm(XMLNode node, int hbx_pos) : hp_output_pin(node, hbx_pos)
{
	//cout << "constructor hp_pin_pwm" << endl;
	my_last_direction = "down";
	my_status = 5;
}


hp_mess2send_t hp_pin_pwm::create_send_mess(string value, bool timer_message)
{
	hp_mess2send_t res;
	string str_value = "";
	res.timer_hold = -1;

	if(value == "change"){
		if(my_status == 0){
			str_value = my_active_value=="1"?"009":"255";
			res.to_value = "9";
			//str_value = "009";
		} else {
			str_value = my_active_value=="1"?"255":"009";
			res.to_value = "0";
		}
	} else if(value == "9"){
		str_value = my_active_value=="1"?"009":"255";
		res.to_value = "9";
	} else if(value == "0"){
		str_value = my_active_value=="1"?"255":"009";
		res.to_value = "0";
	} else if( value == "fluently"){
		res.to_value = "fluently";
		if(my_last_direction == "down"){
			if(my_status < 9){
				str_value = "00" +patch::to_string( my_status+1);
			} else {
				str_value = "STOP";
			}
		} else if( my_last_direction == "up"){
			if(my_status > 1){
				str_value = "00" +patch::to_string( my_status-1);
			} else if(my_status == 0 || my_status == 1) {
				str_value = "255";
			} else {
				str_value = "STOP";
			}
		}
	} else { 
		res.to_value = value;
		if(my_active_value == "1"){
			if(atoi(value.c_str()) > 0 && atoi(value.c_str()) < 9){
				str_value = "00"+value;
			}
		} else {
			if(atoi(value.c_str()) > 0 && atoi(value.c_str()) < 9){
				str_value = "00"+patch::to_string(9-atoi(value.c_str()));
			}
		}
	}

	if(my_pin_timer != -1 && timer_message){
		res.timer_hold = my_pin_timer;
	}

	res.xbee_id =patch::to_string(my_hbx_pos);
	res.uc_mess_type = "1";
	res.mess = my_id+str_value;
	//res.to_value = value;
	res.last_direction = my_last_direction=="up"?"down":"up";
	res.service_type = "lightning";
	res.ident = this->get_desc();
	//cout << "Vytvaram spravu: " << res.mess << " value: " << str_value << " pre pin: " << this->get_desc() << " timer messs: " << timer_message << " pozadovana hodnota: " << value << endl;

	return res;
}

string hp_pin_pwm::get_status()
{
	string res = "";

	return patch::to_string(my_status);
}

string hp_pin_pwm::set_status(string status)
{
	//cout <<"Setting status to: " << status << endl;
	if(status != "255"){
		if(my_status < atoi(status.c_str())){
			my_last_direction = "up";
		} else {
			my_last_direction = "down";
		}
	} else {
		my_last_direction = "down";
	}
	if(status == "255"){
		my_status = 0;
		my_priorty_on = false;
	} else {
		my_status = atoi(status.c_str());
	}
	my_changed = true;
	return patch::to_string(my_status);
}


hp_pin_pwm::~hp_pin_pwm() {}
