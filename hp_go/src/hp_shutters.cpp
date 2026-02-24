#include "../include/hp_shutters.h"

hp_shutters::hp_shutters(XMLNode node, vector<hp_virtual_pin *> pins ) 
{
	my_critical_speed = 0;
	all_ident = xml_parser.get_node_value(node, "all_ident");
	all_up_ident= xml_parser.get_node_value(node, "all_up_ident");
	all_down_ident= xml_parser.get_node_value(node, "all_down_ident");
	up_ident= xml_parser.get_node_value(node, "up_ident");
	down_ident= xml_parser.get_node_value(node, "down_ident");
	my_save_control_enabled = xml_parser.get_node_value(node, "save_control") == "yes"?true:false;
	my_direct_control = xml_parser.get_node_value(node, "direct_control") == "yes"?true:false;
	if(my_save_control_enabled){
		my_critical_speed = patch::string2int(xml_parser.get_node_value(node,"save_control","critical_speed"));
	}
	for(int i=0; i<node.nChildNode("shutter"); i++){
		my_shutters.push_back(new hp_shutter(node.getChildNode("shutter",i), pins));
	}
	my_enabled_shutt_move = true;
				
	my_status = !xml_parser.has_error();
}
hp_shutter *hp_shutters::find_shutter(string actor)
{
	if(!my_enabled_shutt_move){
		cout <<"Silny vietor, pohyb zaluzii zakazany... " << endl;
		return NULL;
	}
	for(unsigned int i=0; i<my_shutters.size(); i++){
		if(my_shutters[i]->get_gui_ident() == actor){ 
			if(my_shutters[i]->get_pin_dir() != -1 && my_shutters[i]->get_pin_on_off() != -1){
				return my_shutters[i];
			} else {
				cout << "Neplatna pozicia pre shutter pin,  onoffpin: " << my_shutters[i]->get_pin_on_off() << "\t dir pin: " << my_shutters[i]->get_pin_dir() << endl;
			}
		}
	}

	return NULL;
}
vector<hp_shutter *> hp_shutters::find_multi_rule(string actor)
{
	vector<hp_shutter *> res;
	if(!my_enabled_shutt_move){
		cout <<"Silny vietor, pohyb zaluzii zakazany... " << endl;
		return res;
	}
	for(unsigned int i=0; i<my_shutters.size(); i++){
		if(my_shutters[i]->has_actor(actor)){ 
			if(my_shutters[i]->get_pin_dir() != -1 && my_shutters[i]->get_pin_on_off() != -1){
				res.push_back(my_shutters[i]);
			} else {
				cout << "Neplatna pozicia pre shutter pin,  onoffpin: " << my_shutters[i]->get_pin_on_off() << "\t dir pin: " << my_shutters[i]->get_pin_dir() << endl;
			}
		}
	}
	return res;
}
const vector<hp_shutter *> hp_shutters::move_all_shutters()
{
	return my_shutters;
}

vector<hp_shutter *> hp_shutters::check_critical_wind(int speed)
{
	vector<hp_shutter *> res;
	if(my_save_control_enabled){
		//	return my_shutters;
		if(speed >= my_critical_speed){
			return my_shutters;
		} 
	}
	return res;
}

void hp_shutters::setup_shut_tilt(string shutt, int tilt)
{
	for(unsigned int i=0; i<my_shutters.size(); i++){
		if(my_shutters[i]->get_gui_ident() == shutt){
			my_shutters[i]->set_tilt(tilt);
		}
	}
}

bool hp_shutters::has_rule4pin(string actor)
{
	if(my_shutters.size() > 0){
		for(unsigned int i=0; i<my_shutters.size(); i++){
			if(my_shutters[i]->has_actor(actor)){
				return true;
			}
		}
	}

	return false;
}

hp_shutters::~hp_shutters() 
{
	for(unsigned int i=0; i<my_shutters.size(); i++){
		delete my_shutters[i];
	}
}
