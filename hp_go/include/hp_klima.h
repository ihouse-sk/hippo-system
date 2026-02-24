#ifndef HP_KLIMA_H
#define HP_KLIMA_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#include <mysql/mysql.h>
#include <map>
#include <thread>
#include <boost/algorithm/string.hpp>

#ifndef HP_KLIMA_SECTION_H
#include "hp_klima_section.h"
#endif

#ifndef HP_THREADED_SOCKET_H
#include "hp_threaded_socket.h"
#endif

#define MAX_FRAME 35


using namespace std;

class hp_klima {
	public:
		hp_klima(XMLNode,hp_db_data_t *db_data);
		string process_db(MYSQL *conn);
		int process_mess(vector<string> data);
		int process_air_resp(string mess);
		void process_temp_mess(string sensor, float value);
		void init_section(string ident, float set_temp, float actual_temp, int running, int fan_speed, int mode, int louvre, int force_thermo);
		~hp_klima();
	private: 
		void push_db_query(string query, int type = DB_STATUSES_QUERY, int log_level = 0);
		void send_air_mess(string ip, int port, string mess);
		vector<string> parse_response(string resp, string separator="_");
		int get_free_id();
		void check_section(int pos);

		hp_db_data_t *my_db_data;
		hp_xml_parser xml_parser;
		vector<hp_klima_section> my_sections;
		vector<int> my_frame_ids;
		vector<string> my_send_messages;
		int my_frame_counter;
};

#endif
