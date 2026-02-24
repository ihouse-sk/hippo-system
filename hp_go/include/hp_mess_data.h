#ifndef HP_MESS_DATA_H
#define HP_MESS_DATA_H

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

#define DELAY_TIME 10000  // delay v hp_go v us

#define MAX_MESS_COUNTER 	6
#define MAX_POSTPONE_COUNT	20

using namespace std;

class hp_mess_data {
	public:
		hp_mess_data(hp_mess2send_t data,int resend_count, int priority = 0, int delay = 0);
		string get_mess() {  return my_mess;}
		string get_xbee_id() { return my_xbee_id;}
		string get_xbee_mac() { return my_xbee_mac; }
		string get_mess_type() { return my_mess_type;}
		string get_debug_data() { return my_debug; }
		string get_ident() { return my_ident;}
		int get_free_slot() { return MAX_MESS_COUNTER-my_counter;}
		int get_delay() { return my_delay;}
		int get_resend_count() { return my_resend_count; }
		int get_priority() { return my_priority; }
		int cout_delay();
		void set_delay(int delay);
		bool add_message(string uc_mess);
		bool check_message(string uc_mess);
		bool postpone_avaible();
		~hp_mess_data();
	private:
		string my_mess;
		string my_uc_mess;
		string my_xbee_id;
		string my_xbee_mac;
		string my_mess_type;
		string my_debug;
		string my_ident;
		int my_counter;
		int my_priority;
		int my_delay;
		int my_resend_count;
		int my_postpone_count;
};

#endif
