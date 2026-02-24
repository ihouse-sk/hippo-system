#include <iostream>
#include "../include/hp2go.h"

using namespace std;


inline bool file_exists(const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main()
{
	XMLNode xml_main_node;
	XMLNode xml_node; 

	if(file_exists("../hp_go_conf.xml")){
		xml_main_node = XMLNode::openFileHelper("../hp_go_conf.xml","iHouse");
	} else if(file_exists("hp_go_conf.xml")){
		xml_main_node = XMLNode::openFileHelper("hp_go_conf.xml","iHouse");
	} else {
		perror("Nenasiel som konfiguracny subor hp_go_conf.xml\n");
		exit(1);
	}

	hp_go *hp_go_c;
	
	xml_node = xml_main_node.getChildNode("file");
	std::string file_name = xml_node.getAttribute("pwd");

	hp_go_c = new hp_go(file_name);
	//hp_go hp_go_c(file_name);

	if(hp_go_c->init() != -1){
		hp_go_c->run();
	}

	delete hp_go_c;

	/*
	 * 14, 13, 26
	 * 12, 3, 27
	 * 7, 9, 28
	 *
	 */

	return 0;
}

