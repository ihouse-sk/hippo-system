#ifndef HP_METEO_GIOM_H
#include "../include/hp_meteo_giom.h"
#endif

#include <fstream>
#include <memory>


using namespace std;

inline bool file_exists(const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main()
{
	XMLNode xml_main_node;
	hp_xml_parser my_xml_parser;
	shared_ptr<hp_meteo_giom> my_giom = nullptr;

	if(file_exists("../hp_go_conf.xml")){
		xml_main_node = XMLNode::openFileHelper("../hp_go_conf.xml","iHouse");
	} else if(file_exists("hp_go_conf.xml")){
		xml_main_node = XMLNode::openFileHelper("hp_go_conf.xml","iHouse");
	} else {
		perror("Nenasiel som konfiguracny subor hp_go_conf.xml\n");
		exit(1);
	}
	std::string file_name = my_xml_parser.get_node_value(xml_main_node, "file", "pwd");
	cout << "conf file: " << file_name << endl;
	if(file_exists(file_name)){
		xml_main_node = XMLNode::openFileHelper(file_name.data(),"hippo");
		if(my_xml_parser.get_node_value(xml_main_node,"meteo_stanica") == "yes"){
			string socket = my_xml_parser.get_node_value(xml_main_node, "cls_socket");
			string ip= my_xml_parser.get_node_value(xml_main_node, "meteo_stanica", "ip");
			my_giom = make_shared<hp_meteo_giom>(socket,59,ip);//new hp_meteo_giom(this->my_cls_socket,59,my_xml_parser.get_node_value(xml_main_node,"meteo_stanica", "ip"));
		} else {
			cout <<"Skontroluj xml, ziadna aktivna meteo stanica v konfiguracnom subore." << endl;
		}
	} else {
		cout <<"Konfiguracny subor neexistuje: " << file_name <<  endl;
	}
	
	if(my_giom != nullptr){
		my_giom->do_work();
	} 


	return 0;
}

