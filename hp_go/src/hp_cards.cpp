#include "../include/hp_cards.h"

hp_cards::hp_cards(XMLNode node) 
{
	for(int i=0; i<node.nChildNode("card"); i++){
		string card_ident = xml_parser.get_node_value(node, "card", "ident",i);
		string card_value = xml_parser.get_node_value(node, "card", "value",i);
		my_cards.push_back(hp_card(node.getChildNode("card",i), card_ident, card_value));
	}

	for(unsigned int i=0; i< my_cards.size(); i++){
	//	my_cards[i].print_card();
	}
}
hp_card *hp_cards::find_card(string value)
{
	for(unsigned int i=0; i<my_cards.size(); i++){
	//	cout <<my_cards[i].get_value() << " == " << value << endl;
		if(my_cards[i].get_value() == value){
			return &my_cards[i];
		}
	}

	return NULL;
}

hp_cards::~hp_cards() {}
