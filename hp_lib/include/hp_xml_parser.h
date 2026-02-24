#ifndef HP_XML_PARSER_H
#define HP_XML_PARSER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include "xmlParser.h"
#include <iomanip>
#include <stdio.h>

#ifndef HP_DEFAULTS_H
#include "hp_defaults.hpp"
#endif

using namespace std;

class hp_xml_parser {
	public:
		hp_xml_parser();
		string get_node_value(XMLNode xml_node, string , string parameter = "value", int pos = 0);
		string fill_str(string str, int len=40);
		bool has_error();
		~hp_xml_parser();
	private: 
		vector<string> my_xml_errors;
};

#endif
