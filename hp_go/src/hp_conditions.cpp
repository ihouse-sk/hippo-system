#include "../include/hp_condition.h"

hp_conditions::hp_conditions(XMLNode node, hp_db_data_t *data)
{
	//my_security_notification= xml_parser.get_node_value(node, "security_notification")=="yes"?true:false;
	for(int i=0; i<node.nChildNode("condition"); i++){
		my_conditions.push_back(hp_condition(node.getChildNode("condition",i)));
	}
	my_db_data = data;
}

void hp_conditions::setup_items_position(const vector<hp_virtual_pin *> pins)
{
	for(uint16_t j=0; j<this->my_conditions.size(); j++){
		for(auto i = pins.begin(); i != pins.end(); ++i){
			my_conditions[j].setup_item((*i)->get_desc(), std::distance(pins.begin(), i), (*i)->get_status());
		}
	}
}

vector<cond_struc_t> hp_conditions::check_condition(int pos, string value)
{
	//cout <<"pos: " << pos << ", value: " << value << endl;
	vector<cond_struc_t> res;
	for(uint16_t j=0; j<this->my_conditions.size(); j++){
		if(my_conditions[j].has_effect(pos,value)){
			res.push_back(my_conditions[j].get_cond_data());
		}
	}
	return res;
}

void hp_conditions::push_db_query(string query, int type, int log_level)
{
	if(my_db_data != NULL){
		hp_db_queries_t tmp;
		tmp.query = query;
		tmp.type = type;
		tmp.log_level = log_level;
	
		my_db_data->mtx.lock();
		my_db_data->queries.push_back(tmp);
		my_db_data->mtx.unlock();
	}
}



hp_conditions::~hp_conditions() {}
