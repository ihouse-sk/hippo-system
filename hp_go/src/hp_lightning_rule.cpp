#include "../include/hp_lightning_rule.h"

hp_lightning_rule::hp_lightning_rule(XMLNode node)
{
	my_on_value = "";
	my_actor = xml_parser.get_node_value(node,"actor","ident");
	my_pushed = atoi(xml_parser.get_node_value(node,"actor","pushed").c_str());
	my_hold = xml_parser.get_node_value(node,"actor","hold");
	my_on_value = xml_parser.get_node_value(node,"actor","on_value");
	my_rule_priority = xml_parser.get_node_value(node,"priority","value") == "yes"?true:false;
	//cout <<"Priority: " << my_rule_priority << " pre pravidol actora: " << my_actor << endl;
	if(string(xml_parser.get_node_value(node,"time", "restriction")) == "yes"){
		my_time_rest = true;
		my_start_time_rest = atoi(xml_parser.get_node_value(node.getChildNode("time"), "start", "hour").c_str())*60 + atoi(xml_parser.get_node_value(node.getChildNode("time"), "start", "min").c_str());
		my_end_time_rest = atoi(xml_parser.get_node_value(node.getChildNode("time"), "end", "hour").c_str())*60 + atoi(xml_parser.get_node_value(node.getChildNode("time"), "end", "min").c_str());
	} else {
		my_time_rest = false;
	}
	for(int i=0; i<node.nChildNode("light"); i++){
		hp_rule_light_t tmp;
		tmp.ident = xml_parser.get_node_value(node,"light","ident",i);
		tmp.to_value= xml_parser.get_node_value(node,"light","to_value",i);
		tmp.timer_type = xml_parser.get_node_value(node,"light","timer_type",i);
		if(tmp.timer_type == "timer"){
			tmp.timer_time = xml_parser.get_node_value(node,"light","timer_hold",i);
		} else {
			cout <<"timer type "<<tmp.timer_type<<"pre actor: " << my_actor << endl;
			int hour = patch::string2int(xml_parser.get_node_value(node,"light","timer_hod",i));
			int min = patch::string2int(xml_parser.get_node_value(node,"light","timer_min",i));
			tmp.timer_time = patch::to_string(hour*60+min);
		}
		tmp.my_pwm_on= xml_parser.get_node_value(node,"light","on_value",i);
		tmp.my_pwm_off= xml_parser.get_node_value(node,"light","off_value",i);
		my_outputs.push_back(tmp);
	}
	/*
	cout << "Actor: " << my_actor << " pushed: " << my_pushed << " hold: " << my_hold << " on_value: " << get_on_value()<< "\t\tlights: " << endl;
	for(unsigned int i=0; i< my_outputs.size(); i++){
		cout << my_outputs[i].ident << " to value: " << my_outputs[i].to_value << endl;
	}
	*/
}

bool hp_lightning_rule::is_valid()
{
	if(!my_time_rest){
		return true;
	}
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	int actual_time = timeinfo->tm_hour*60 + timeinfo->tm_min;
	if(actual_time >= my_start_time_rest && actual_time <= my_end_time_rest){
		return true;
	} else {
		return false;
	}
}

hp_lightning_rule::~hp_lightning_rule() {}
