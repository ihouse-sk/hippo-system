#include "../include/hp_mess_data.h"

hp_mess_data::hp_mess_data(hp_mess2send_t data,int resend_count, int priority, int delay) :  my_counter(1), my_priority(priority), my_delay(delay)
{
	if(data.uc_mess_type == "0"){
		my_mess = data.mess;
	} else {
		my_mess = data.uc_mess_type + data.mess;
	}
	this->my_xbee_id = data.xbee_id;
	this->my_mess_type = data.uc_mess_type;
	this->my_uc_mess = data.mess;
	this->my_ident = data.ident;

	my_resend_count = resend_count;
	if(my_resend_count != 0){
		my_priority = 1;
	}

	my_debug = data.debug;
	my_postpone_count = 0;
}

bool hp_mess_data::postpone_avaible()
{
	if(++my_postpone_count > MAX_POSTPONE_COUNT){
		return false;
	}
	return true;
}
bool hp_mess_data::check_message(string uc_mess)
{
	if(my_uc_mess.find(uc_mess.substr(0,2)) == std::string::npos){
		return false;
	} else {
		return true;
	}
}

bool hp_mess_data::add_message(string uc_mess)
{
	if(my_counter < MAX_MESS_COUNTER && my_uc_mess.find(uc_mess.substr(0,2)) == std::string::npos){
		my_counter++;
		my_uc_mess += uc_mess;
		my_mess = my_mess_type + my_uc_mess;
		return true;
	}
	return false;
}

void hp_mess_data::set_delay(int delay)
{
	if(delay >= 0){
		my_delay = delay;
	}
}

int hp_mess_data::cout_delay()
{
	if(my_delay > 0){
		my_delay--;
	}
	return my_delay;
}


hp_mess_data::~hp_mess_data() {}
