#include "../include/hp_dali_group.h"

dali_group::dali_group(string dali_name, int dali_pos, XMLNode item_node, map<int,int> map_values) :	
	my_dali_master_name(dali_name), my_dali_master_pos(dali_pos), my_map_values(map_values)
{
	my_id = get_node_value(item_node,"id","value");
	my_desc= get_node_value(item_node,"desc","value");
	my_desc2= get_node_value(item_node,"desc2","value");

	item_node = item_node.getChildNode("items");

	for(int i=0; i<item_node.nChildNode("item"); i++){
		my_group_items.push_back(get_node_value(item_node.getChildNode("item",i),"desc"));
	}

	if(my_item_errors.size() > 0){
		cout << "Errors in dali_group: " << endl;
		for(unsigned int i=0; i<my_item_errors.size(); i++){
			cout << my_item_errors[i] << endl;
		}
	}

	for(unsigned int i=0; i<my_group_items.size(); i++){
	//	cout << "item in group: " << my_group_items[i] << endl;
	}
	
}

string dali_group::get_node_value(XMLNode xml_node, string node_name , string parameter)
{
	if(xml_node.getChildNode(node_name.c_str()).isEmpty() || !xml_node.getChildNode(node_name.c_str()).isAttributeSet(parameter.c_str())){
		cout <<"xml node: " << node_name <<"  chyba" << endl;
		my_item_errors.push_back("Chyba pri citani xml node: "+node_name);
		return "";
	} else {
		return xml_node.getChildNode(node_name.c_str()).getAttribute(parameter.c_str());
	}
}

int dali_group::set_actual_value(int value)
{

	return 0;
}

dali_group::~dali_group()
{
}
