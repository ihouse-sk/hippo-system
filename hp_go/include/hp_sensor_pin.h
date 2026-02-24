#ifndef HP_SENSOR_PIN_H
#define HP_SENSOR_PIN_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_VIRTUAL_PIN_H
#include "hp_virtual_pin.h"
#endif

using namespace std;

class hp_sensor_pin : public hp_virtual_pin
{
	public:
		hp_sensor_pin(XMLNode, int hbx_pos);
		hp_mess2send_t create_send_mess(string value, bool timer_message = false);
		bool check_periodicity() {return false;};
		bool check_input(string input_type);
		void setup_input_rules(bool short_pushed, bool long_pushed, bool doube_pushed, bool shutter_control = false);
		~hp_sensor_pin();
	protected:
		bool my_shord_pushed;
		bool my_long_pushed;
		bool my_double_pushed;
		bool my_shutter_control;
	private: 
		hp_xml_parser xml_parser;

};

#endif
