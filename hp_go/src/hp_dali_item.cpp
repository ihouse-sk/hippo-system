#include "../include/hp_dali_item.h"

dali_item::dali_item(XMLNode item_node)
{
	my_id = get_node_value(item_node,"id","value");
	my_item_type= get_node_value(item_node,"item_type","value");
	my_item_function= get_node_value(item_node,"item_type","function");
	my_desc= get_node_value(item_node,"desc","value");
	my_desc2= get_node_value(item_node,"desc2","value");

	if(my_item_errors.size() > 0){
		for(unsigned int i=0; i<my_item_errors.size(); i++){
			cout << my_item_errors[i] << endl;
		}
	}

	my_actual_value = 0;
	my_dali_master_pos = 0;
}

string dali_item::get_node_value(XMLNode xml_node, string node_name , string parameter)
{
	if(xml_node.getChildNode(node_name.c_str()).isEmpty() || !xml_node.getChildNode(node_name.c_str()).isAttributeSet(parameter.c_str())){
		cout <<"xml node: " << node_name <<"  chyba" << endl;
		my_item_errors.push_back("Chyba pri citani xml node: "+node_name);
		return "";
	} else {
		return xml_node.getChildNode(node_name.c_str()).getAttribute(parameter.c_str());
	}
}

string dali_item::create_mess(int send_value)
{
	string res = "";
	if(send_value){}
	return res;
}

dali_item::~dali_item()
{
}
