#ifndef HP_GUARD_H
#define HP_GUARD_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

typedef struct {
	string type;
	string str_1;
	int num_1;
} hp_guard_notify_t;

class hp_guard
{
	public:
		hp_guard(XMLNode node);
		string get_ident() { return my_ident; }
		string get_type() { return my_type; }
		string get_condition() { return my_condition;}
		string get_text() { return my_guard_text;}
		string get_text_ok() { return my_guard_text_ok;}
		string get_unit() { return my_unit; }
		float get_critical_value() { return my_critical_value; }
		int n_notifiers() { return notifiers.size();}
		bool get_status() { return my_status; }
		void set_status(bool st) { my_status = st; }
		void set_last_change_time() { my_last_change_time = time(NULL);}
		void setup_pin_pos(int pos) { my_pin_position = pos; }
		int get_pin_pos() { return my_pin_position; }
		time_t get_last_change_time() { return my_last_change_time; }
		int get_check_time() { return my_check_time; }
	//	hp_guard_notify_t *get_notifier(int pos) { return (pos >0 && pos < (int)notifiers.size())?&notifiers[pos]:NULL;}
		const hp_guard_notify_t *get_notifier(int pos);
		~hp_guard();
	private: 
		hp_xml_parser my_xml_parser;

		string my_type;
		string my_ident;
		string my_guard_text;
		string my_guard_text_ok;
		int my_check_time;
		int my_pin_position;
		time_t my_last_change_time;
		string my_condition;
		float my_critical_value;
		bool my_status;
		string my_unit;

		vector<hp_guard_notify_t> notifiers;
};

#endif
