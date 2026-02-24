#include "../include/hp_card.h"

hp_card::hp_card(XMLNode node,string ident, string value): my_ident(ident), my_value(value)
{
	for(int i=0; i<node.nChildNode("connection"); i++){
		string from_xbee = xml_parser.get_node_value(node.getChildNode("connection",i), "from_xbee","id");
		for(int j=0; j<node.getChildNode("connection",i).nChildNode("ident"); j++){
			hp_rule_card_t tmp;
			tmp.from_xbee = patch::string2int(from_xbee);
			tmp.ident = xml_parser.get_node_value(node.getChildNode("connection",i), "ident", "value",j);
			tmp.to_value = xml_parser.get_node_value(node.getChildNode("connection",i), "ident", "to_value",j);
			tmp.timer_time = xml_parser.get_node_value(node.getChildNode("connection",i), "ident", "timer_hold",j);
			my_rule_card.push_back(tmp);
		}
	}
}

vector<hp_rule_card_t> hp_card::get_valid_rules(int hbx_pos)
{
	vector<hp_rule_card_t> res;
	for(unsigned int i=0; i<my_rule_card.size(); i++){
		if(my_rule_card[i].from_xbee == hbx_pos){
			res.push_back(my_rule_card[i]);
		}
	}
	return res;
}

void hp_card::print_card()
{
	cout <<"ident: " << my_ident << ", value: " << my_value << " Connections: "<< endl;
	for(unsigned int i=0; i<my_rule_card.size(); i++){
		cout << "from xbee: " << my_rule_card[i].from_xbee << " ident: " << my_rule_card[i].ident << " to_value: " << my_rule_card[i].to_value << " timer hold: " << my_rule_card[i].timer_time << endl;
	}
}

hp_card::~hp_card() {}
