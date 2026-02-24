#include "../include/hp_sensor_pin.h"

hp_sensor_pin::hp_sensor_pin(XMLNode node, int hbx_pos) : hp_virtual_pin(node, hbx_pos)
{
	my_periodic_check = -1;
}

bool hp_sensor_pin::check_input(string input_type)
{
	return false;
}

void hp_sensor_pin::setup_input_rules(bool short_pushed, bool long_pushed, bool doube_pushed, bool shutter_control )
{
	//cout << "Nastavujem rule pre : " << get_desc()<< ", : " << short_pushed << " " << long_pushed << " " << doube_pushed << endl;
	my_shord_pushed = short_pushed;
	my_long_pushed = long_pushed;
	my_double_pushed = doube_pushed;
	my_shutter_control = shutter_control;
}
		

hp_mess2send_t  hp_sensor_pin::create_send_mess(string value, bool timer_message)
{
	hp_mess2send_t tmp;

	return tmp;
}


hp_sensor_pin::~hp_sensor_pin() {}
