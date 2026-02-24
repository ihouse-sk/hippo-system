#ifndef HP_2HEATING_H
#define HP_2HEATING_H

#include <boost/algorithm/string.hpp>
#include <jsoncpp/json/json.h>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/../../hp_lib/include/hp_xml_parser.h"
#endif



#ifndef HP_HEATING_SECTION_H
#include "hp_heating_section.h"
#endif

#ifndef HP_HEATING_SECTION_MDB_H
#include "hp_heating_section_mdb.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_MODBUS_H
#include "hp_modbus.h"
#endif

#ifndef HP_NOTIFICATION_H
#include "hp_notification.h"
#endif

using namespace std;

class hp2heating {
	public:
		hp2heating(XMLNode,hp_db_data_t *db_data );
		string process_db(MYSQL *conn);
		int process_thermostat_mess(vector<string> mess);
		void set_query_temp(float temp) { my_query_temp = temp; }
		hp_heating_section *find_section(string sensor, string by = "sensor");
		vector<hp_heating_data_t> set_heating_mode(int mode, bool set_temp = false );
		vector<hp_heating_data_t> process_section(string name, float query_temp = -1);
		vector<hp_heating_data_t> process_section_temp(string sensor_name, string temp, hp_notification *nn);
		vector<hp_heating_data_t> check_heating(int min);
		vector<hp_heating_data_t> check_windows(string ident, int value);
		string get_eco_ident() { return my_eco_ident; }
		string get_all_ident() { return my_all_ident; }
		float get_auto_query_temp() { return my_auto_query_temp; }
		float get_all_query_temp() {return my_all_query_temp<0?my_def_temp:my_all_query_temp; }
		int get_heating_mode() { return my_heating_mode; }
		float get_eco_temp() { return my_eco_temp; }
		float get_def_temp() { return my_def_temp;}
		void set_def_temp(float v) { my_def_temp = v;}
		float get_def_tempering() { return my_def_tempering_value;}
		void set_def_tempering(float v) { my_def_tempering_value = v;}
		void set_eco_temp(float temp) { my_eco_temp = temp;}
		void set_all_query_temp(float temp); //{ my_all_query_temp = temp; }
		void set_auto_query_temp (float temp); //{ my_auto_query_temp= temp<=0?this->my_def_temp:temp;}
		bool deactivate_tempering (string actor_ident);
		string get_report();
		void push_zones_states();
		void update_mdb_data(hp_mdb_data_t *tx_data,vector<modbus_devices_t> mdb);
		void set_mdb_master_pos(vector<modbus_devices_t> mdb);
		void process_mdb_response(hp_mdb_data_t *rx_data);
		Json::Value get_shm_data();
		Json::Value get_ws_data() { return my_ws_data; }
		void clear_ws_data() { my_ws_data.clear(); }
		~hp2heating();
	private: 
		void push_db_query(string query, int type = DB_STATUSES_QUERY, int log_level = 0);
		vector<string> parse_response(string resp, string separator);
		void push_ws_data(string type, string ident, string value, string ex1="", string ex2="");

		hp_db_data_t *my_db_data;
		hp_xml_parser xml_parser;
		string my_all_ident;
		float my_def_temp;
		float my_def_tempering_value;
		float my_actual_temp;
		float my_query_temp;
		string my_ref_temp;
		Json::Value my_ws_data;
		bool my_use_ws;

		string my_eco_ident;
		float my_eco_temp;
		float my_heating_treshold;
		float my_all_query_temp;
		float my_auto_query_temp;

		vector <hp_heating_section> my_heating_section;
		vector <hp_heating_section_mdb> my_heating_section_mdb;
		int my_heating_mode;
		string my_master_switch;
		string my_master_cooler;
		bool my_has_master_switch;
		/*
		vector <string> my_main_ventils;
		bool my_has_main_ventils;

		

		*/

};

#endif
