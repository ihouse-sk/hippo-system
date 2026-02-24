#ifndef HP_VIRTUAL_PIN_H
#define HP_VIRTUAL_PIN_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_CONN_PIN_S
#define HP_CONN_PIN_S
typedef struct {
	string ident;
	string on_value;
	string to_value;
} hp_conn_pin_t;
#endif

#ifndef HP_MESS2SEND_S
#define HP_MESS2SEND_S
typedef struct {
	string xbee_id;
	string xbee_mac;
	string uc_mess_type;
	string mess;
	string service_type;
	string last_direction;
	string to_value;
	int timer_hold;
	int repeat_timer;
	int max_triggers;
	int min_value;
	string debug;
	string ident;
} hp_mess2send_t;
#endif


#define UC_BLIND_TIME		0x32
#define UC_CHECK_PORTS		0x36

#define HP_PIN_INPUT 	0
#define HP_PIN_OUTPUT	1

#define PIN_STATUS_LIGHT 	0
#define PIN_PWM 		1
#define PIN_SHUTTER 		3
#define PIN_SOCKET 		4
#define PIN_ELECTRO_CONS	5
#define PIN_SPRINKLER		6
#define PIN_HEATER 		7
#define PIN_COOLER 		8
#define PIN_GATES_ALL		9
#define PIN_GAS_CONS 		10
#define PIN_WATER_CONS		11
#define PIN_GSM		12
#define PIN_REST		22

using namespace std;

class hp_virtual_pin {
	public:
		hp_virtual_pin(XMLNode, int hbx_pos);
		void print_pin();
		string get_id() { return my_id; }
		string get_desc() { return my_desc; }
		string get_desc_2() { return my_desc2; }
		string get_gui_desc() { return my_gui_desc; }
		string get_function() { return my_function; }
		string get_active_value() { return my_active_value; }
		string get_location() { return my_location; }
		bool update_gui() { return my_gui_update; }
		bool is_scenario() { return my_scenaria_active; }
		bool is_alarm() { return my_alarm_active; }
		bool get_priotity_state() { return my_priorty_on; }
		void set_priority_state(bool priority) {  my_priorty_on = priority; }
		int get_int_type() { return my_int_type; }
		int get_hbx_pos() { return my_hbx_pos; }
		int get_io_type() { return my_io_type; }
		int get_periodicity() { return my_periodic_check;};
		bool is_inverz() { return this->my_type[0]=='i'?true:false;};
		int get_internal_timer() { return my_pin_timer; }
		string get_update_string();

		const vector<hp_conn_pin_t> get_conn_pins() { return my_conn_pins;}
		string get_onOff_ident() { return my_onOff_pin!=""?my_onOff_pin:""; }
		string get_report();

		// for pins
		virtual string set_status(string status) = 0;
		virtual string get_status() = 0;
		//// for output pins
		virtual hp_mess2send_t create_send_mess(string value, bool timer_message = false) = 0;
		/// for input pins
		virtual bool check_input(string input_type) = 0;
		virtual bool check_periodicity() = 0;
		virtual void setup_input_rules(bool short_pushed, bool long_pushed, bool doube_pushed, bool shutter_control = false) = 0;
		virtual ~hp_virtual_pin();
	protected:
		string my_id;
		int my_hbx_pos;
		int my_status;
		string my_active_value;
		int my_pin_timer;
		string my_timer_on_value, my_timer_off_value;
		int my_repeat_timer;
		int my_max_triggers;
		hp_xml_parser xml_parser;
		bool my_shord_pushed;
		bool my_long_pushed;
		bool my_double_pushed;
		bool my_shutter_control;
		bool my_priorty_on;
		vector<hp_conn_pin_t> my_conn_pins;
		string my_onOff_pin;
		int my_periodic_check;
		int my_periodic_counter;
		bool my_changed;
	private: 

		string my_desc, my_desc2, my_function, my_type;
		int my_int_type;
		int my_io_type;
		bool my_alarm_active, my_scenaria_active;
		string my_gui_desc;
		bool my_gui_update;
		string my_location;
		
		bool my_valid_status;
};

#endif
