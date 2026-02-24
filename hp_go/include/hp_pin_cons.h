#ifndef HP_PIN_CONS_H
#define HP_PIN_CONS_H

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

#define CONS_INTERVAL 300

using namespace std;

class hp_pin_cons : public hp_input_pin
{
	public:
		hp_pin_cons(XMLNode, int hbx_pos);
		
		string set_status(string status) ;
		string get_status();
		~hp_pin_cons();
	private: 
		float my_impulz_factor;
};

#endif
