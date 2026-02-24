#ifndef HP_GO_H 
#define HP_GO_H

#include <jsoncpp/json/json.h>
#include <unordered_set>
#include <thread>
//#include <memory>
//#include "../../hp_lib/include/server_ws.hpp"

//#include "../../hp_lib/include/server_wss.hpp"

/*
	Vymazanie prvku z vektora...:
	vector<int> c = {1,2,3,6,1,4,5};
	// neefektivne
	for(vector<int>::iterator itr = c.begin(); itr != c.end(); ){
		if(*itr == 1){
			itr = c.erase(itr);
		} else {
			itr++;
		}
	}
	//efektivnejsie odstranenie
	auto itr = remove(c.begin(), c.end(),1);
	c.erase(itr, c.end());
	c.shrik_to_fit(); /c++11
	
	pre list - member fct remove()
	pre associate container a unordered cont. use erase() - member fct

 */

//#define YIT
//#define AXON_DEBUG


#define MAX_RECCONECT_DELAY 60

#define SHORT_PIN_RULE 50
#define DOUBLE_PIN_THRESHOLD 80
#define LONG_PIN_RULE 300

#define DIAGNOSTIC_INTERVAL 45

#define SHARED_ENABLED 1
#define DB_EARLY_UPDATE 1

#define BASE_DEC 0
#define BASE_HEX 1
#define MAX_DELAY_TIME 1500
#define MAX_RESEND_COUNT 3
#define MAX_SIMULATION_TRIES	20
#define DELAY_CONSTANT 		20

#define EVENT_UPDATE_ALARMS 		0
#define EVENT_MESS_HEATING 		1
#define EVENT_HEATING_CHECK 		2
#define EVENT_HEATING_TEMPERING		3
#define EVENT_SHUTTERS			4
#define EVENT_TIMER_CLOCK_MESS  	5
#define EVENT_U_MESSAGE			6
#define EVENT_OFF_PIN			7
#define EVENT_SETUP_BLIND_TIME		8
#define EVENT_JABL_SEC_COUNTDOWN	9
#define EVENT_JABL_LOCK_ZONE		10
#define EVENT_WAT_ALL_ZONE		11

#define FORCE_HEATING_MESS	0

#define SIMULATION

/*
#ifndef HP_METEO_GIOM_H
#include "hp_meteo_giom.h"
#endif
*/

#ifndef HP_HBX_H
#include "hp_hbx.h"
#endif

#ifndef HP_PIN_CONS_H
#include "hp_pin_cons.h"
#endif

#ifndef HP_PIN_PWM_H
#include "hp_pin_pwm.h"
#endif

#ifndef HP_PIN_HEATER_H_
#include "hp_pin_heater.h"
#endif

#ifndef HP_PIN_STATUS_LIGHT_H
#include "hp_pin_status_light.h"
#endif

#ifndef HP_PIN_SHUTTER_H
#include "hp_pin_shutter.h"
#endif

#ifndef HP_PUSH_BUTTON_H
#include "hp_push_button.h"
#endif

#ifndef HP_ONOFFBUTTON_H
#include "hp_onOffButton.h"
#endif

#ifndef HP_PIN_TEMP_H
#include "hp_pin_temp.h"
#endif

#ifndef HP_LISTEN_THREAD_H
#include "hp_listen_thread.h"
#endif

#ifndef HP_SENDER_H
#include "hp_sender.h"
#endif

#ifndef HP_SHARED_MEMORY_H
#include "hp_shared_memory.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_LIGHTNING_H
#include "hp_lightning.h"
#endif

#ifndef HP_2HEATING_H
#include "hp2heating.h"
#endif

#ifndef HP_SHUTTERS_H
#include "hp_shutters.h"
#endif

#ifndef HP_WATERING_H
#include "hp_watering.h"
#endif

#ifndef HP_SECURITY_H
#include "hp_security.h"
#endif

#ifndef HP_JABLOTRON_H
#include "hp_jablotron.h"
#endif

#ifndef HP_CARDS_H
#include "hp_cards.h"
#endif

#ifndef HP_BUTTON_SCENARIO_H
#include "hp_button_scenario.h"
#endif

