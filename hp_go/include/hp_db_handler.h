#ifndef HP_DB_HANDLER_H
#define HP_DB_HANDLER_H

#include <mutex>
#include <fstream>
#include <vector>
#include <mysql/mysql.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <unistd.h>

using namespace std;
#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#if defined(__aarch64__) || defined(__arm__)
#endif
	#define AXON_SERVER

#define DB_STOP 71

#define DB_STATUSES_QUERY 1
#define DB_ALL_STATUSES 2
#define DB_QUERY 3
#define DB_TRANSACTION 4

#define DB_LOG_COM 10
#define DB_LOG_ERROR 11

#define DB_SEND_ERROR 20

#define DB_SYSTEM_STATUS 30

#ifndef DB_QUERIES_S
#define DB_QUERIES_S
typedef struct {
	string query;
	int type;
	int log_level;
} hp_db_queries_t;
#endif

#ifndef DB_DATA_S
#define DB_DATA_S
typedef struct {
	//vector<string> queries;
	vector<hp_db_queries_t> queries;
	std::mutex mtx;
	bool finish;
} hp_db_data_t;
#endif

#define DB_SLEEP 100000  // us

#define CONN_OPEN_TIME 10000 // ms
#define ALL_STATUSES_TIME 60000 // ms
#define HBXS_TIME 5000 // ms

#define SYSTEM_TESTING 1
#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"

class hp_db_handler {
	public:
		hp_db_handler(string user, string passwd, string db, hp_db_data_t *data);
		void operator()();
		~hp_db_handler();
	private: 
		string my_mysql_user;
		string my_mysql_passwd;
		string my_mysql_db;
		hp_db_data_t *my_db_data;
		string my_query_all_statuses;
		string my_query_hbxs;
		string my_comm_log;
		string my_current_folder;
		int my_current_hour;
		vector<string> my_transaction_queries;

		void create_new_folders();
		void backup_logs();
		int execute_query(string query);
		int execute_all_statuses();
		void process_transaction();
		void write_message(hp_db_queries_t data);
		std::string time_now();
		bool create_multicasts();
		void stop_multicasts();
		bool send_multicasts_data(string data);

		int my_open_counter;
		int my_all_counter;
		int my_hbxs_counter;
		int my_log_level;
		int my_last_hour;
		int my_last_min;

		struct sockaddr_in my_cast_addr;
		int my_fd_cast;
		bool my_multicasts_state;
		int my_multicasts_start;
		//struct ip_mreq mreq;
		
		MYSQL *conn;
};

#endif
