#ifndef HP_TURNIKETS_H
#define HP_TURNIKETS_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_TURNIKET_H
#include "hp_turniket.h"
#endif

#define DELAY_INTERVAL 60

typedef struct {
	string id;
	string card_id;
	string card_type;
	time_t valid_until;
	time_t last_enter;
} cards_data_t;

typedef struct {
	string query;
	bool valid_card;
} card_valid_t;

using namespace std;

class hp_turnikets {
	public:
		hp_turnikets(XMLNode);
		string get_table_name() { return my_table_name; }
		string get_gui_ident() { return my_gui_ident; }
		void add_card(cards_data_t card);
		void remove_card(string card_id);
		void print_cards();
		vector<string> erase_old_cards();
		hp_turniket *find_turniket(string mac);
		hp_turniket *check_turniket(string ident);
		card_valid_t is_valid_card(string card_id);
		~hp_turnikets();
	private: 
		hp_xml_parser xml_parser;
		vector<cards_data_t> my_cards;
		vector<hp_turniket> my_real_turnikets;
		string my_table_name;
		string my_gui_ident;
};

#endif
