#include "../include/hp_security.h"

hp_security::hp_security(XMLNode node)
{
	my_simulation_ident = "holiday";
	my_simulation_value = 0;
	my_all_ident = xml_parser.get_node_value(node, "all_ident");
	my_signalization_enabled = xml_parser.get_node_value(node,"signalization", "enabled") == "yes"?true:false;
	if(my_signalization_enabled){
		my_signalization_ident=xml_parser.get_node_value(node,"signalization", "ident");
	}
	for(int i=0; i<node.nChildNode("section"); i++){
		my_zones.push_back(hp_security_zone(node.getChildNode("section",i)));
	}
	my_status = !xml_parser.has_error();
	my_simulation_enabled = xml_parser.get_node_value(node,"simulation", "enabled") == "yes"?true:false;
	if(my_simulation_enabled){
		my_simulation_ident = xml_parser.get_node_value(node,"simulation", "ident");
	} else {
		my_simulation_ident = "";
	}
}

Json::Value hp_security::get_shm_data()
{
	Json::Value res;
	for(unsigned int i=0; i<my_zones.size(); i++){
	//	cout << my_zones[i].get_id() << ": " << my_zones[i].get_sec_status() << endl;
		res[res.size()]["id"] = my_zones[i].get_id();
		res[res.size()-1]["status"] = my_zones[i].get_sec_status();
		res[res.size()-1]["actor"] = my_zones[i].get_sec_status();
		res[res.size()-1]["alarmCountdown"] = my_zones[i].get_current_countdown();
	//	cout <<  (my_zones[i].get_id() +"_"+ patch::to_string(my_zones[i].get_sec_status()) + "_"+ patch::to_string(my_zones[i].get_current_countdown()) +";") << endl;
	}

	return res;
}
string hp_security::check_signalization()
{
	string res = "0";
	for(unsigned int i=0; i<my_zones.size(); i++){
		if(my_zones[i].get_sec_status() == 1){
			res= "1";
		}
	}

	return res;
}

bool hp_security::has_armed_zone()
{
	bool res = false;
	for(uint16_t i=0; i<this->my_zones.size(); i++){
		if(my_zones[i].get_sec_status() == 1){
			res = true;
			break;
		}
	}
	return res;
}
int hp_security::get_all_zone_status()
{
	int status = 1;
	for(unsigned int i=0; i<my_zones.size(); i++){
		if(my_zones[i].get_sec_status() == 0){
			status = 0;
		}
	}
	return status;
}
vector<string> hp_security::get_actors4armed_zones()
{
	vector<string> res;
	for(unsigned int i=0; i<my_zones.size(); i++){
		if(my_zones[i].get_sec_status() == 1){
			vector<string> tmp = my_zones[i].get_sensors();
			res.insert(res.end(),tmp.begin(), tmp.end());
		}
	}
	return res;
}

vector<string> hp_security::get_zones_ident()
{
	vector<string> res;
	for(unsigned int i=0; i<my_zones.size(); i++){
		res.push_back(my_zones[i].get_id());
	}
	return res;
}

string hp_security::get_zone_ident(int pos) 
{ 
	if(pos >= 0 && pos < (int)my_zones.size()) {
		return my_zones[pos].get_id();
	} else {
		return "";
	}
}

hp_security_zone *hp_security::chech_pir(string sensor)
{
	hp_security_zone *res = NULL;	
	for(unsigned int i=0; i<my_zones.size(); i++){
		if(my_zones[i].has_sensor(sensor)){
		//cout <<actor<< " == " << my_zones[i].get_actor() << " status: " << my_zones[i].get_sec_status() <<  endl; 
			return &my_zones[i];;
		}
	}
	return res;
}

hp_security_zone *hp_security::find_zone(string id)
{
	hp_security_zone *res = NULL;	
	for(unsigned int i=0; i<my_zones.size(); i++){
		//cout <<id << " == " << my_zones[i].get_id() << endl; 
		if(id == my_zones[i].get_id()){
			return &my_zones[i];;
		}
	}
	return res;
}

bool hp_security::is_active_zone(string zone_ident)
{
	bool res = false;

	for(unsigned int i = 0; i<my_zones.size(); i++){
		if(my_zones[i].get_id() == zone_ident){
			return true;
		}
	}

	return res;
}

hp_security::~hp_security() {}
