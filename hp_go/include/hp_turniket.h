#ifndef HP_TURNIKET_H
#define HP_TURNIKET_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

typedef struct {
	string ident;
	int hold;
	string direction;
	string off_value;
	int off_delay;
} turniket_actor_t;

typedef struct {
	string reader_mac;
	string reader_pos;
} turniket_reader_t;

using namespace std;

class hp_turniket {
	public:
		hp_turniket(XMLNode);
		bool has_reader(string mac);
		vector<turniket_actor_t> check_control(string ident, string on_value);
		bool has_input(string ident);
		vector<turniket_actor_t> get_actors() { return my_turniket_actors; }
		~hp_turniket();
	private: 
		hp_xml_parser xml_parser;
		vector<turniket_actor_t> my_turniket_actors;
		vector<turniket_reader_t> my_turniket_readers;
		string my_ident;
		string my_type;
		string my_reader_mac;
		string my_input_control;
		string my_input_control_value;
		string my_input_control_send;


		void print_turniket();
};

#endif
