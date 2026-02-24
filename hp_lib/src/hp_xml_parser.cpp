#include "../include/hp_xml_parser.h"

hp_xml_parser::hp_xml_parser() {}

string hp_xml_parser::get_node_value(XMLNode xml_node, string node_name , string parameter, int pos)
{
	if(xml_node.getChildNode(node_name.c_str(),pos).isEmpty() || !xml_node.getChildNode(node_name.c_str(),pos).isAttributeSet(parameter.c_str())){
		if(node_name != "onoffpin"){
			if(parameter != "on_value"){
				my_xml_errors.push_back("Chyba pri citani xml node: "+node_name);
			}
		}
		return "";
	} else {
		return xml_node.getChildNode(node_name.c_str(),pos).getAttribute(parameter.c_str());
	}
}

bool hp_xml_parser::has_error()
{
	if(my_xml_errors.size() > 0){
		return true;
	} else {
		return false;
	}
}
string hp_xml_parser::fill_str(string str, int len)
{
	string res;
	res = str;
	//boost::trim_right(res);
	while((int)res.length() < len){
		res += " ";
	}
	return res;
}




hp_xml_parser::~hp_xml_parser(){}
