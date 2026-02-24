#ifndef HP_GATES_H
#define HP_GATES_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_GATE_H
#include "hp_gate.h"
#endif

using namespace std;

class hp_gates {
	public:
		hp_gates(XMLNode);
		hp_gate *is_gate_pin(string pin);
		vector<hp_gate> get_gates() { return my_gates; }
		int get_gates_size() { return (int)my_gates.size(); }
		string get_lock_ident(int gate_pos);
		string get_lock(int gate_pos);
		string get_status_ident(int gate_pos);
		string get_status(int gate_pos);
		void init_gate_status(string gate_gui_ident, int value);
		string get_report();
		~hp_gates();
	private: 
		hp_xml_parser xml_parser;
		vector<hp_gate> my_gates;
};

#endif
