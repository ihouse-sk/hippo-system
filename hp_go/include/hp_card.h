#ifndef HP_CARD_H
#define HP_CARD_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

#ifndef HP_RULE_CARD_S
#define HP_RULE_CARD_S
typedef struct {
	int from_xbee;
	string ident;
	string to_value;
	string timer_time;
	string timer_type;
} hp_rule_card_t;
#endif

class hp_card {
	public:
		hp_card(XMLNode,string ident, string value);
		string get_value() { return my_value; }
		vector<hp_rule_card_t> get_valid_rules(int hbx_pos);
		void print_card();
		~hp_card();
	private: 
		hp_xml_parser xml_parser;
		string my_ident;
		string my_value;
		vector<hp_rule_card_t> my_rule_card;
};

#endif
