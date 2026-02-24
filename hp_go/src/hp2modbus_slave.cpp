#include "../include/hp2modbus_slave.h"


hp2modbus_slave::hp2modbus_slave(XMLNode node)
{
	hp_xml_parser par;
	my_desc= par.get_node_value(node, "desc");
	my_address= patch::string2int(par.get_node_value(node, "address"));
	my_register = patch::string2int(par.get_node_value(node, "register"));
	my_desc2= par.get_node_value(node, "desc", "desc_2");
	my_type = par.get_node_value(node, "type");
	this->my_register_size = patch::string2int(par.get_node_value(node, "type", "size"));
	this->my_autoread_time = patch::string2int(par.get_node_value(node, "autoread"));
	my_master_id= par.get_node_value(node, "master");

	my_status = 0;
	my_autoread_send = false;
	my_autoread_last = time(NULL) - my_autoread_time/2;

	cout <<"Desc: " << my_desc << ", my master: " << my_master_id << ", address: " << my_address << ", auto time: " << my_autoread_time <<  endl;
}

bool hp2modbus_slave::run_autoread()
{
	bool res = false;
	if(my_autoread_time){
		if(time(NULL) >= my_autoread_last + my_autoread_time){
			if(!my_autoread_send){
				my_autoread_last = time(NULL);
				my_autoread_send = true;
				res  = true;
			}
		} else {
			my_autoread_send = false;
		}
	} 
	return res;
}