#ifndef HP_REPEAT_MESS_H
#include "hp_repeat_mess.h"
#endif

#ifndef HP_GATES_H
#include "hp_gates.h"
#endif

#ifndef HP_REKUPERACIA_H
#include "hp_rekuperacia.h"
#endif

#ifndef HP_TURNIKETS_H
#include "hp_turnikets.h"
#endif

#ifndef HP_TIMING_H
#include "hp_timing.h"
#endif

#ifndef HP_SEC_CAL_H
#include "hp_sec_calc.h"
#endif

#ifndef HP_NOTIFICATION_H
#include "hp_notification.h"
#endif

#ifndef HP_DALI_H
#include "hp_dali.h"
#endif

#ifndef HP_MODBUS_H
#include "hp_modbus.h"
#endif

#ifndef HP2MODBUS_H
#include "hp2modbus.h"
#endif

#ifndef HP_CONDITIONS_H
#include "hp_condition.h"
#endif

#ifndef HP_IMPULZ_COUNTER_H
#include "hp_impulz_counter.h"
#endif

#ifndef HP_WSS_SERVER_H
#include "hp_wss_server.h"
#endif

#ifndef HP_WSS_CLIENT_H
#include "hp_wss_client.h"
#endif

#ifndef RULE_CHECK_S
#define RULE_CHECK_S
typedef struct {
	int counter;
	int inactive_count;
	bool short_active;
	int pin_pos;
} hp_rule_checker_t;
#endif

#ifndef ALARM_CHECK_S
#define ALARM_CHECK_S
typedef struct {
	int pin_pos;
	int hour;
	int min;
	string send_value;
	string scen_command;
	string id;
} hp_alarm_checker_t;
#endif

#ifndef DELAYED_EVENT_S
#define DELAYED_EVENT_S

typedef struct {
	time_t run_time;
	int event_type;
	string var;
} delayed_event_t;
#endif

#ifndef METEO_S
#define METEO_S
typedef struct {
	string ident;
	string value;
} meteo_cnt_t;
#endif

using namespace std;
using std::ostream;
//using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

template<typename T>
ostream& operator<< (ostream& out, const vector<T>& v) {
	out << "{";
	size_t last = v.size() - 1;
	for(size_t i = 0; i < v.size(); ++i) {
		out << v[i];
		if (i != last){
			out << ", ";
		}
	}
	out << "}";
	return out;
}

/*
inline bool operator==(delayed_event_t& d,const int e_type){
	return e_type == d.event_type;
}
*/
struct Bug_socket {
	string socket_name;
	int sd;
	struct sockaddr_un m_unix_address;
	int err_counter=0;
	int send_data(string mess) {
		sd = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sd <0) {
			err_counter++;
			//cout <<"Chyba socketu" << endl;
			return sd;
		}
		err_counter = 0;
		m_unix_address.sun_family = AF_UNIX;
		strcpy(m_unix_address.sun_path,socket_name.data());
		if(connect(sd, (struct sockaddr *)&m_unix_address, sizeof(m_unix_address)) != -1){
			write(sd, mess.data(), sizeof(char)*mess.length());
			shutdown(sd,2);
			close(sd);
			return 0;
		}
		return -1;
	}
};

class hp_go {
	public:
		hp_go(string conf_file);
		int init(bool reread = false);
		int run();
		~hp_go();

	private: 
		std::vector<int> myt;
		//int open_connection(string ip, int port);
		int read_config();
		int create_driver_xml(XMLNode node);
		//int find_dali_item(string find, string property,int dalinet_pos = 0, int start = 0);
		int init_db();
		int fill_db();
		int sync_with_db();
		int reread_xml();
		void generate_json_web();
		string generate_json_wss();
		int check_force_restart();
		void print_report();
		void fill_shared_memory();
		string get_last_id(string table_name);
		int process_hbxs(MYSQL *conn);
		int process_statuses(MYSQL *conn);
		int process_scenaria(MYSQL *conn);
		int process_alarm(MYSQL *conn, bool only_check = false);
		int process_security(MYSQL *conn);
		int execute_query(string query, MYSQL *conn = NULL);
		void get_trusted_devices();
		int check_gui_mess(string ident);
		int push_db_statuses(bool first_time = false);
		void push_ws_data(string type, string ident, string value);


