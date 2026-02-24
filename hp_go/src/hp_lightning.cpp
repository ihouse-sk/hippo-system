#include "../include/hp_lightning.h"

hp_lightning::hp_lightning(XMLNode node)
{
	my_id = node.getAttribute("id");
	my_actual_mode = "normal";
	my_gui_id = "";
	if(xml_parser.get_node_value(node,"day_night_mode") == "yes"){
		my_dn_active = true;
		my_dn_type = xml_parser.get_node_value(node.getChildNode("day_night_mode"), "type");
		my_gui_id= xml_parser.get_node_value(node.getChildNode("day_night_mode"), "gui");
		if(my_dn_type == "time"){
			string day_start = xml_parser.get_node_value(node.getChildNode("day_night_mode"), "day", "from_hod");
			string day_end= xml_parser.get_node_value(node.getChildNode("day_night_mode"), "day", "to_hod");
			string night_start = xml_parser.get_node_value(node.getChildNode("day_night_mode"), "night", "from_hod");
			string night_end= xml_parser.get_node_value(node.getChildNode("day_night_mode"), "night", "to_hod");
			my_day_start = 60*patch::string2int(day_start.substr(0,day_start.find(":"))) + patch::string2int(day_start.substr(day_start.find(":")+1));
			my_day_end= 60*patch::string2int(day_end.substr(0,day_end.find(":"))) + patch::string2int(day_end.substr(day_end.find(":")+1));
			my_night_start= 60*patch::string2int(night_start .substr(0,night_start .find(":"))) + patch::string2int(night_start .substr(night_start .find(":")+1));
			my_night_end= 60*patch::string2int(night_end.substr(0,night_end.find(":"))) + patch::string2int(night_end.substr(night_end.find(":")+1));
			for(uint16_t i=0; i<3600; i++){
			}
			int now;
			time_t rawtime;
			struct tm * timeinfo;
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
			now = timeinfo->tm_hour*60 + timeinfo->tm_min;
			bool dn_valid = false;
			if(my_day_end  < my_day_start){
				if(now < my_day_end || now > my_day_start){
					my_actual_mode = "day";
					dn_valid = true;
				}
			} else {
				if(now > my_day_start && now <= my_day_end){
					if(!dn_valid){
						my_actual_mode = "day";
						dn_valid = true;
					}
				}
			}
			/// test ci sme v noci
			if(my_night_end < my_night_start){
				if(now > my_night_start || now < my_night_end){
					if(!dn_valid){
						my_actual_mode = "night";
						dn_valid = true;
					}
				}
			} else {
				if(now > my_night_start && now <= my_day_end){
					my_actual_mode = "night";
				}
			}
			//cout << "now: " << now << "   ," << my_day_start<< "," << my_day_end << ", night: " << my_night_start << "," << my_night_end<<endl;
		//	cout <<"my actual mode: " << my_actual_mode << endl;
		} else if (my_dn_type == "pin"){
			my_dn_actor_ident = xml_parser.get_node_value(node.getChildNode("day_night_mode"), "ident");
			//cout <<"Pin for dn change: " << my_dn_actor_ident << endl;
		}
		my_day_value= patch::string2int(xml_parser.get_node_value(node.getChildNode("day_night_mode"), "day_value"));
		my_night_value= patch::string2int(xml_parser.get_node_value(node.getChildNode("day_night_mode"), "night_value"));
	} else {
		my_dn_active = false;
	}

	for(int i=0; i<node.nChildNode("rule"); i++){
		string rule_mode = node.getChildNode("rule",i).getAttribute("mode");
		if(rule_mode == "normal"){
			my_normal_rules.push_back(hp_lightning_rule(node.getChildNode("rule",i)));
		}
		if(rule_mode == "day"){
			my_day_rules.push_back(hp_lightning_rule(node.getChildNode("rule",i)));
		}
		if(rule_mode == "night"){
			my_night_rules.push_back(hp_lightning_rule(node.getChildNode("rule",i)));
		}
	}
	for(unsigned int i=0; i<my_normal_rules.size(); i++){
	//	cout << my_normal_rules[i].get_actor() << " hold: " << my_normal_rules[i].get_hold() << " pushed: " << my_normal_rules[i].get_pushed() << endl;
	}
	for(unsigned int i=0; i<my_day_rules.size(); i++){
	//	cout << my_day_rules[i].get_actor() << " hold: " << my_day_rules[i].get_hold() << " pushed: " << my_day_rules[i].get_pushed() << endl;
	}
}
vector<bool> hp_lightning::get_rules4pin(string desc, string mode)
{
	//cout <<"get rules for: " << desc  << " mode: " << mode << endl;
	vector<bool> res;
	unsigned int i;
	for(i=0; i<3; i++){
		res.push_back(false);
	}
	for(i=0; i<my_normal_rules.size(); i++){
		if(my_normal_rules[i].get_actor() == desc){
			if(my_normal_rules[i].get_pushed() == 2){
				res[2] = true;
			}
			if(my_normal_rules[i].get_hold() == "short"){
				res[0] = true;
			}
			if(my_normal_rules[i].get_hold() == "long"){
				res[1] = true;
			}
		}
	}
	if(mode == "night"){
		for(i=0; i<my_night_rules.size(); i++){
			if(my_night_rules[i].get_actor() == desc){
				if(my_night_rules[i].get_pushed() == 2){
					res[2] = true;
				}
				if(my_night_rules[i].get_hold() == "short"){
					res[0] = true;
				}
				if(my_night_rules[i].get_hold() == "long"){
					res[1] = true;
				}
			}
		}
	}
	if(mode == "day"){
		for(i=0; i<my_day_rules.size(); i++){
			if(my_day_rules[i].get_actor() == desc){
				if(my_day_rules[i].get_pushed() == 2){
					res[2] = true;
				}
				if(my_day_rules[i].get_hold() == "short"){
					res[0] = true;
				}
				if(my_day_rules[i].get_hold() == "long"){
					res[1] = true;
				}
			}
		}
	}
	return res;
}
std::pair<string,string> hp_lightning::check_dn_change(string ident, int value )
{
	std::pair<string,string> res;
	res=std::make_pair<string,string>("","");
	if(my_dn_active){
		string new_mode = "";
		int new_mode_int = 2;
		if(my_dn_type == "time"){
			int now;
			time_t rawtime;
			struct tm * timeinfo;
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
			now = timeinfo->tm_hour*60 + timeinfo->tm_min;
			if(my_day_end == now || my_night_end == now){
				new_mode = "normal";
				new_mode_int = 2;
			}

			if(my_day_start == now){
				new_mode = "day";
				new_mode_int = 0;
			}
			if(my_night_start == now){
				new_mode = "night";
				new_mode_int = 1;
			}

			/*
			if(my_day_end < my_day_start){
				if(now >= my_day_start && now < 24*60){
					new_mode = "day";
				} else if(now <= my_day_end){
					new_mode = "day";
				}
			} else if(now >= my_day_start && now <= my_day_end){
				new_mode = "day";
			}
			
			if(my_night_end < my_night_start){
				if(now >= my_night_start && now < 24*60){
					new_mode = "night";
				} else if(now <= my_night_end){
					new_mode = "night";
				}
			} else if(now >= my_night_start && now <= my_night_end){
				new_mode = "night";
			}
			*/
		//	cout <<"now: " << now << " ds: " << my_day_start << " de: " << my_day_end << " ns: " << my_night_start << " ne: "<< my_night_end << endl<<"Novy mode: " << new_mode << " aktualmode: "<< my_actual_mode << endl;
			
		} else if (my_dn_type == "pin"){
			if(ident == my_dn_actor_ident){
				if(my_day_value == value){
					new_mode = "day";
				}
				if(my_night_value == value){
					new_mode = "night";
				}
			}
		}
		if(new_mode != ""){
			if(new_mode != my_actual_mode){
				my_actual_mode = new_mode;
				res=std::make_pair<string,string>(string(my_gui_id),patch::to_string(new_mode_int));
			}
		} 
	}

	return res;
}
int hp_lightning:: get_actual_int_mode()
{	
	int res = 2;
	if(this->my_actual_mode == "night"){
		res = 1;
	}
	if(this->my_actual_mode == "day"){
		res = 0;
	}
	return res;
}

