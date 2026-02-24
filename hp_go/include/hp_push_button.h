#ifndef HP_PUSH_BUTTON_H
#define HP_PUSH_BUTTON_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_INPUT_PIN_H
#include "hp_input_pin.h"
#endif

using namespace std;

class hp_push_button : public hp_input_pin
{
	public:
		hp_push_button(XMLNode, int hbx_pos);
		
		string set_status(string status) ;
		string get_status();
		~hp_push_button();
	private: 
};

#endif
