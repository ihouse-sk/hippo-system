#ifndef HP_SHUTTERS_H
#define HP_SHUTTERS_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_SHUTTER_H
#include "hp_shutter.h"
#endif

using namespace std;

class hp_shutters {
	public:
		hp_shutters(XMLNode, vector<hp_virtual_pin *> pins);
		hp_shutter *find_shutter(string actor);
		vector<hp_shutter *> find_multi_rule(string actor);
		bool has_rule4pin(string actor);
		string get_all_ident() { return all_ident;}
		string get_all_up_ident () { return all_up_ident ;}
		string get_all_down_ident () { return all_down_ident;}
		string get_up_ident () { return up_ident;}
		string get_down_ident () { return down_ident;}
		bool has_direct_control() { return my_direct_control;}
		void set_move_enabled (bool move_enabled) { my_enabled_shutt_move = move_enabled; }
		bool get_move_enabled () { return my_enabled_shutt_move; }
		vector<hp_shutter *> check_critical_wind(int speed);
		const vector<hp_shutter *> move_all_shutters();
		void setup_shut_tilt(string shutt, int tilt);
		~hp_shutters();
	private: 
		hp_xml_parser xml_parser;
		string all_ident, all_up_ident, all_down_ident, up_ident, down_ident;
		vector<hp_shutter*> my_shutters;
		bool my_status;
		bool my_save_control_enabled;
		bool my_enabled_shutt_move;
		int my_critical_speed;
		bool my_direct_control;
};

#endif