		void delete_objects(bool reread = false);
		void stop_thread();
		map<string,string> get_scen_data(string scen_id);

		hp_mess2send_t prepare_mess(string id_xbee, string mess_type, string mess, string debug = "");
		delayed_event_t prepare_event(int ev_type, int ev_delay, string ev_var);

		int find_item(string find, string property,int io_type = -1,int hbx_pos = 0, int start = 0);
		int process_gui_mess(string mess);
		int process_driver_resp(string mess);
		int process_txstat_mess(vector<string> data);
		int process_ack_mess(vector<string> data);
		int process_act_mess(vector<string> data);
		int process_brc_mess(vector<string> data);
		int process_xbee_mess(vector<string> data);
		int process_meteo_mess(vector<string> data);
		int process_rule(int pin_pos, int pushed, string hold, bool is_active_value = true, string on_value = "");
		int process_pin_conn(hp_conn_pin_t pin, string to_value, bool active_value = true);
		void find_out_rule(string ident,int actor_pos, string pwm_value = "", hp_mess2send_t *mess = NULL);
		void add_mess2sender(hp_mess2send_t mess, int pin_pos, int delay=0);
		void add_message(int pin_pos, string to_value, bool check_timer_hold = true, int delay =0, int create_off_mess = 0, bool delete_timer_mess = false, bool add_repeat = true, bool force_add = false);
		void add_message(hp_mess2send_t mess, int resend_count = 0, int priority = 0, int delay = 0);
		void add_message(string hbx_pos,string mess_type, string mess, int resend_count = 0, int priority = 0, int delay=0);
		int process_running_zone();
		int process_gui_alarm_mess(string mess);
		int process_gui_shutt_mess(string mess);
		int process_gui_scen_mess(string mess);
		int process_gui_secur_mess(string mess);
		int process_gui_temp_mess(string mess);
		int process_gui_wat_mess(string mess);
		int process_gui_air_mess(string mess);
		int process_gui_comm(string mess, hp_button_scenario *but_scen = NULL);
		int process_gui_turnikets(string mess);
		int process_ext_pin(string mess);
		int process_sim_data();
		int process_dn_change();
		int sync_hbx(int hpb_pos);
		int start_simulation(int );
		int set_auto_temp();
		void turn_off_wat_zone(string zone_ident);

		vector<string> parse_response(string resp, string separator="_");
		void check_sent_messages();
		void check_modbus_mess();
		void check_running_rules();
		void check_repead_mess();
		void check_periodic_pins();
		void check_alarm_mess(int hour, int min);
		void check_watering(int hour, int min);
		void check_heating_zones(int mode=-1, int delay = 0, bool set_temp = false);
		void check_heating(int min = -1);
		void check_hbx_times();
		void check_sec_singnalization();
		void check_conditions(int pos, string to_value);
		void process_delayed_events();
		void process_delayed_events_itr();
		void process_gate(int pin_pos, string value);
		void process_jabl_pin(int pin_pos, string value);
		void calculate_consumption(struct tm *);
		string send2temp_messages(string section_name, float query_temp = -1);
		void erase_running_secur(string id);
		void erase_delayed_event(int event_type = EVENT_SHUTTERS);
		bool has_delayed_event(int pin_pos);

		void send_U_messages();
		void send_U_message(int hbx_pos, int delay = 0);
		void setup_blinds_time();
		void send_diagnostic();
		void get_ouput_states();
		void process_dia_mess(int hbx_pos);
		void process_act_temp(int hbx_pos,string data);
		void process_act_cons(int hbx_pos,string data);
		void process_act_card(int hbx_pos,string data);
		void process_sht(int hbx_pos, string data);
		void restart_uc(int hbx_pos);
		void setup_hbx_outputs(int hbx_pos);

