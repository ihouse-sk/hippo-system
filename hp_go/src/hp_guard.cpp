#include "../include/hp_guard.h"

hp_guard::hp_guard(XMLNode node)
{
	my_check_time = -1;
	my_critical_value = 0;
	my_status = false;
	my_last_change_time = time(NULL);
	my_pin_position = -1;

	my_ident= my_xml_parser.get_node_value(node,"id");
	my_type= my_xml_parser.get_node_value(node,"type");
	my_guard_text= my_xml_parser.get_node_value(node,"guard_text");
	my_guard_text_ok= my_xml_parser.get_node_value(node,"guard_text_ok");
	my_unit = my_xml_parser.get_node_value(node,"unit");
	if(my_type == "time"){
		my_check_time = patch::string2int(my_xml_parser.get_node_value(node, "type", "check_time"))*60;
	} else  if(my_type == "value"){
		my_condition = my_xml_parser.get_node_value(node,"type", "condition");
		my_critical_value= patch::string2float(my_xml_parser.get_node_value(node, "type", "critical_value"));
	} 
	for(uint16_t i=0; i<node.getChildNode("notifications").nChildNode(); i++){
		hp_guard_notify_t tmp;
		tmp.str_1 = "";
		tmp.num_1 = -1;
		tmp.type = my_xml_parser.get_node_value(node.getChildNode("notifications"),"notify", "value", i);
		tmp.str_1= my_xml_parser.get_node_value(node.getChildNode("notifications"),"notify", "str_1", i);
		tmp.num_1= patch::string2int(my_xml_parser.get_node_value(node.getChildNode("notifications"),"notify", "num_1", i));
		notifiers.push_back(tmp);
	}
}

const hp_guard_notify_t *hp_guard::get_notifier(int pos)
{
	if(pos >= 0 && pos < (int)notifiers.size()){
		return &notifiers[pos];
	} else {
		return NULL;
	}
}

hp_guard::~hp_guard() {}
