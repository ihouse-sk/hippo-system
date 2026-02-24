#ifndef HP_PIN_TEMP_H
#define HP_PIN_TEMP_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_SENSOR_PIN_H
#include "hp_sensor_pin.h"
#endif

using namespace std;

class hp_pin_temp : public hp_sensor_pin
{
	public:
		hp_pin_temp(XMLNode, int hbx_pos);
		string set_status(string status) ;
		string get_status();
		~hp_pin_temp();
	protected:
	private: 
		string my_temp_str;
		float my_temp;
		hp_xml_parser xml_parser;
		float my_offset;
};

#endif
