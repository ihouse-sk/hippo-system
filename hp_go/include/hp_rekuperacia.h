#ifndef HP_REKUPERACIA_H
#define HP_REKUPERACIA_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_REKUPERACIA_ZONE_H
#include "hp_rekuperacia_zone.h"
#endif

#ifndef HP_VIRTUAL_PIN_H
#include "hp_virtual_pin.h"
#endif

using namespace std;

class hp_rekuperacia {
	public:
		hp_rekuperacia(XMLNode);
		hp_rekuperacia_zone *find_zone(string zone_ident);
		const vector<string> get_rek_zones_idents();
		vector<string> sync_rek(const vector<hp_virtual_pin *>);
		bool has_rek_pin(string ident);
		string get_actual_state() { return my_active_mode;}
		~hp_rekuperacia();
	private: 
		hp_xml_parser xml_parser;
		vector<hp_rekuperacia_zone> my_rek_zones;
		vector<rek_actor_t> my_all_actors;
		string my_active_mode;
};

#endif
