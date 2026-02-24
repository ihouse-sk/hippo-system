#ifndef HP_NOTIFICATION_H
#define HP_NOTIFICATION_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <thread>

#define NN_CHECK	"check"
#define NN_START	"start"
#define NN_ERROR	"error"
#define NN_INFO		"info"

#ifndef HP_PIN_NOTIFIER_H
#include "hp_pin_notifier.h"
#endif

#ifndef HP_GUARD_H
#include "hp_guard.h"
#endif

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_THREADED_SOCKET_H
#include "hp_threaded_socket.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_VIRTUAL_PIN_H
#include "hp_virtual_pin.h"
#endif

using namespace std;

class hp_notification 
{
	public:
		hp_notification(XMLNode node, string system_ident, hp_db_data_t *data = NULL, bool simulator = false);
		void check_pin_notification(string pin_id, string value, string pin_label);
		void check_periodic_guard(const vector<hp_virtual_pin *> pins);
		void setup_guards_position(const vector<hp_virtual_pin *> pins);
		void push_notification(string mess_type, string mess);
		void push_security_notification(string mess, int sec_value);
		int get_hbx_check_time() { return my_hbx_check_time; }
		~hp_notification();
	private: 
		hp_xml_parser xml_parser;
		hp_db_data_t *my_db_data;
		vector<hp_pin_notifier> my_pins_not;
		vector<hp_guard> my_guards;
		bool my_hbx_guard;
		bool my_security_notification;
		string my_telegram_token;
		string my_main_chat_id;
		string my_system_identifier;
		string my_main_url;
		int my_hbx_check_time;
		bool my_simulator;

		void push_telegram_notification(string mess);
		void push_main_information(string mess_type, string mess);
		void push_guard_notification(const hp_guard_notify_t *notify, string condition, string text, string ident, string value, int status, string unit = "minut" );
		void push_db_query(string query, int type=DB_STATUSES_QUERY, int log_level=0);
		void create_thread(int port, string url, string mess, string mess_type="", bool https = true);
		string replace_url(string mess);
};

#endif
