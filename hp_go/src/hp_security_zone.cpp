#include "../include/hp_security_zone.h"

hp_security_zone::hp_security_zone(XMLNode node) 
{
	my_ident = node.getAttribute("id");
	my_wait_time  = patch::string2int(xml_parser.get_node_value(node, "wait_time"));
	my_repeated_action = xml_parser.get_node_value(node, "repeated_action") == "yes"?true:false;
	my_repeat_triggers = patch::string2int(xml_parser.get_node_value(node, "repeated_action", "max_triggers"));

//	cout <<"My ident zone: "<< my_ident << " Actors: " << endl;
	for(int i=0; i<node.nChildNode("actor"); i++){
		my_actors.push_back(xml_parser.get_node_value(node, "actor", "ident", i));
		//cout << xml_parser.get_node_value(node, "actor", "ident", i) << endl;
	}
//	cout << "Sensors: " << endl;
	for(int i=0; i<node.nChildNode("sensor"); i++){
		my_sensors.push_back(xml_parser.get_node_value(node, "sensor", "ident", i));
	//	cout << xml_parser.get_node_value(node, "sensor", "ident", i) << endl;
	}
	my_sec_status = 0;
	my_countdown_counter = -1;
	my_status = !xml_parser.has_error();
	my_last_check_time = time(NULL)-1000;
	my_next_repeat= time(NULL);
	my_repeated_counter = 0;
}

bool hp_security_zone::repeat_action()
{
	if(my_repeated_action){
		if(my_next_repeat > time(NULL)){
			return false;
		}
		//cout << my_repeat_triggers << ", counter: " << my_repeated_counter  << endl;
		if(my_repeat_triggers == 0){
			return true;
		} else {
			if(my_repeated_counter > my_repeat_triggers){
				return false;
			} else {
				my_repeated_counter++;
				return true;
			}
		}
	}else {
		return false;
	}
}

bool hp_security_zone::has_sensor(string sensor)
{
	if(this->my_sec_status == 1 || this->my_sec_status == 2){
		for(unsigned int i=0; i<my_sensors.size(); i++){
			if(my_sensors[i] == sensor){
				if(my_last_check_time + 60 > time(NULL)){
				//	cout <<"Druhy pohyb v zone.. poplach... " << endl;
					return true;
				} else {
					//cout <<"Prvy pohyb v zone: " << my_ident << " v case: " << time(NULL) << " last check time: " << my_last_check_time << endl;
				}
				my_last_check_time = time(NULL);
			}
		}
	}
	return false;
}

int hp_security_zone::decrease_counter()
{
	//cout <<"zona: " << my_ident << " counter: " << my_countdown_counter << endl;
	if(my_countdown_counter > 0) {
		my_countdown_counter--;
	}

	return my_countdown_counter;
}

hp_security_zone::~hp_security_zone() {}
