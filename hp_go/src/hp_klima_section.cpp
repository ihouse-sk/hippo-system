#include "../include/hp_klima_section.h"

hp_klima_section::hp_klima_section(XMLNode node)
{
	//my_all_ident = xml_parser.get_node_value(node, "all", "ident");
	//<section id="spalna" air_condition="klimaSpalna" predef_temp="21" ip="127.0.0.1" port="11111" temp="tempSpalna"/>
	this->my_ident =  node.getAttribute("id");
	this->my_klima_ident = node.getAttribute("air_condition");
	this->my_temp_ident=  node.getAttribute("temp");
	this->my_def_temp_value = patch::string2float(node.getAttribute("predef_temp"));
	this->my_actual_temp = my_def_temp_value;
	this->my_query_temp = my_def_temp_value;
	this->my_modbus_ip = node.getAttribute("ip");
	this->my_modbus_port= patch::string2int(string(node.getAttribute("port")));

	//cout <<"klima id: " << my_ident<< " temp: " << my_klima_ident << " ip: " << my_modbus_ip << endl;
}
hp_klima_section::~hp_klima_section() {}
