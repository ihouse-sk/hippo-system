#include "../include/hp_repeat_mess.h"

hp_repeat_mess::hp_repeat_mess(hp_mess2send_t data,int send_periodicity, string direction, int pin_pos, int token) : my_uc_mess(data.mess), my_xbee_id(data.xbee_id), my_mess_type(data.uc_mess_type), my_periodicity(send_periodicity), my_direction(direction), my_pin_pos(pin_pos), my_life_token(token)
{
	my_mess = data.mess;
	my_service_type = data.service_type;
	my_min_value = 1;
	my_counter = 0;
}

bool hp_repeat_mess::check_periodicity()
{
	my_counter++;
	if(my_counter == my_periodicity){
		my_counter = 0;
		return true;
	}
	return false;
}
bool hp_repeat_mess::check_life_token()
{
	//cout <<"Life token: " << my_life_token << endl;
	if(my_life_token > 0){
		my_life_token--;
		return true;
	} else {
		if(my_life_token == -1){
			return true;
		} else {
			return false;
		}
	}
}

hp_mess2send_t hp_repeat_mess::create_mess()
{
	hp_mess2send_t res;

	res.xbee_id = my_xbee_id;
	res.uc_mess_type = my_mess_type;
	if(my_service_type == "lightning"){
		//cout << "Message: " << my_mess << endl;
		string last_value;
		if(my_mess.find("255") != std::string::npos){
			last_value = "0";
		} else {
			last_value = my_mess.substr(my_mess.length()-1);
		}
		my_mess =my_mess.substr(0,my_mess.length()-1);
		int value = atoi(last_value.c_str());
		if(my_direction == "up"){
			if(value < 9) {
				value++;
				my_mess+=patch::to_string(value);
			} else {
				my_mess+="STOP";
			}
		} else {
		//	cout << "generujem repeat message, value: " << value << endl;
			if(value > my_min_value && value < 10){
				value--;
				my_mess+=patch::to_string(value);
			} else if (value == my_min_value) {
				my_mess = my_mess.substr(0,2) + "255";
			} else {
				my_mess+="STOP";
			}
		}
		res.mess = my_mess; 
	} else if (my_service_type  == "shutter"){
		if(my_life_token > 0){
			res.mess = my_mess;
			my_life_token--;
		} else {
			res.mess = "STOP";
		}
	} else if (my_service_type == "repeat_mess"){
		res.to_value = "1";
		res.mess = my_mess; 
	}

	return res;
}

hp_repeat_mess::~hp_repeat_mess() {}
