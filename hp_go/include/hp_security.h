#ifndef HP_SECURITY_H
#define HP_SECURITY_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>

#include <jsoncpp/json/json.h>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_SECURITY_ZONE_H
#include "hp_security_zone.h"
#endif

using namespace std;

class hp_security {
	public:
		hp_security(XMLNode);
		unsigned int get_zones_count() { return my_zones.size(); }
		string get_zone_ident(int pos);
		const string get_all_ident() { return my_all_ident;}
		vector<string> get_zones_ident();
		vector<string> get_actors4armed_zones();
		hp_security_zone *find_zone(string id);
		hp_security_zone *chech_pir(string actor);
		bool is_active_zone(string zone_ident);
		bool is_sim_enabled() { return my_simulation_enabled; }
		bool get_simulation_value() { return my_simulation_value; }
		bool has_armed_zone();
		void set_sim_enabled(bool sim) { my_simulation_enabled = sim;}
		void set_sim_value(bool value) { my_simulation_value = value; }
		int get_all_zone_status();
		
		Json::Value get_shm_data();
		string check_signalization();
		string get_signalization_ident() { return my_signalization_ident; }
		bool has_signalization() { return my_signalization_enabled;}
		string get_simulation_ident() { return my_simulation_ident;}
		~hp_security();
	private: 
		hp_xml_parser xml_parser;
		string my_all_ident;
		string my_signalization_ident;
		vector<hp_security_zone> my_zones;
		bool my_status;
		bool my_signalization_enabled;
		bool my_simulation_enabled;
		bool my_simulation_value;
		string my_simulation_ident;
};

#endif
