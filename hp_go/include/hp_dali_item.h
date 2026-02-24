#ifndef DALI_ITEM_H
#define DALI_ITEM_H

#include <iostream>
#include <vector>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class dali_item {

	public:
		//dali_item(string id, string item_type, string item_function, string desc, string desc2);
		dali_item(XMLNode item_node);
		string get_id() { return my_id;}
		string get_item_type() { return my_item_type;}
		string get_item_function() { return my_item_function;}
		string get_desc() { return my_desc;}
		string get_desc2() { return my_desc2;}
		string get_dali_master_name() { return my_dali_master_name; }
		int get_dali_master_pos() { return my_dali_master_pos;}
		int get_actual_value() { return my_actual_value;}
		void set_master_pos(int pos) { my_dali_master_pos = pos; }
		virtual string get_last_direction() {return "";};
		void set_last_direction(string dir) {my_last_direction = dir; }

		virtual int set_actual_value(int value) { my_actual_value = value; return 0;}
		virtual string create_mess(int send_value);
		virtual string get_query_cmd() { return "";};
		virtual	string get_guery_group() { return ""; }
		virtual string get_query_dev_type() { return ""; }
		virtual ~dali_item();
	protected:
	//private:
		string my_dali_master_name;
		int my_dali_master_pos;
		string my_id;
		string my_item_type;
		string my_item_function;
		string my_desc;
		string my_desc2;
		int my_actual_value;
		string my_last_direction;

	protected:
		vector<string> my_item_errors;

		string get_node_value(XMLNode xml_node, string , string parameter = "value");
};

#endif
