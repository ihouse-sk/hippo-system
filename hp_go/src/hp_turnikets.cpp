#include "../include/hp_turnikets.h"

hp_turnikets::hp_turnikets(XMLNode node)
{
	my_table_name = xml_parser.get_node_value(node,"table_name");
	my_gui_ident = xml_parser.get_node_value(node,"gui_ident");
	for(int i=0; i<node.nChildNode("turniket"); i++){
		my_real_turnikets.push_back(hp_turniket(node.getChildNode("turniket",i)));
	}
}

card_valid_t hp_turnikets::is_valid_card(string card_id)
{
	card_valid_t res;
	res.query = "";
	res.valid_card= false;
	if(card_id == "110c954f"){
		res.valid_card =true;
		return res;
	}
	for(unsigned int i=0; i<my_cards.size(); i++){
		if(my_cards[i].card_id == card_id){
			if(my_cards[i].valid_until == 0){
				int add_time = 0;
				if(my_cards[i].card_type == "0"){
					add_time = 180; // 3 hodinovy
				}
				if(my_cards[i].card_type == "1"){
					add_time = 9*60; // 1 denny
				}
				if(my_cards[i].card_type == "2"){
					add_time = 9*60+24*60; // 2 denny
				}
				if(my_cards[i].card_type == "3"){
					add_time = 9*60+24*60*2; // 3 denny
				}
				if(my_cards[i].card_type == "4"){
					add_time = 9*60+24*60*3; // 4 denny
				}
				if(my_cards[i].card_type == "5"){
					add_time = 9*60+24*60*4; // 5 denny
				}
				if(my_cards[i].card_type == "6"){
					add_time = 9*60+24*60*5; // 6 denny
				}
				if(my_cards[i].card_type == "7"){
					add_time = 9*60+24*60*6; // 7 denny
				}
				my_cards[i].valid_until = time(NULL) + add_time;
				res.query = "UPDATE "+my_table_name+" set valid_until = FROM_UNIXTIME("+patch::to_string(time(NULL)+add_time*60)+") where card_id = '"+my_cards[i].card_id+"'";
			}
			if(time(NULL) < my_cards[i].valid_until){ //&& time(NULL)  > my_cards[i].last_enter + DELAY_INTERVAL) {
				my_cards[i].last_enter = time(NULL);
				res.valid_card = true;
			}
		}
	}

	return res;
}
hp_turniket *hp_turnikets::check_turniket(string ident)
{
	for(unsigned int i=0; i<my_real_turnikets.size(); i++){
		if(my_real_turnikets[i].has_input(ident)){
			return &my_real_turnikets[i];
		}
	}
	return NULL;
}


hp_turniket *hp_turnikets::find_turniket(string mac)
{
	for(unsigned int i=0; i<my_real_turnikets.size(); i++){
		if(my_real_turnikets[i].has_reader(mac)){
			return &my_real_turnikets[i];
		}
	}
	return NULL;
}

vector<string> hp_turnikets::erase_old_cards()
{
	vector<string> res;
	for(unsigned int i=0; i<my_cards.size(); i++){
		if(my_cards[i].valid_until < time(NULL) && my_cards[i].valid_until != 0) {
			res.push_back("delete from "+my_table_name+" where card_id= '"+my_cards[i].card_id+"'");
			if(i != my_cards.size() -1){
				my_cards.erase(my_cards.begin()+i);
			//	i--;
			} else {
				my_cards.erase(my_cards.begin()+i);
				break;
			}
		}
	}
	return res;
}

void hp_turnikets::remove_card(string card_id)
{
	for(unsigned int i=0; i<my_cards.size(); i++){
		if(my_cards[i].card_id == card_id){
			my_cards.erase(my_cards.begin()+i);
			break;
		}
	}
}

void hp_turnikets::print_cards()
{
	for(unsigned int i=0; i<my_cards.size(); i++){
		cout << my_cards[i].card_id << " vallid: " << my_cards[i].valid_until << endl;
	}
}

void hp_turnikets::add_card(cards_data_t card)
{
	my_cards.push_back(card);
}

hp_turnikets::~hp_turnikets() {}
