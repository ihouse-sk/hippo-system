#include "../include/hp_pin_notifier.h"

hp_pin_notifier::hp_pin_notifier(XMLNode node)
{
	my_ident= my_xml_parser.get_node_value(node,"id");
	my_type= my_xml_parser.get_node_value(node,"type");
	if(my_type == "php"){
		my_link= my_xml_parser.get_node_value(node,"link");
	} else if(my_type == "socket"){
		my_ip= my_xml_parser.get_node_value(node,"link", "ip");
		my_port= my_xml_parser.get_node_value(node,"link", "port");
	}

}
string hp_pin_notifier::get_url(string value)
{
	//link value="http://178.143.18.28:20080/Miro_novy/"/>
	string res = "";
	res = my_link + "values_writer.php?key1=addData&mess="+my_ident+"_"+value;
	return res;
}

hp_pin_notifier::~hp_pin_notifier() {}
