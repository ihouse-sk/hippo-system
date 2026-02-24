#include "../include/hp_watering_zone.h"

hp_watering_zone::hp_watering_zone(string ident)
{
	my_ident = ident;
	for(int i=0; i<WATERING_DAY_COUNT; i++){
		this->my_start_time.push_back(0);
		this->my_time_enabled.push_back(0);
	}
	for(int i=0; i<7; i++){
		this->my_week_day_enabled.push_back(0);
	}
	my_zone_type = ZONE_ALL;
	
	my_status = !xml_parser.has_error();
}

hp_watering_zone::hp_watering_zone(XMLNode node) 
{
	my_ident = node.getAttribute("ident");
	for(int i=0; i<node.nChildNode("sprinkler"); i++){
		my_actors.push_back(xml_parser.get_node_value(node, "sprinkler","value",i));
	}
	for(int i=0; i<WATERING_DAY_COUNT; i++){
		this->my_start_time.push_back(0);
		this->my_time_enabled.push_back(0);
	}
	for(int i=0; i<7; i++){
		this->my_week_day_enabled.push_back(0);
	}
	my_zone_type = ZONE_SINGLE;
	
	my_status = !xml_parser.has_error();
}

void hp_watering_zone::set_start_time(int value, int pos)
{
	if(pos >= 0 && my_start_time.size() > (unsigned int)pos){
		my_start_time[pos] = value;
	}
}


void hp_watering_zone::set_week_day_enabled(int value, int pos)
{
	if(pos >= 0 && my_week_day_enabled.size() > (unsigned int)pos){
		my_week_day_enabled[pos] = value;
	}
}
void hp_watering_zone::set_time_enabled(int value, int pos)
{
	if(pos >= 0 && my_time_enabled.size() > (unsigned int)pos){
		my_time_enabled[pos] = value;
	}
}

bool hp_watering_zone::is_end_time(int now)
{
	if(my_running){
		for(unsigned int i=0; i<my_start_time.size(); i++){
			if(my_start_time[i]+my_timer > 60*24){
				if(now == my_start_time[i]+my_timer - 60*24){
					return true;
				}
			} else if(now == my_start_time[i]+ my_timer){
				return true;
			}
		}
	}
	return false;
}

bool hp_watering_zone::is_start_time(int now)
{
	//cout <<"Zone: " << my_ident << " enabled: " << my_auto_enabled << endl;
	if(my_auto_enabled){
		for(unsigned int i=0; i<my_time_enabled.size(); i++){
		//	cout << my_time_enabled[i] << " ==  1 && " <<  my_start_time[i] << " == " << now << endl;
			if(my_time_enabled[i] == 1 && my_start_time[i] == now){
				return true;
			}
		}
	}
	return false;
}

void hp_watering_zone::setup_zone(int running, int auto_enabled, int timer, vector<int> start_time, vector<int> time_enabled, vector<int> week_day_enabled)
{
	my_running = running;
	my_auto_enabled = auto_enabled;
	my_timer = timer;
	my_start_time = start_time;
	my_time_enabled = time_enabled;
	my_week_day_enabled = week_day_enabled;
}

hp_watering_zone::~hp_watering_zone() {}
