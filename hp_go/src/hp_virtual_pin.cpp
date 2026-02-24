#include "../include/hp_virtual_pin.h"

hp_virtual_pin::hp_virtual_pin(XMLNode node, int hbx_pos) : my_hbx_pos(hbx_pos), my_pin_timer(-1), my_onOff_pin("")
{
	my_id= xml_parser.get_node_value(node, "id");
	my_desc = xml_parser.get_node_value(node, "desc");
	my_desc2 = xml_parser.get_node_value(node, "desc_2");
	my_function = xml_parser.get_node_value(node, "desc_2","function");
	my_type = xml_parser.get_node_value(node, "pin_type", "type");
	my_active_value = my_type[0] == 'i'?"0":"1";
	my_gui_desc = xml_parser.get_node_value(node, "gui_send", "ident");
	my_location= xml_parser.get_node_value(node, "location");
	my_gui_update = xml_parser.get_node_value(node, "gui_send")=="yes"?true:false;
	if(xml_parser.get_node_value(node,"timer","hold") != "no" && xml_parser.get_node_value(node,"timer","hold") != "0"){
		my_pin_timer = patch::string2int(xml_parser.get_node_value(node,"timer","hold"));
		if(xml_parser.get_node_value(node,"timer","repeat_option") != "no"){
			my_repeat_timer= patch::string2int(xml_parser.get_node_value(node,"timer","repeat_time"));
			my_max_triggers= patch::string2int(xml_parser.get_node_value(node,"timer","max_triggers"));
		} else {
			my_repeat_timer= -1;
			my_max_triggers = -1;
		}
	} else {
		my_pin_timer = -1;
		my_repeat_timer= -1;
		my_max_triggers = -1;
	}
	if(xml_parser.get_node_value(node,"connections","value") != "no"){
		for(int i=0; i<node.getChildNode("connections").nChildNode("connection"); i++){
			hp_conn_pin_t tmp;
			tmp.ident = xml_parser.get_node_value(node.getChildNode("connections"), "connection", "ident",i);
			tmp.to_value= xml_parser.get_node_value(node.getChildNode("connections"), "connection", "to_value",i);
			tmp.on_value = xml_parser.get_node_value(node.getChildNode("connections"), "connection", "on_value",i);
			if(tmp.on_value == ""){
				tmp.on_value = "change";
			}
			this->my_conn_pins.push_back(tmp);
		}
	}
	if(xml_parser.get_node_value(node,"onoffpin","value") != "no"){
		my_onOff_pin = xml_parser.get_node_value(node,"onoffpin", "ident");
	}

	my_scenaria_active = xml_parser.get_node_value(node, "desc_2","scenaria")=="yes"?true:false;
	my_alarm_active= xml_parser.get_node_value(node, "desc_2","alarm")=="yes"?true:false;

	if(my_function == "status_light"){
		my_int_type = PIN_STATUS_LIGHT;
		my_io_type = HP_PIN_OUTPUT;
	} else if (my_function == "pwm"){
		my_int_type = PIN_PWM;
		my_io_type = HP_PIN_OUTPUT;
	} else if(my_function == "shutter"){
		my_int_type = PIN_SHUTTER;
		my_io_type = HP_PIN_OUTPUT;
	} else if(my_function == "socket"){
		my_int_type= PIN_SOCKET;
		my_io_type = HP_PIN_OUTPUT;
	} else if(my_function == "electro_cons"){
		my_int_type = PIN_ELECTRO_CONS;
		my_io_type = HP_PIN_INPUT;
	} else if(my_function == "sprinkler"){
		my_int_type = PIN_SPRINKLER;
		my_io_type = HP_PIN_OUTPUT;
        } else if(my_function == "heater"){
		my_int_type = PIN_HEATER;
		my_io_type = HP_PIN_OUTPUT;
	} else if(my_function == "cooler"){
		my_int_type = PIN_COOLER;
		my_io_type = HP_PIN_OUTPUT;
	} else if(my_function == "gas_cons"){
		my_int_type = PIN_GAS_CONS;
		my_io_type = HP_PIN_INPUT;
	} else if(my_function == "doorLock" || my_function == "gate" || my_function == "gateDoublePin" || my_function == "fountain" || my_function == "ventilator"){
		my_int_type = PIN_GATES_ALL;
		my_io_type = HP_PIN_OUTPUT;
	} else if(my_function == "GSM"){
		my_int_type = PIN_GSM;
		my_io_type = HP_PIN_OUTPUT;
	} else {
		my_io_type = HP_PIN_INPUT;
		my_int_type = PIN_REST;
	}

	my_valid_status = !xml_parser.has_error();
	my_status = 0;
	my_shord_pushed = my_long_pushed = my_double_pushed = my_shutter_control = false;
	my_priorty_on = false;
	my_changed = false;
	//print_pin();
}
string hp_virtual_pin::get_report()
{
	string res;
	res = xml_parser.fill_str(this->my_desc,30)  + "stav: " + xml_parser.fill_str(patch::to_string(this->my_status),5) + "my_priorty_on: " + xml_parser.fill_str(patch::to_string(my_priorty_on),5) + xml_parser.fill_str(this->my_desc2)+ "\n";
	return res;
}
string hp_virtual_pin::get_update_string()
{
	string res ="";
	if(my_changed){
		res = "UPDATE STATUSES set STATUS = '"+patch::to_string(get_status())+"' where ITEM = '"+this->my_gui_desc+"'";
	}
	my_changed = false;
	return res;
}

void hp_virtual_pin::print_pin()
{
	cout <<"hbx_pos: " << my_hbx_pos << ", id: "<< my_id << ", desc: " << my_desc << ", desc2: " << my_desc2 << ", function: " << my_function << "  type: " << my_type << " active: " << my_active_value << ", iotype: " << this->my_io_type <<endl;//" short: " << my_shord_pushed << " long: " << my_long_pushed << " double: " << my_double_pushed<<  endl;
	if(my_conn_pins.size() > 0){
		cout << "Connection: " <<endl;
		for(unsigned int i=0; i<my_conn_pins.size(); i++){
			cout <<"ident: " << my_conn_pins[i].ident << " to_value: " << my_conn_pins[i].to_value << endl;
		}
	}
}

hp_virtual_pin::~hp_virtual_pin() {}
