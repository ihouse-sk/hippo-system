#include "../include/hp_shutter.h"

hp_shutter::hp_shutter(XMLNode node , vector<hp_virtual_pin *> pins) : my_last_move_direction(SHUT_UP)
{
	my_gui_ident= xml_parser.get_node_value(node, "gui_ident");
	string dir = xml_parser.get_node_value(node, "dir");
	this->my_pin_dir = find_item(dir, pins);
	this->my_pin_on_off = find_item(xml_parser.get_node_value(node, "onoff"), pins);
	this->my_timer = patch::string2int(xml_parser.get_node_value(node, "timer"));
	this->my_position= patch::string2int(xml_parser.get_node_value(node, "position"));
	if(xml_parser.get_node_value(node,"buttons") == "yes"){
		for(int i=0; i<node.getChildNode("buttons").nChildNode("button"); i++){
			my_control_buttons.push_back(xml_parser.get_node_value(node.getChildNode("buttons"), "button", "value",i));
			/*
			int tmp = find_item(xml_parser.get_node_value(node.getChildNode("buttons"), "button", "value",i),pins);
			if(tmp != -1){
				this->my_control_buttons.push_back(tmp);
			}
			*/
		}
	}
/*
	cout << "pin_dir: "<< my_pin_dir << " pinonoff: "<< my_pin_on_off << " timer: " << my_timer << endl;
	for(unsigned int i=0; i<my_control_buttons.size(); i++){
		cout << "cpin: " << my_control_buttons[i] << endl;
	}
	*/

	my_tilt = 0;
	my_send_steps =0;
	my_status = !xml_parser.has_error();
}

bool hp_shutter::has_actor(string actor)
{
	for(unsigned int i=0; i<my_control_buttons.size(); i++){
		if(actor == my_control_buttons[i]){
			return true;
		}
	}
	return false;
}

int hp_shutter::get_mevement_steps(string dir)
{
	int res = -1;
	if(dir == SHUT_UP){ // viacej svetla
		res = my_tilt;
	} else if (dir == SHUT_DOWN){ // viacej tmy
		res = SHUTTER_MAX_TILT - my_tilt;
	}
	//cout <<"Direction: " << dir << " stepcount: " << res << endl;
	my_send_steps = res;
	if(SHUTTER_CONTROL == 1){
		res = 5;
	}
	return res;
}

void hp_shutter::set_tilt(int tilt)
{
	//cout <<"last move: " << my_last_move_direction << " tilt: " << tilt << " my_send_steps: " << my_send_steps << endl;
	if(my_last_move_direction == SHUT_DOWN){
		my_tilt += (my_send_steps - tilt) ;
	}
	if(my_last_move_direction == SHUT_UP){
		my_tilt -= (my_send_steps - tilt);
	}
}

int hp_shutter::find_item(string find, vector<hp_virtual_pin *> pins)
{
	int res = -1;
	for(unsigned int i=0; i<pins.size(); i++){
		if(pins[i]->get_desc() == find){
			res = i;
			break;
		}
	}
	return res;
}
hp_shutter::~hp_shutter() {}
