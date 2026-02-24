#include "../include/hp_dali_pwm_light.h"


dali_pwm_light::dali_pwm_light(XMLNode item_node) : dali_item(item_node)
{
	//cout << get_dali_master_name()<<endl;
	my_query_cmd = get_node_value(item_node,"query_cmd"); 
	my_query_group = get_node_value(item_node,"query_group"); 
	my_query_dev_type = get_node_value(item_node,"query_dev_type"); 
	my_map_values.insert(std::pair<int,int>(0,0));
	my_map_values.insert(std::pair<int,int>(1,170));
	my_map_values.insert(std::pair<int,int>(2,195));
	my_map_values.insert(std::pair<int,int>(3,210));
	my_map_values.insert(std::pair<int,int>(4,220));
	my_map_values.insert(std::pair<int,int>(5,229));
	my_map_values.insert(std::pair<int,int>(6,235));
	my_map_values.insert(std::pair<int,int>(7,241));
	my_map_values.insert(std::pair<int,int>(8,246));
	my_map_values.insert(std::pair<int,int>(9,250));

	my_last_direction = "down";
}

string dali_pwm_light::get_last_direction()
{
	return my_last_direction;
}

int dali_pwm_light::set_actual_value(int value)
{
	int res = -1;
	for(unsigned int i=0; i<my_map_values.size(); i++){
		if(i<my_map_values.size()-1){
			if(value >= my_map_values[i] && value < my_map_values[i+1]){
				res = i;
				break;
			}
		} else {
			res = i;
		}
	}
	//cout << "Pozadovana hodnota v dali_pwm status na: " << value << " a nastavujem na: " << res <<" my_actual_value:" << my_actual_value  << " pre " << my_id << " " << my_desc <<  endl;
	if(res != my_actual_value){
		if(res != -1){
			my_actual_value = res;
		}
	} else {
		res = -1;
	}
	if(my_actual_value == 9){
		my_last_direction = "up";
	} else if(my_actual_value == 0){
		my_last_direction = "down";
	}
	return res;
}

string dali_pwm_light::create_mess(int send_value)
{
	string res = "";
	if(send_value != 0){
		send_value = my_map_values[send_value];
		//cout << "calculatet value: " << value << endl;
		if(send_value >= 250){
			res = my_id+"_on";
		} else {
			res = my_id+"_acr_"+patch::to_string(send_value);
		}
	} else {
		res = my_id+"_off";
	}

	return res;
}
dali_pwm_light::~dali_pwm_light()
{
}


