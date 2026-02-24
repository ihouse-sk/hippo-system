#ifndef DALI_PWM_LIGHT_H
#define DALI_PWM_LIGHT_H

#include <iostream>
#include "hp_dali_item.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifdef DALI_GROUP_S
#define DALI_GROUP_S
typedef struct {
	string id;
	string desc;
	string desc2;
} dali_group_t;
#endif

#ifdef DALI_SCENE_S
#define DALI_SCENE_S
typedef struct {
	string id;
	string desc;
	string desc2;
} dali_scene_t;
#endif

using namespace std;

class dali_pwm_light : public dali_item {
	public:
		dali_pwm_light(XMLNode item_node);
		string get_query_cmd() { return my_query_cmd;}
		string get_guery_group() { return my_query_group; }
		string get_query_dev_type() { return my_query_dev_type; }
		string create_mess(int send_value);
		string get_last_direction();
		int set_actual_value(int value);
		~dali_pwm_light();
	private:
		string my_query_cmd;
		string my_query_group;
		string my_query_dev_type;
		map<int,int> my_map_values;

};

#endif
