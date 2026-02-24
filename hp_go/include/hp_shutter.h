#ifndef HP_SHUTTER_H
#define HP_SHUTTER_H

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

#define SHUT_UP 	"up"
#define SHUT_DOWN 	"down"
#define SHUTTER_STEP_TIME 	30
#define SHUTTER_STEP_INDEX	1	
#define SHUTTER_MAX_TILT	12 // krajna poloha zaluzii ked su maximalne zatiahnute (viacej tmy)

#define SHUTTER_CONTROL 1 /// 1 - plynule, 0 - skokove

using namespace std;

class hp_shutter {
	public:
		hp_shutter(XMLNode, vector<hp_virtual_pin *> pins);
		bool has_actor(string actor);
		string get_last_move_direction() { return my_last_move_direction; }
		string get_gui_ident() { return my_gui_ident; }
		int get_pin_dir() { return my_pin_dir; }
		int get_pin_on_off() { return my_pin_on_off; }
		int get_my_timer() { return my_timer;}
		int get_tilt() { return my_tilt; }
		void set_direct_tilt(int tilt) { my_tilt = tilt;}
		int get_mevement_steps (string dir);
		int get_pos() { return my_position; }
		void set_tilt(int tilt);

		void set_move_direction(string move_dir) { my_last_move_direction = move_dir; }
		~hp_shutter();
	private: 
		hp_xml_parser xml_parser;
		string my_gui_ident; 
		string my_last_move_direction;
		int my_pin_dir;
		int my_pin_on_off;
		int my_timer;
		int my_position;
		vector<string > my_control_buttons;
		bool my_status;
		int my_tilt;
		int my_send_steps;

		int find_item(string find, vector<hp_virtual_pin *> pins);
};

#endif
