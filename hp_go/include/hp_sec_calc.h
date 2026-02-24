#ifndef HP_SEC_CALC_H
#define HP_SEC_CALC_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <mysql/mysql.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mutex>
#include <cstring>

using namespace std;
#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef SIM_STRUCT_S
#define SIM_STRUCT_S
typedef struct {
	string ident;
	string to_value;
	int min;
	int hour;
} sim_data_t;
#endif

#ifndef DOVOLENKA_STRUCT_S
#define DOVOLENKA_STRUCT_S
typedef struct {
	bool finished;
	int day_minus;
	vector<sim_data_t> sim_data;
	mutex mtx;
} hp_sim_data_t;
#endif

#define SIM_OFF 	0
#define SIM_CALCULATION	1
#define SIM_ON 		2

class hp_sec_calc {
	public:
		hp_sec_calc(string user, string passwd, string db, hp_sim_data_t *sim_data );
		void operator()();
		~hp_sec_calc();
	private: 
		string my_mysql_user;
		string my_mysql_passwd;
		string my_mysql_db;
		hp_sim_data_t *my_sim_data;
				
		MYSQL *conn;

};

#endif
