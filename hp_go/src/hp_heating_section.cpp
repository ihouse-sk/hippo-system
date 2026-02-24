#include "../include/hp_heating_section.h"

hp_heating_section::hp_heating_section(XMLNode node,float diff, int start_time)
{
	my_has_main_vents = false;
	my_id = xml_parser.get_node_value(node, "id");
	my_def_temp_value = patch::string2float(xml_parser.get_node_value(node, "default_temp"));
	my_ref_temp = xml_parser.get_node_value(node, "reference_temp");
	my_tempering_enabled = xml_parser.get_node_value(node, "tempering")=="yes"?true:false;
	my_direction = "up";
	my_heating_treshold = diff;
	my_tempering_time = patch::string2int(xml_parser.get_node_value(node, "tempering_time"));
	my_tempering_start = start_time;
	//cout <<"zone: " << my_id << " start time: " << my_tempering_start << endl;

	my_ref_temp_value = my_def_temp_value;
	my_query_temp = my_def_temp_value;
	for(int i=0; i<node.nChildNode("zone"); i++){
		this->my_heating_zones.push_back(hp2heating_zone(node.getChildNode("zone",i),my_def_temp_value));
	}
	
	if(xml_parser.get_node_value(node,"main_vents") == "yes"){
		my_has_main_vents = true;
		for(int i=0; i<node.getChildNode("main_vents").nChildNode("vent"); i++){
			my_main_vents.push_back(xml_parser.get_node_value(node.getChildNode("main_vents"), "vent", "ident",i));
		}
	}
	my_auto_query_temp = my_def_temp_value;
	my_tempering_actived = false;
}

void hp_heating_section::set_temp4section(float temp, string sensor)
{
	if(sensor == my_ref_temp){
		my_ref_temp_value = temp;
	}
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		if(my_heating_zones[i].get_sensor_ident() == sensor){
			my_heating_zones[i].set_actual_temp(temp);
		}
	}
}
int hp_heating_section::has_sensor(string name)
{
	if(this->my_ref_temp == name){
		return SENSOR_REFERENCE;
	}
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		if(my_heating_zones[i].get_sensor_ident() == name){
			return SENSOR_ZONE;
		}
	}
	return SENSOR_NON;
}

bool hp_heating_section::has_heater(string heater)
{
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		vector<string> heaters = my_heating_zones[i].get_heaters();
		for(unsigned int j=0; j<heaters.size(); j++){
			if(heaters[j] == heater){
				return true;
			}
		}
	}

	return false;
}

vector<hp_heating_data_t> hp_heating_section::get_tempering_data()
{
	vector<hp_heating_data_t> res;
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		if(my_heating_zones[i].get_heater_state() == 0){
			vector<string> heaters = my_heating_zones[i].get_heaters();
			for(unsigned int j=0; j<heaters.size(); j++){
				hp_heating_data_t tmp;
				tmp.value = "1";
				tmp.ident = heaters[j];
				tmp.type = ACTOR_HEATER;
				tmp.off_delay = this->my_tempering_time;
				res.push_back(tmp);
			}
		}
	}

	return res;
}

int hp_heating_section::check_tempering_start(int min, int heating_mode)
{
	if(!my_tempering_enabled || my_tempering_time == 0){
		return -1;
	}
	if(heating_mode == HEATING_MANUAL){
		//cout << "temp: " << my_ref_temp_value  << " > " << (my_query_temp+ HEATING_THRESH_TEMPERING) << endl;
		if(my_ref_temp_value  > (my_query_temp+ HEATING_THRESH_TEMPERING)){
			return -1;
		}
	}
	if(heating_mode == HEATING_AUTO){
		if(my_ref_temp_value  > (my_auto_query_temp+ HEATING_THRESH_TEMPERING)){
			return -1;
		}
	}
		//|| this->my_heating_mode == HEATING_ECO || this->my_heating_mode == HEATING_AUTO)
	while(min >= HEATING_OSC_PERIOD){
		min -= HEATING_OSC_PERIOD;
	} 
	//cout << "\n\nheater state: " << my_heater_state << " pre zonu: " << my_id<< " min: " << min << " my_start: " << my_tempering_start <<endl;
	if(my_heater_state == 1){
		return -1;
	}
	if(min == this->my_tempering_start){
		my_tempering_actived = true;
		return my_tempering_time;
	}
	return -1;
}

