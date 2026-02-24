#ifndef HP_OUTPUT_PIN_H
#define HP_OUTPUT_PIN_H

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

class hp_output_pin : public hp_virtual_pin
{
	public:
		hp_output_pin(XMLNode, int hbx_pos);
		virtual hp_mess2send_t create_send_mess(string value, bool timer_message = false) = 0;

		bool check_periodicity() {return false;};
		bool check_input(string input_type);
		void setup_input_rules(bool short_pushed, bool long_pushed, bool doube_pushed, bool shutter_control = false);
		~hp_output_pin();
	protected:
		string add_zero(string str);
	private: 
		hp_xml_parser xml_parser;

};

#endif
