#ifndef HP_REKUPERACIA_ZONE_H
#define HP_REKUPERACIA_ZONE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

typedef struct {
	string actor;
	string to_value;
} rek_actor_t;

using namespace std;

class hp_rekuperacia_zone {
	public:
		hp_rekuperacia_zone(XMLNode, string gate_type);
		string get_ident() { return my_ident; }
		bool is_active_zone(vector<rek_actor_t> all);
		vector<rek_actor_t> get_actors() { return my_actors;}
		~hp_rekuperacia_zone();
	private: 
		hp_xml_parser xml_parser;
		string my_ident;
		vector<rek_actor_t> my_actors;
	};

#endif
