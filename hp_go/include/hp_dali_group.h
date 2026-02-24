#ifndef DALI_GROUP_H
#define DALI_GROUP_H

#include <iostream>
#include <vector>

#include <map>
#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class dali_group{

	public:
		//dali_item(string id, string item_type, string item_function, string desc, string desc2);
		dali_group(string dali_name, int , XMLNode item_node, map<int,int>);
		string get_id() { return my_id;}
		string get_desc() { return my_desc;}
		string get_desc2() { return my_desc2;}
		string get_dali_master_name() { return my_dali_master_name; }
		vector<string> get_group_items() { return my_group_items; }
		int get_dali_master_pos() { return my_dali_master_pos;}
		int get_actual_value() { return my_actual_value;}
		int set_actual_value(int value);
		~dali_group();
	//protected:
	private:
		string my_dali_master_name;
		int my_dali_master_pos;
		string my_id;
		string my_desc;
		string my_desc2;
		int my_actual_value;
		vector<string> my_item_errors;
		vector<string> my_group_items;
		map<int,int> my_map_values;

		string get_node_value(XMLNode xml_node, string , string parameter = "value");
};

#endif
