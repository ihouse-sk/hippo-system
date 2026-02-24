#ifndef HP_REPEAT_MESS_H
#define HP_REPEAT_MESS_H

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

class hp_repeat_mess {
	public:
		hp_repeat_mess(hp_mess2send_t data,int send_periodicity, string direction, int pin_pos, int token = -1);
		string get_mess() { return my_mess;}
		string get_xbee_id() { return my_xbee_id;}
		string get_mess_type() { return my_mess_type;}
		int get_pin_pos() { return my_pin_pos; }
		int get_token() { return my_life_token; }
		string get_service_type() { return my_service_type;}
		bool check_periodicity();
		bool check_life_token();
		hp_mess2send_t create_mess();
		~hp_repeat_mess();
	private:
		string my_mess;
		string my_uc_mess;
		string my_xbee_id;
		string my_mess_type;
		string my_service_type;
		int my_min_value;
		int my_periodicity;
		string my_direction;
		string my_source_mess;
		int my_pin_pos;
		int my_life_token;
		int my_counter;
};

#endif
