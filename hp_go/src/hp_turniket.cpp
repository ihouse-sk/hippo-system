#include "../include/hp_turniket.h"

hp_turniket::hp_turniket(XMLNode node)
{
	my_ident= xml_parser.get_node_value(node,"ident");
	my_type= xml_parser.get_node_value(node,"type");
	my_input_control= xml_parser.get_node_value(node,"control", "ident");
	my_input_control_value= xml_parser.get_node_value(node,"control", "on_value");
	my_input_control_send = xml_parser.get_node_value(node,"control", "send_value");
	for(int i=0; i<node.getChildNode("actors").nChildNode("actor"); i++){
		turniket_actor_t tmp;
		tmp.ident = xml_parser.get_node_value(node.getChildNode("actors"),"actor","value",i);
		tmp.hold= patch::string2int(xml_parser.get_node_value(node.getChildNode("actors"),"actor","hold",i));
		tmp.direction= xml_parser.get_node_value(node.getChildNode("actors"),"actor","direction",i);
		my_turniket_actors.push_back(tmp);
	}
	for(int i=0; i<node.getChildNode("readers").nChildNode("reader"); i++){
		turniket_reader_t tmp;
		tmp.reader_mac= xml_parser.get_node_value(node.getChildNode("readers"),"reader","value",i);
		tmp.reader_pos= xml_parser.get_node_value(node.getChildNode("readers"),"reader","position",i);
		my_turniket_readers.push_back(tmp);
	}
	//print_turniket();
}

bool hp_turniket::has_input(string ident)
{
	if(ident == my_input_control){
		return true;
	} else {
		return false;
	}
}

vector<turniket_actor_t> hp_turniket::check_control(string ident, string on_value)
{
	vector<turniket_actor_t> res;
	if(ident == my_input_control && on_value == my_input_control_value){
		for(unsigned int i=0; i<my_turniket_actors.size(); i++){
			my_turniket_actors[i].off_value = my_input_control_send;
			my_turniket_actors[i].off_delay = 200;
			res.push_back(my_turniket_actors[i]);
		}
	}

	return res;
}


bool hp_turniket::has_reader(string mac)
{
	for(unsigned int i=0; i<my_turniket_readers.size(); i++){
		if(my_turniket_readers[i].reader_mac == mac){
			return true;
		}
	}
	return false;
}
void hp_turniket::print_turniket()
{
	cout << "ident: " << my_ident << " type: " << my_type << " actors: " << endl;
	for(unsigned int i=0; i<my_turniket_actors.size(); i++){
		cout << "ident: " << my_turniket_actors[i].ident << ", hold: " << my_turniket_actors[i].hold << endl;
	}
	cout <<"readers: " << endl;
	for(unsigned int i=0; i<my_turniket_readers.size(); i++){
		cout << "mac: " << my_turniket_readers[i].reader_mac<< ", pos: " << my_turniket_readers[i].reader_pos << endl;
	}
}

hp_turniket::~hp_turniket() {}