		void push_db_query(string query, int type = DB_STATUSES_QUERY, int log_level = 0);
		void push_all_statuses(string ident, string value);
		void push_shm_data(string ident, string value, int type = SHARED_PUSH_SINGLE_VALUE, int service = SHARED_SERVICE_STATUS);
		void push_delayed_event(delayed_event_t event);
		void push_system_state(struct tm * info);
		void push_heating_state();
		
		void read_dali_sht();

		hp_incoming *m_incomming; //struktura pre listen thread a mutexy (struktura je definovana v hp_listen_thread)
		hp_listen_thread *m_listen_thread; //objekt pre pocuvaci thread
		hp_dali_sht *m_dali_sht = NULL;

		hp_mdb_data_t *my_mdb_rx_data;
		hp_mdb_data_t *my_mdb_tx_data;

		hp2mdb_data_t *my2mdb_rx_data;
		hp2mdb_data_t *my2mdb_tx_data;
		hp_shared_memory *my_shm;

		hp_data_provider *my_data_provider;
		hp_db_data_t my_db_data;
		hp_sec_calc *my_sec_handler;
		hp_timing_t my_timing_data;
		hp_db_handler *my_db_handler;
		hp_timing *my_timing;
		hp_lightning *my_lightning;
		hp_shutters *my_shutters;
		hp_security *my_security;
		hp_jablotron *my_jablotron;
		hp2heating *my2heating;
		hp_watering *my_watering;
		hp_cards *my_cards;
		hp_conditions *my_conditions;
		hp_impulz_counter *my_impulz_counter;
		hp_notification *my_notification;
		hp_advanced_scenario *my_cin_scen;
		hp_advanced_scenario *my_cout_scen;
		vector<hp_button_scenario*> my_button_scen;
		hp_gates *my_gates;
		hp_rekuperacia *my_rekuperacia;
		hp_turnikets *my_turnikets;
		hp_dali *my_dali;
		hp_modbus *my_modbus;
		hp2modbus *my2modbus;
#ifdef SIMULATION
		vector<hp_alarm_checker_t> my_sim_control_data; 
		std::thread *my_sim_thread;
		hp_sim_data_t *my_sim_data;
#endif
		hp_xml_parser my_xml_parser;
		hp_sender *my_sender;

		hp_client_socket *bug_socket = NULL;

		vector<hp_hbx *> my_hbxs;
		vector<hp_virtual_pin *> my_pins;
		unordered_set<hp_virtual_pin *> sset;
		vector<hp_send_messages_t> my_send_messages;
		vector<hp_rule_checker_t> my_rule_checker;
		vector<hp_alarm_checker_t> my_alarm_checker;
		vector<hp_repeat_mess> my_repeat_mess;
		vector<hp_security_zone *> my_running_zone;
		vector<meteo_cnt_t> my_meteo_idents;
		vector<delayed_event_t> my_delayed_events;
		vector<int> my_periodic_pins;
		vector<modbus_devices_t> my_modbus_devices;
		map<string,string> my_trusted_devices;
		map<string,string> my_rooms;

		int my_socket;
		int my_log_level;
		int my_cpp_webport;
		string my_conf_file;
		string my_xbee_socket;
		string my_cls_socket;
		string my_mysql_db, my_mysql_passwd, my_mysql_user;
		string my_simulation_ident;
		string my_system_identifier;
		string my_wss_ident = "";
		string my_shm_ws_data = "";
		Json::Value my_ws_data;
		
		bool my_check_trusted_devices;
		bool my_simulator;
		bool my_has_electro_cons;
		bool my_has_hpp_web;
		bool my_use_https;
		bool my_use_old_hbx_timer;
		bool my_use_ws;

		int my_simulation_state;
		int my_simulation_control_counter;
		int my_simulation_try_counter;

		int my_sec_counter;
		int my_test_counter;
		int my_last_min;
		int my_last_hour;
		int my_system_start;
		int my_wss_connect_counter = 0;
		int my_wss_try_reconnect_in = 14;

		time_t wss_client_connect_time = 0;
		time_t last_check_time = 0;
	    	int wss_client_connect_delay = 2;


		void print_axon_report(string file, string line, string mess="");
#ifdef AXON_DEBUG
		string my_axon_file;
#endif
};



#endif
