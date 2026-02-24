#ifndef HP_GATE_H
#define HP_GATE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#define INPUT_IMPULZ 	"impulz"
#define INPUT_CHANGE	"change"

#define UPDATE_GATE_CONTROL_NON 	0
#define UPDATE_GATE_CONTROL_NORMAL	1
#define UPDATE_GATE_CONTROL_INVERZ	2

#define GATE_STATUS_IMPULZ	"impulz" /// - dopisat co to je
#define GATE_STATUS_CHANGE	"change" /// -dopisat co to je

#define GATE_LOCK_NON		"non"
#define GATE_LOCK_NORMAL	"normal" // zobrazenie v grafike log 0 je odomknute a log 1 je zamknute
#define GATE_LOCK_INVERZ	"inverz"// zobrazenie v grafike log 1 je odomknute a log 0 je zamknute

//#ifndef HP_GATE_DATE_S
//typedef struct {
//	string 

using namespace std;

class hp_gate {
	public:
		hp_gate(XMLNode);
		string get_ident() { return my_gui_ident; }
		string get_gui_lock_ident () { return my_lock_gui_ident; }
		string get_gui_lock() { return my_lock_gui_ident; }
		string get_status_gui_ident() { return my_status_gui_ident; }
		string get_status() { return patch::to_string(my_gate_state); }
		string get_ident_closed() { return my_pin_closed; }
		string get_ident_opened() { return my_pin_opened; }
		int setup_state(int value, string pin);
		int update_control_pin() { return my_update_control_pin;}
		int get_status_pin_count() { return my_status_pin_count; }
		string get_move_ident (string dir);
		string get_lock_control_type() { return my_lock_control_type; }
		void init_state_setup(int state) { my_gate_state = state; }
		string get_report();
		~hp_gate();
	private: 
		hp_xml_parser xml_parser;
		string my_type;
		string my_gui_ident;
		string my_pin_up, my_pin_down;
		string my_pin_opened;
		string my_pin_closed;

		string my_status_gui_ident;
		string my_status_control_type;
		int my_status_pin_count;

		string my_lock_gui_ident;
		string my_lock_control_type;
		int my_lock_pin_count;

		int my_state_opened;
		int my_state_closed;
		int my_gate_state;
		int my_lock_state;
		int my_update_control_pin;
};

#endif
