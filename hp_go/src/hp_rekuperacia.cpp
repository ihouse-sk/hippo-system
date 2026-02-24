#include "../include/hp_rekuperacia.h"

hp_rekuperacia::hp_rekuperacia(XMLNode node)
{
	my_active_mode = "";
	for(int i=0; i<node.nChildNode("mode"); i++){  // spracovanie sluzieb
		my_rek_zones.push_back(hp_rekuperacia_zone(node.getChildNode("mode",i), xml_parser.get_node_value(node, "mode", "ident",i)));
	//	my_rek_zones.push_back(hp_rekuperacia_zone(node.getChildNode("mode"), xml_parser.get_node_value(node, "mode", "ident")));
	//	my_rek_zones.push_back(hp_rekuperacia_zone(node.getChildNode("mode"), xml_parser.get_node_value(node, "mode", "ident")));
	//	my_rek_zones.push_back(hp_rekuperacia_zone(node.getChildNode("mode"), xml_parser.get_node_value(node, "mode", "ident")));
	}

	for(unsigned int i=0; i<my_rek_zones.size(); i++){
		vector<rek_actor_t> tmp = my_rek_zones[i].get_actors();
		for(unsigned j=0; j<tmp.size(); j++){
			bool add_new = true;
			for(unsigned k=0; k<my_all_actors.size(); k++){
				if(my_all_actors[k].actor == tmp[j].actor){
					add_new = false;
					break;
				}
			}
			if(add_new){
				my_all_actors.push_back(tmp[j]);
			}
		}
	}
	/*
	cout<<"My all actors: ";
	for(unsigned k=0; k<my_all_actors.size(); k++){
		cout <<my_all_actors[k].actor << ", ";
	}
	cout <<endl;
	*/
}

bool hp_rekuperacia::has_rek_pin(string ident)
{
	for(unsigned int i=0; i<my_all_actors.size(); i++){
		if(my_all_actors[i].actor == ident){
			return true;
		}
	}
	return false;
}

vector<string> hp_rekuperacia::sync_rek(const vector<hp_virtual_pin *> pins)
{
	vector<string> res;
	for(unsigned int i=0; i<my_all_actors.size(); i++){
		for(unsigned int j=0; j<pins.size(); j++){
			if(pins[j]->get_desc() == my_all_actors[i].actor){
				my_all_actors[i].to_value = pins[j]->get_status();
				break;
			}
		}
	}
	for(unsigned int i=0; i<my_rek_zones.size(); i++){
		if(my_rek_zones[i].is_active_zone(my_all_actors)){
			my_active_mode = my_rek_zones[i].get_ident();
		}
	}
	for(unsigned int i=0; i<my_rek_zones.size(); i++){
		if(my_rek_zones[i].get_ident() == my_active_mode){
			res.push_back("UPDATE STATUSES set STATUS = 1 where ITEM = '"+my_rek_zones[i].get_ident()+"'");
		} else {
			res.push_back("UPDATE STATUSES set STATUS = 0 where ITEM = '"+my_rek_zones[i].get_ident()+"'");
		}
	}
	return res;
}

hp_rekuperacia_zone *hp_rekuperacia::find_zone(string zone_ident)
{
	hp_rekuperacia_zone *zone=NULL;
	for(unsigned int i=0 ;i<my_rek_zones.size(); i++){
		if(my_rek_zones[i].get_ident() == zone_ident){
			return &my_rek_zones[i];
		}
	}

	return zone;
}
const vector<string> hp_rekuperacia::get_rek_zones_idents()
{
	vector<string> res;

	for(unsigned int i=0; i<my_rek_zones.size(); i++){
		res.push_back(my_rek_zones[i].get_ident());
	}
	return res;
}

hp_rekuperacia::~hp_rekuperacia() {}