vector<string> hp_heating_section::get_sensors()
{
	vector<string> res;
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		res.push_back(my_heating_zones[i].get_sensor_ident());
	}
	return res;
}
vector<hp_heating_data_t> hp_heating_section::check_windows(string ident, int value)
{
	vector<hp_heating_data_t> res;
	for(auto i : this->my_heating_zones){
		if(i.set_blocked(ident, value)){
			vector<string> heaters = i.get_heaters();
			for(auto j: heaters){
				hp_heating_data_t tmp;
				tmp.value = i.is_blocked()?"0":"1";
			}
		}
	}
	return res;
}

vector<hp_heating_data_t> hp_heating_section::process_section(int mode, int eco_temp,string sensor_name)//, bool heating_mode_changed)
{
	vector<hp_heating_data_t> res;
	string to_value = "";
	stringstream debug;
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		if(sensor_name != "" && sensor_name != my_heating_zones[i].get_sensor_ident()){
			continue;
		}

		if(mode == HEATING_ECO){
			if(my_heating_zones[i].get_actual_temp() < eco_temp){
				to_value = "1";
			} else {
				to_value = "0";
			}
			vector<string> heaters = my_heating_zones[i].get_heaters();
			for(unsigned int j=0; j<heaters.size(); j++){
				hp_heating_data_t tmp;
				tmp.value = to_value;
				tmp.ident = heaters[j];
				tmp.type = ACTOR_HEATER;
				res.push_back(tmp);
			}
			if(my_heating_zones[i].get_cooler_state()){
				my_heating_zones[i].set_cooler_state(0);
				heaters = my_heating_zones[i].get_collers();
				for(unsigned int j=0; j<heaters.size(); j++){
					hp_heating_data_t tmp;
					tmp.value = "0";
					tmp.ident = heaters[j];
					tmp.type = ACTOR_COOLER;
					res.push_back(tmp);
				}
			}
			if(to_value == "1"){
				my_heating_zones[i].set_heater_state(1);
			} else {
				my_heating_zones[i].set_heater_state(0);
			}
		} else if(mode == HEATING_COOLING){
			//cout <<"Last direction: " << my_heating_zones[i].get_direction() << endl;
			if(my_heating_zones[i].get_direction() == "down"){
				if(my_heating_zones[i].get_actual_temp() > this->my_query_temp){
					to_value = "1";
				} else {
					my_heating_zones[i].set_direction("up");
					to_value = "0";
				}
			} else {
				if(my_heating_zones[i].get_actual_temp()-my_heating_treshold > this->my_query_temp){
					to_value = "1";
					my_heating_zones[i].set_direction("down");
				} else {
					to_value = "0";
				}
			}
			if(to_value == "1"){
				my_heating_zones[i].set_cooler_state(1);
			} else {
				my_heating_zones[i].set_cooler_state(0);
			}

			//cout <<"tovalue: " << to_value<< " bala: " << my_heating_zones[i].get_actual_temp() << " < " << this->my_query_temp << "Last direction: " << my_heating_zones[i].get_direction() <<endl;
			vector<string> heaters = my_heating_zones[i].get_collers();
			for(unsigned int j=0; j<heaters.size(); j++){
				hp_heating_data_t tmp;
				tmp.value = to_value;
				tmp.ident = heaters[j];
				tmp.type = ACTOR_COOLER;
				res.push_back(tmp);
			}
			if(my_heating_zones[i].get_heater_state()){
				my_heating_zones[i].set_heater_state(0);
				heaters = my_heating_zones[i].get_heaters();
				for(unsigned int j=0; j<heaters.size(); j++){
					hp_heating_data_t tmp;
					tmp.value = "0";
					tmp.ident = heaters[j];
					tmp.type = ACTOR_HEATER;
					res.push_back(tmp);
				}
			}
		} else if(mode == HEATING_OFF) {
			vector<string> heaters = my_heating_zones[i].get_heaters();
			my_heating_zones[i].set_heater_state(0);
			my_heating_zones[i].set_cooler_state(0);
			for(unsigned int j=0; j<heaters.size(); j++){
				hp_heating_data_t tmp;
				tmp.value = "0";
				tmp.ident = heaters[j];
				tmp.type = ACTOR_HEATER;
				res.push_back(tmp);
			}
			heaters = my_heating_zones[i].get_collers();
			for(unsigned int j=0; j<heaters.size(); j++){
				hp_heating_data_t tmp;
				tmp.value = "0";
				tmp.ident = heaters[j];
				tmp.type = ACTOR_COOLER;
				res.push_back(tmp);
			}
		} else {
			float query_temp;
			if(mode == HEATING_AUTO){
				query_temp = this->my_auto_query_temp;
			} else {
				query_temp = this->my_query_temp;
			}
			if(my_heating_zones[i].get_direction() == "up"){
				if(my_heating_zones[i].get_actual_temp() < query_temp){
					debug << "HEATING_MANUAL, actual: " << my_heating_zones[i].get_actual_temp() << "< query: " << query_temp << " to valiue: 1, dir: up";
					to_value = "1";
				} else {
					debug << "HEATING_MANUAL, actual: " << my_heating_zones[i].get_actual_temp() << ">= query: " << query_temp << " to valiue: 0, dir: up-changing to down";
					my_heating_zones[i].set_direction("down");
					to_value = "0";
				}
			} else {
				if(my_heating_zones[i].get_actual_temp()+my_heating_treshold< query_temp){
					to_value = "1";
					debug << "HEATING_MANUAL, actual: " << my_heating_zones[i].get_actual_temp() << "+" << my_heating_treshold << "< query: " << query_temp << " to valiue: 1, dir: down-changing to up";
					my_heating_zones[i].set_direction("up");
				} else {
					to_value = "0";
					debug << "HEATING_MANUAL, actual: " << my_heating_zones[i].get_actual_temp() << ">= query: " << query_temp << " to valiue: 0, dir: down";
				}
			}
		//	cout << "Ident: " << my_id << " mode: " << mode << " sensor: " << my_heating_zones[i].get_sensor_ident() << " temps: " << my_heating_zones[i].get_actual_temp() << " query: "<<  this->my_query_temp << " direction: " << my_heating_zones[i].get_direction() << " to_value: " << to_value <<endl;
			if(to_value == "1"){
				my_heating_zones[i].set_heater_state(1);
			} else {
				my_heating_zones[i].set_heater_state(0);
			}

			//cout <<"tovalue: " << to_value<< " bala: " << my_heating_zones[i].get_actual_temp() << " < " << this->my_query_temp << endl;
			vector<string> heaters = my_heating_zones[i].get_heaters();
			for(unsigned int j=0; j<heaters.size(); j++){
				hp_heating_data_t tmp;
				tmp.value = to_value;
				tmp.ident = heaters[j];
				tmp.type = ACTOR_HEATER;
				tmp.debug_data = debug.str();
				res.push_back(tmp);
			}
			if(my_heating_zones[i].get_cooler_state()){
				my_heating_zones[i].set_cooler_state(0);
				heaters = my_heating_zones[i].get_collers();
				for(unsigned int j=0; j<heaters.size(); j++){
					hp_heating_data_t tmp;
					tmp.value = "0";
					tmp.ident = heaters[j];
					tmp.type = ACTOR_COOLER;
					res.push_back(tmp);
				}
			}

		}
	}
	my_heater_state = 0;
	my_cooler_state = 0;
	for(unsigned int i=0; i<my_heating_zones.size(); i++){
		//cout <<"Heating zone state pre senzor: " << my_heating_zones[i].get_sensor_ident()  << " je: " << my_heating_zones[i].get_heater_state() << endl;
		if(my_heating_zones[i].get_heater_state() == 1){
			my_heater_state = 1;
		}
		if(my_heating_zones[i].get_cooler_state() == 1){
			my_cooler_state= 1;
		}
	}

	/*
	for(unsigned int i=0; i<res.size(); i++){
		if(res[i].type == ACTOR_HEATER && res[i].value == "1"){
			my_heater_state = 1;
		}
		if(res[i].type == ACTOR_COOLER && res[i].value == "1"){
			my_cooler_state = 1;
		}
	}
	*/

	return res;
}
string hp_heating_section::get_section_state()
{
	if(my_heater_state == 1){
		return "1";
	} else if(my_cooler_state == 1){
		return "2";
	} else {
		return "0";
	}
}

hp_heating_section::~hp_heating_section() {}
