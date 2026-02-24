#ifndef HP_DALI_H
#define HP_DALI_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <thread>
#include <jsoncpp/json/json.h>


#include <mysql/mysql.h>
#define SLEEP_TIME 1000

#ifndef DALI_DEVICE_H
#include "hp_dalinet_device.h"
#endif

#ifndef DALI_GROUP_H
#include "hp_dali_group.h"
#endif

#ifndef DALI_PWM_LIGHT_H
#include "hp_dali_pwm_light.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_LIGHTNING_RULE_H
#include "hp_lightning_rule.h"
#endif

#define DALI_DELAY_TIME	0
#define DELAY_MESS 	1

#ifndef DALI_DEVICES_S
#define DALI_DEVICES_S
typedef struct {
	string ip;
	int port;
	string name;
	int pin_start;
} dali_devices_t;
#endif

#ifndef DALI_COMMANDS_S
#define DALI_COMMANDS_S
typedef struct {
	unsigned int id;	
	time_t send_time;
	int dalinet_pos;
	string mess;
} dali_commands_t;
#endif

#ifndef DALI_TIMER_S
#define DALI_TIMER_S
typedef struct {
	unsigned int id;	
	unsigned int after_id;
	int delay_type;
	int delay_counter;	
	int dalinet_pos;
	int button_pin_pos;
	string mess;
} dali_timers_t;
#endif

#ifndef SHT_MESS_S
#define SHT_MESS_S
typedef struct {
	string type;
	string ident;
	string status;
} sht_mess_t;
#endif

typedef struct hp_dali_sht{
	std::vector<sht_mess_t> mess;
	std::mutex mtx;
}hp_dali_sht;


//#define USE_SMART_POINTER
using namespace std;

class hp_dali {
	public:
		hp_dali(XMLNode node,hp_db_data_t *db_data, hp_dali_sht *);
		int fill_db(MYSQL *conn);
		void process_dali_response();
		int process_gui_command(string response);
		vector<int> process_light_rule(hp_rule_light_t rule, int pin_pos);
		void push_synchro_mess(vector<int> values, int pin_pos,string ident);
		void button_off(int pin_pos);
		void read_startup_values();
		Json::Value get_shm_data();
		~hp_dali();

	private: 
	//int open_connection(string ip, int port);
		int read_config(XMLNode node);
		string get_node_value(XMLNode xml_node, string , string parameter = "value");
		int find_dali_item(string find, string property,int dalinet_pos = 0, int start = 0);
		int find_dali_group(string find, string property,int dalinet_pos = 0, int start = 0);
		int init_db();
		int execute_query(string query, MYSQL *conn = NULL);
		void process_response(string response, int dalinet_pos);
		void process_Aresponse(string response, int dalinet_pos);
		void process_Gresponse(string response, int dalinet_pos);
		void process_BCresponse(string response, int dalinet_pos);
		unsigned int add_timer_item(int delay, int pos, string mess, int delay_type,unsigned int after_id = 0, int button_pin_pos = -1);
		void check_timers();
		void check_commands();
		int translate_resp_value(string resp, int item_pos);
		int calc_status4value(string value);
		string calc_value4status(int status);
		string create_mess(string mess, int item_pos);
		void push_cmd(string mess, int dalinet_pos, int mess_id = -1);
		void print_buffers();
		void delete_data();
		void push_db_query(string query, int type = DB_STATUSES_QUERY, int log_level = 0);
#ifdef USE_SMART_POINTER
		vector<std::unique_ptr<dali_item>> my_dali_items;
#else
		vector<dali_item *> my_dali_items;
#endif
		hp_db_data_t *my_db_data;
		hp_dali_sht *my_shared_data=NULL;
		hp_xml_parser my_xml_parser;

		vector<dali_group *> my_dali_groups;

		vector<dali_devices_t> my_dali_devices;
		vector<int> my_dali_ports;
		vector<dalinet_device *> my_dalinet_devices;
		vector<dali_data_t *>  my_dalinet_devices_data;
		vector<thread *> my_dalinet_threads;
		vector<string> my_xml_errors;
		vector<dali_timers_t> my_dali_timers;
		vector<dali_commands_t> my_dali_commands;

		Json::Value shared_ws_data;

		int my_gui_port;
		int my_dali_port;
		int my_socket;
		int my_log_level;
		string my_conf_file;
		string my_mysql_db, my_mysql_passwd, my_mysql_user;
		map<int,int> my_map_values;
		int my_delay_id;
		string my_dali_mode;
		int my_automatic_mode_pos;
		int my_mode_counter;
		bool testing;
};



#endif
