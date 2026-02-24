#ifndef HP_WATERING_H
#define HP_WATERING_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#include <mysql/mysql.h>
#include <map>
#include <jsoncpp/json/json.h>
#include <cstring>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_WATERING_ZONE_H
#include "hp_watering_zone.h"
#endif

#ifndef HP_WATERING_DATA_S
#define HP_WATERING_DATA_S
typedef struct {
	map<string,int> out;
	string query;
} hp_wat_data_t;
#endif

using namespace std;

class hp_watering {
	public:
		hp_watering(XMLNode);
		string process_db(MYSQL *conn);
		void setup_zone(string zone_ident,int running, int auto_enabled, int timer, vector<int> start_time, vector<int> time_enabled, vector<int> week_day_enabled);
		void update_zone(vector<string> parsed);
		vector<hp_wat_data_t> check_start(int now);
		vector<string> check_end(int now);
		hp_watering_zone *find_wat_zone(string ident);
		vector<string> get_zones_ident();
		vector<hp_wat_data_t> setup_wat_enabled(string ident, int value);
		bool is_wat_sensor(string ident) { return ident==my_sensor_ident?true:false;}
		string get_sensor_ident() { return my_sensor_ident; }
		string get_report();
		string get_all_ident() { return my_all_zone_ident; }
		string add_zero(string str,int len = 2);
		Json::Value get_shm_data(bool v2 =false);
		~hp_watering();
	private: 
		hp_xml_parser xml_parser;
		vector<hp_watering_zone> my_wat_zones;
		bool my_watering_enabled;
		string my_sensor_ident;
		string my_all_zone_ident;
		int my_max_running_zones;
		int my_running_zones_count;
};

#endif
