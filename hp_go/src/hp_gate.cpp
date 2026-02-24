#include "../include/hp_gate.h"

hp_gate::hp_gate(XMLNode node)
{

	my_pin_down = "";
	my_pin_closed = "";

	my_lock_gui_ident = xml_parser.get_node_value(node, "lock", "ident");
	my_lock_pin_count = patch::string2int(xml_parser.get_node_value(node,"lock", "pin_count"));
	my_lock_control_type= xml_parser.get_node_value(node,"lock", "control_type");
	if(my_lock_pin_count == 1){
		my_pin_up= xml_parser.get_node_value(node.getChildNode("lock"), "pin_ident_up");
	} else if (my_lock_pin_count == 2){
		my_pin_up= xml_parser.get_node_value(node.getChildNode("lock"), "pin_ident_up");
		my_pin_down= xml_parser.get_node_value(node.getChildNode("lock"), "pin_ident_down");
	} else if (my_lock_pin_count == 3){
		my_pin_up= xml_parser.get_node_value(node.getChildNode("lock"), "pin_ident_up");
		my_pin_down= xml_parser.get_node_value(node.getChildNode("lock"), "pin_ident_down");
	}

	my_status_gui_ident = xml_parser.get_node_value(node,"state", "ident");
	my_status_pin_count = patch::string2int(xml_parser.get_node_value(node,"state", "pin_count"));
	my_status_control_type= xml_parser.get_node_value(node,"state", "control_type");
	if(my_status_pin_count == 1){
		my_pin_opened= xml_parser.get_node_value(node.getChildNode("state"), "mag_kontakt_open");
	} else if (my_status_pin_count == 2){
		my_pin_opened= xml_parser.get_node_value(node.getChildNode("state"), "mag_kontakt_open");
		my_pin_closed= xml_parser.get_node_value(node.getChildNode("state"), "mag_kontakt_close");
	} else if (my_status_pin_count == 3){
		my_pin_opened= xml_parser.get_node_value(node.getChildNode("state"), "mag_kontakt_open");
		my_pin_closed= xml_parser.get_node_value(node.getChildNode("state"), "mag_kontakt_close");
	}
	//cout << "Gate lock: " << this->my_lock_gui_ident << " lock pin_count: " << my_lock_pin_count << ", lock_up: " << my_pin_up << ", lock_down: " << my_pin_down << endl;
	//cout << "\tStatus: " << this->my_status_gui_ident << " status pin_count: " << my_status_pin_count<< ", status open: " << my_pin_opened<< ", status closed: " << my_pin_closed<< endl;
	
	my_state_opened = 0;
	my_state_closed = 0;
	my_gate_state = 2;
	my_lock_state = -1;
}
string hp_gate::get_report()
{
	string res ="";
	res += "gui lock: " + xml_parser.fill_str(my_lock_gui_ident,20) + "value: " +  xml_parser.fill_str(patch::to_string(my_lock_state),10);
	res += "gui state: " + xml_parser.fill_str(my_lock_gui_ident,20) + "value: " +  xml_parser.fill_str(patch::to_string(this->my_gate_state),10);

	if(my_status_pin_count == 1){
		res += "pin: " + xml_parser.fill_str(patch::to_string(this->my_state_opened),5);
	} else if (my_status_pin_count == 2){
		res += "pin open: " + xml_parser.fill_str(patch::to_string(this->my_state_opened),5) + "pin close: " + xml_parser.fill_str(patch::to_string(this->my_state_closed),5);
	} else if (my_status_pin_count == 3){
	}
	res += "\n";

	return res;
}


int hp_gate::setup_state(int value, string pin)
{
	if(my_status_pin_count == 0){
		my_gate_state = !my_gate_state;
		return my_gate_state;
	}
	if(my_status_control_type== GATE_STATUS_IMPULZ){
		if(my_status_pin_count == 1){
			my_gate_state=0;
		} else if (my_status_pin_count == 2){
			if(pin == my_pin_closed){
				if(value == 1){
					my_gate_state = 0;
				}
			} else if(pin == my_pin_opened){
				if(value == 1){
					my_gate_state = 1;
				}
			}
		} else if (my_status_pin_count == 3){
			/// DOPLNIT 3 pinovy vstup open,closed a ked je niekde medzi
		}
	} else if(my_status_control_type== GATE_STATUS_CHANGE){
		if(my_status_pin_count == 1){
			if(pin == my_pin_opened){
				if(value == 0){
					my_gate_state = 0;
				} else {
					my_gate_state = 1;
				}
			}
		} else if (my_status_pin_count == 2){
			if(pin == my_pin_closed){
				my_state_closed = value;
			} else if(pin == my_pin_opened){
				my_state_opened = value;
			}
			if(my_state_opened == 0 && my_state_closed == 0){
				my_gate_state = 2;
			} else {
				if(my_state_opened == 1){
					my_gate_state = 1;
				} else if (my_state_closed == 1){
					my_gate_state = 0;
				}
			}
		} else if (my_status_pin_count == 3){
			/// DOPLNIT 3 pinovy vstup open,closed a ked je niekde medzi
		}
	}
	return my_gate_state;
}

string hp_gate::get_move_ident (string dir)
{
	string res = "";
	if(my_lock_pin_count == 1){
		res = my_pin_up;
	} else if (my_lock_pin_count == 2){
		if(dir == "1"){
			res = my_pin_up;
		} else {
			res = my_pin_down;
		}
	}

	return res;
}

hp_gate::~hp_gate() {}
