#ifndef HP2MODBUS_DEVICES_H
#define HP2MODBUS_DEVICES_H

#include <iostream>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp2modbus_devices{
	public:
		hp2modbus_devices(XMLNode node);
		string get_id() { return my_id;}
		string get_ip() { return my_ip;}
		int get_port() { return my_port;}
		string get_type() { return my_type;}
	private:
		string my_ip;
		string my_id;
		int my_port;
		string my_type;
	
};
#endif
