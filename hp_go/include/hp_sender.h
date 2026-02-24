#ifndef HP_SENDER_H
#define HP_SENDER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#include <algorithm>
#include <thread>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_VIRTUAL_PIN_H
#include "hp_virtual_pin.h"
#endif

#ifndef HP_CLIENT_SOCKET_H
#include "hp_client_socket.h"
#endif

#ifndef HP_MESS_DATA_H
#include "hp_mess_data.h"
#endif
#ifndef HP_HBX_H
#include "hp_hbx.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_THREADED_SOCKET_H
#include "hp_threaded_socket.h"
#endif

#define HBX_BUFFER 1
#define MAX_XBEE_MESS_ID 98
#define DISPLAY_MESSAGES 1

#ifndef SEND_MESSAGES_S
#define SEND_MESSAGES_S
typedef struct {
	string mess;
	int time_form_send;
	int sent_count;
	int txstat;
	string xbee_id;
	string frame_id;
	string mess_type;
} hp_send_messages_t;
#endif

#ifndef UC_ID_DATA_S
#define UC_ID_DATA_S
typedef struct {
	unsigned char id;
	bool free;
} hp_id_data_t;
#endif

using namespace std;

class hp_sender {
	public:
		hp_sender(string xbee_socket);
		hp_sender(string xbee_socket, hp_db_data_t *data);
		//bool add_message(vector<string> data, int resend_count = 0, int priority = 0, int delay = 0);
		bool add_message(hp_mess2send_t data, int resend_count = 0, int priority = 0, int delay = 0);
		void send_messages(vector<hp_hbx *> &hbxs, vector<hp_send_messages_t> &send_messages_vect);
		void delete_timer_mess(string mess, string xbee_id);
		void free_xbee_id(int id);
		void free_uc_id(string id);
		bool is_free_xbee_id(int id);
		void print_non_free_id();
		vector<int> get_non_free();
		~hp_sender();
	private:
		hp_client_socket *my_client_socket;
		string my_xbee_socket;
		vector<hp_mess_data> my_messages;
		vector<int > my_xbee_mess_id;
		vector<hp_id_data_t> my_uc_mess_id;
		int my_start_xbee_id;
		hp_db_data_t *my_db_data;

		int get_free_xbee_mess_id();
		string get_free_uc_mess_id();
		time_t last_time;

		void push_db_query(string query, int type=DB_STATUSES_QUERY, int log_level=0);
};

#endif
