#ifndef HP_CARDS_H
#define HP_CARDS_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_CARD_H
#include "hp_card.h"
#endif

using namespace std;

class hp_cards {
	public:
		hp_cards(XMLNode);
		hp_card *find_card(string value);
		~hp_cards();
	private: 
		hp_xml_parser xml_parser;
		vector<hp_card> my_cards;
};

#endif
