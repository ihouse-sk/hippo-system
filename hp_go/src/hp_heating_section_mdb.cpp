
#include "../include/hp_heating_section_mdb.h"

hp_heating_section_mdb::hp_heating_section_mdb(XMLNode node)
{
	my_id = xml_parser.get_node_value(node, "id");
	my_def_temp_value = patch::string2float(xml_parser.get_node_value(node, "default_temp"));

	my_ref_temp_value = my_def_temp_value;
	my_query_temp = my_def_temp_value;
	my_auto_query_temp = my_def_temp_value;


	my_master_name = xml_parser.get_node_value(node, "master_name");
	my_read_address = patch::string2int(xml_parser.get_node_value(node.getChildNode("read_data"), "start_address"));
	my_funct_code = patch::string2int(xml_parser.get_node_value(node.getChildNode("read_data"), "function_code"));
	my_read_coils_num = node.getChildNode("read_data").getChildNode("coils").nChildNode("coil");
	for(uint16_t i=0; i<my_read_coils_num; i++){
	}
}

void hp_heating_section_mdb::set_temp4section(float temp)
{
	
}

string hp_heating_section_mdb::get_section_state()
{
	if(my_heater_state == 1){
		return "1";
	} else if(my_cooler_state == 1){
		return "2";
	} else {
		return "0";
	}
}

/*
vector<hp_heating_data_t> hp_heating_section_mdb::process_section(int mode, int eco_temp,string sensor_name)//, bool heating_mode_changed)
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

	return res;
}
*/

hp_heating_section_mdb::~hp_heating_section_mdb() {}
