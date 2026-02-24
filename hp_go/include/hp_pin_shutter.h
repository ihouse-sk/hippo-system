#ifndef HP_PIN_SHUTTER_H
#define HP_PIN_SHUTTER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_OUTPUT_PIN_H
#include "hp_output_pin.h"
#endif

using namespace std;

class hp_pin_shutter : public hp_output_pin
{
	public:
		hp_pin_shutter(XMLNode, int hbx_pos);
		
		hp_mess2send_t create_send_mess(string value, bool timer_message =false);
		string set_status(string status) ;
		string get_status();
		~hp_pin_shutter();
};

#endif
