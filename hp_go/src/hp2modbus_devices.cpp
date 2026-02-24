#include "../include/hp2modbus_devices.h"


hp2modbus_devices::hp2modbus_devices(XMLNode node)
{
	hp_xml_parser par;
	my_ip = par.get_node_value(node, "ip");
	my_id = par.get_node_value(node, "id");
	my_type= par.get_node_value(node, "modbus_type");
	my_port = patch::string2int(par.get_node_value(node, "port"));

	cout <<"Id: " << my_id << ", ip: " << my_ip << ", type: " << my_type << ", port: " << my_port << endl;
}
