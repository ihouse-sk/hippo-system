#ifndef HP2MODBUS_DATA_H
#define HP2MODBUS_DATA_H

#include <vector>
#include <iostream>

#define BUF_MAX 100

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp2modbus_data {
	public:
		hp2modbus_data(string desc = "", string value= "", int mess_id = 0, string status = "");
		string get_desc() { return my_slave_desc;}
		string get_value() { return my_value;}
		string get_status() { return my_status;}
		void print_data() {
			cout <<"Desc: " << my_slave_desc << ", value: " << my_value << ", status: " << my_status << endl;
		}
	private:
		string my_slave_desc;
		string my_value;
		string my_status;
		int my_mess_id;

};
#endif
