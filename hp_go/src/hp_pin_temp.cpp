#include "../include/hp_pin_temp.h"

hp_pin_temp::hp_pin_temp(XMLNode node, int hbx_pos) : hp_sensor_pin(node, hbx_pos)
{
	my_offset = patch::string2float(xml_parser.get_node_value(node,"pin_type","offset"));
}
string hp_pin_temp::get_status()
{
	return my_temp_str;
}

string hp_pin_temp::set_status(string status)
{
//	cout <<get_desc()<< ", setting temp to: " << status << endl;
	float f_status = patch::string2float(status);
	my_temp = f_status + my_offset;
	my_temp_str = patch::to_string(my_temp);

	return my_temp_str;
}

hp_pin_temp::~hp_pin_temp() {}
