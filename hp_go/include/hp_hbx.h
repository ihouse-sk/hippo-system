#ifndef HP_HBX_H
#define HP_HBX_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#define RESTART_NACK 1
#define RESTART_WATCHDOG 2

#define MESS_TIME_THRESHOLD 10
#define RESTART_TIME_TRESHOLD 120
#define MESS_U_THRESHOLD 10
#define PROTECT_INTERVAL 10

using namespace std;

class hp_hbx {
	public:
		hp_hbx(XMLNode, int hbx_pos, bool);
		string get_mac() { return my_mac; }
		void set_pos(int pos) { my_pos = pos;}
		void set_restart_type(int type);
		int  get_restart_type();
		int  get_pos() {return my_pos;}
		void increase_mess_counter() { 
			my_mess_counter++;
			my_mess_timer = time(NULL);
		}
		void decrease_mess_counter() { if(my_mess_counter > 0) { my_mess_counter--;}}
		void set_hbx_status(int status) { my_hbx_status = status; }
		void set_internal_timer() {  
			my_internal_timer = time(NULL);
			//cout <<"Seting up internal timer pre hbx: " << my_mac << " pos:  " << my_pos << endl;
		}
		string check_internal_timer(int check_time);
		int get_hbx_status() { return my_hbx_status; }
		int  get_mess_counter() {return my_mess_counter;}
		string get_location() { return my_location; }
		string get_ext_ip() { return my_ext_ip; }
		string get_name() { return my_name; }
		int get_ext_port() { return my_ext_port; }
		bool is_active() { return my_hbx_active; }
		bool restart_enabled();
		bool in_protect_interval();
		string check_hbx_timer();
		bool was_u_sent() { return my_u_send; }
		void u_sent(bool u_sent) { 
			my_u_send = u_sent;
			if(my_u_send){
				my_u_timer = time(NULL);
			} else {
				my_u_timer = 0;
			}
		} 
		void print_hbx();
		~hp_hbx();
	private: 
		hp_xml_parser xml_parser;
		string my_mac;
		string my_name;
		string my_type;
		string my_location;
		string my_ext_ip;
		int my_ext_port;

		bool my_status;
		bool my_u_send;
		bool my_hbx_active;
		bool my_notification_send;
		time_t my_internal_timer;
		time_t my_u_timer;
		time_t my_restart_timer;
		int my_restart_type;
		int my_pos;
		int my_mess_counter;
		time_t my_mess_timer;

		int my_hbx_status;
};

#endif
