#ifndef HP_PIN_NOTIFIER_H 
#define HP_PIN_NOTIFIER_H 

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp_pin_notifier
{
	public:
		hp_pin_notifier(XMLNode node);
		string get_ident() { return my_ident; }
		string get_type() { return my_type; }
		string get_ip() { return my_ip; }
		string get_port() { return my_port; }
		string get_url(string );
		~hp_pin_notifier();
	private: 
		hp_xml_parser my_xml_parser;

		string my_type;
		string my_ident;
		string my_link;
		string my_ip;
		string my_port;
};

#endif
