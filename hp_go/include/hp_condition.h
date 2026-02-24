#ifndef HP_CONDITIONS_H
#define HP_CONDITIONS_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_VIRTUAL_PIN_H
#include "hp_virtual_pin.h"
#endif

#define CONDITION_ATLEASTONE	"atLeastOne"
#define CONDITION_ALL		"all"

typedef struct {
	string ident;
	int pos;
	string qvalue;
	string actual_value;
} cond_items_t;

typedef struct {
	string to_value;
	int pin_pos;
} cond_struc_t;
	

using namespace std;

class hp_condition 
{
	public:
		hp_condition(XMLNode node) {
			my_ident= xml_parser.get_node_value(node, "actor_ident");
			my_active_condition_value = xml_parser.get_node_value(node, "when");
			my_condition_type = xml_parser.get_node_value(node, "when", "condition");
			for(int i=0; i<node.getChildNode("when").nChildNode("item"); i++){
				cond_items_t tmp;
				tmp.ident = xml_parser.get_node_value(node.getChildNode("when"), "item","ident",i);
				tmp.pos = -1;
				tmp.actual_value = "0";
				tmp.qvalue = xml_parser.get_node_value(node.getChildNode("when"), "item","value",i);
				this->my_conds_items.push_back(tmp);
			}
			my_ident_pos = -1;
		}
		cond_struc_t get_cond_data() {
			cond_struc_t res;
			res.pin_pos = this->my_ident_pos;
			res.to_value = my_active_condition_value;
			bool valid_cond;
			if(my_condition_type == CONDITION_ATLEASTONE){
				valid_cond = false;
			}
			if(my_condition_type == CONDITION_ALL){
				valid_cond = true;
			}

			for(uint16_t i=0; i<my_conds_items.size(); i++){
				if(my_condition_type == CONDITION_ATLEASTONE){
					if(my_conds_items[i].actual_value == my_conds_items[i].qvalue){
						valid_cond = true;
					}
				}
				if(my_condition_type == CONDITION_ALL){
					if(my_conds_items[i].actual_value != my_conds_items[i].qvalue){
						valid_cond = false;
						break;
					}
				}
			}
			if(!valid_cond){
				if(my_active_condition_value == "0"){
					res.to_value = "1";
				} else {
					res.to_value = "0";
				}
			}

			return res;
		}
		bool has_effect(int pos, string value){
			for(uint16_t i=0; i<my_conds_items.size(); i++){
				if(pos == my_conds_items[i].pos){
					my_conds_items[i].actual_value = value;
					return true;
				}
			}
			return false;
		}
		void setup_item(string name, int pos, string value) {
			if(name == this->my_ident){
				my_ident_pos = pos;
			}
			for(uint16_t i=0; i<my_conds_items.size(); i++){
				if(name == my_conds_items[i].ident){
					my_conds_items[i].pos = pos;
					my_conds_items[i].actual_value= value;
				//	cout << "pos: " << pos << ", value: " << value <<" for: " << name << endl;
					break;
				}
			}
		}
		~hp_condition(){};
	private:
		hp_xml_parser xml_parser;
		string my_ident;
		int my_ident_pos;
		string my_active_condition_value;
		string my_condition_type;
		vector<cond_items_t> my_conds_items;
};

class hp_conditions 
{
	public:
		hp_conditions(XMLNode node, hp_db_data_t *data = NULL);
		void setup_items_position(const vector<hp_virtual_pin *> pins);
		vector<cond_struc_t> check_condition(int pos, string value);
		~hp_conditions();
	private: 
		hp_xml_parser xml_parser;
		hp_db_data_t *my_db_data;

		vector<hp_condition> my_conditions;

		void push_db_query(string query, int type=DB_STATUSES_QUERY, int log_level=0);
};

#endif