void hp_lightning::set_actual_mode(string value)
{
	int ivalue = patch::string2int(value);
	if(ivalue == 0){
		my_actual_mode = "day";
	}
	if(ivalue == 1){
		my_actual_mode = "night";
	}
	if(ivalue == 2){
		my_actual_mode = "normal";
	}
}

hp_lightning_rule *hp_lightning::find_rule(string actor, int pushed, string hold, string on_value)
{
	//cout <<"Looking rule for : " << actor << " day_mode: " << my_actual_mode << " size: " << my_day_rules.size() <<  endl;
	unsigned int i;
	if(my_actual_mode == "day"){
		for(i=0; i<my_day_rules.size(); i++){
		//	cout <<  on_value << " == " << my_day_rules[i].get_on_value() <<  endl;
			if(my_day_rules[i].get_actor() == actor && my_day_rules[i].get_pushed() == pushed && my_day_rules[i].get_hold() == hold && on_value == my_day_rules[i].get_on_value()){
		//	if(my_day_rules[i].get_actor() == actor && my_day_rules[i].get_pushed() == pushed && my_day_rules[i].get_hold() == hold ){
				if(my_day_rules[i].is_valid()){
					return &my_day_rules[i];
				}
			}
		}
	}
	if(my_actual_mode == "night"){
		for(i=0; i<my_night_rules.size(); i++){
	//		cout<<  my_night_rules[i].get_actor()  << " == " <<  actor << " && " <<  my_night_rules[i].get_pushed() << " == " <<  pushed << " && " <<  my_night_rules[i].get_hold() << " == " << hold << endl;
			if(my_night_rules[i].get_actor() == actor && my_night_rules[i].get_pushed() == pushed && my_night_rules[i].get_hold() == hold&& on_value == my_night_rules[i].get_on_value()){
				if(my_night_rules[i].is_valid()){
					return &my_night_rules[i];
				}
			}
		}
	}
	for(i=0; i<my_normal_rules.size(); i++){
		//cout<<  my_normal_rules[i].get_actor()  << " == " <<  actor << " && " <<  my_normal_rules[i].get_pushed() << " == " <<  pushed << " && " <<  my_normal_rules[i].get_hold() << " == " << hold << endl;
		if(my_normal_rules[i].get_actor() == actor && my_normal_rules[i].get_pushed() == pushed && my_normal_rules[i].get_hold() == hold && (on_value == my_normal_rules[i].get_on_value() || my_normal_rules[i].get_on_value() == "")){
			if(my_normal_rules[i].is_valid()){
				return &my_normal_rules[i];
			}
		}
	}
	return NULL;
}

hp_lightning::~hp_lightning() 
{
	this->my_normal_rules.clear();
	this->my_day_rules.clear();
	this->my_night_rules.clear();
}
