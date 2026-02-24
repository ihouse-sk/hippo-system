
#include "../include/hp_rekuperacia_zone.h"

hp_rekuperacia_zone::hp_rekuperacia_zone(XMLNode node, string ident) : my_ident(ident)
{
	for(int i=0; i<node.nChildNode("ident"); i++){
		rek_actor_t tmp;
		tmp.actor = xml_parser.get_node_value(node,"ident","value",i);
		tmp.to_value= xml_parser.get_node_value(node,"ident","to_value",i);
		my_actors.push_back(tmp);
	}
}
bool hp_rekuperacia_zone::is_active_zone(vector<rek_actor_t> all)
{
	for(unsigned int i=0; i<my_actors.size(); i++){
		for(unsigned int j=0; j<all.size(); j++){
			if(my_actors[i].actor == all[j].actor){
				if(my_actors[i].to_value != all[j].to_value){
					return false;
				}
			}
		}
	}

	return true;
}

hp_rekuperacia_zone::~hp_rekuperacia_zone() {}
