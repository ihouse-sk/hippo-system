#ifndef HP_JABLOTRON_H
#define HP_JABLOTRON_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_JABLOTRON_ZONE_H
#include "hp_jablotron_zone.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

using namespace std;

class hp_jablotron {
	public:
		hp_jablotron(XMLNode,hp_db_data_t *db_data = NULL);
		string process_jablotron(MYSQL *conn);
		vector<string> get_armed_zones();
		bool all_zones_armed();
		bool has_armed_zone();
		bool is_sim_enabled() { return my_simulation_enabled; }
		string get_simulation_ident() { return my_simulation_ident; }
		bool get_simulation_value() { return my_simulation_value; }
		void set_sim_value(bool value) { my_simulation_value = value; }
		hp_jablotron_zone *find_zone(string id, string type="ident");
		Json::Value get_shm_data();
		~hp_jablotron();
	private: 
		hp_db_data_t *my_db_data;
		vector<hp_jablotron_zone *> my_zones;
		hp_xml_parser xml_parser;
		bool my_simulation_enabled;
		bool my_simulation_value;
		string my_simulation_ident;
};

#endif
