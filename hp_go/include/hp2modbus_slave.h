#ifndef HP2MODBUS_SLAVE_H
#define HP2MODBUS_SLAVE_H

#include <iostream>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp2modbus_slave{
	public:
		hp2modbus_slave(XMLNode node);
		string get_id() { return my_desc;}
		int get_address() { return my_address;}
		string get_desc_2() { return my_desc2;}
		string get_type() { return my_type;}
		int get_register_size() { return my_register_size; }
		int get_register() { return my_register; }
		string get_master_id() { return my_master_id; }
		void set_status(float status) { 
			my_status = status;
		}
		string get_status() {return patch::to_string(my_status);}
		bool run_autoread();
	//	hp2modbus_slave(const hp2modbus_slave&) = delete;
	private:
		string my_desc;
		int my_address;
		int my_register;
		string my_desc2;
		string my_type;
		int my_register_size;
		string my_master_id;
		int my_autoread_time;
		int my_autoread_last;
		bool my_autoread_send;

		float my_status;
};
#endif
