#include "../include/hp_gates.h"

hp_gates::hp_gates(XMLNode node)
{
	for(int i=0; i<node.nChildNode("gate"); i++){
	//	string gate_type = xml_parser.get_node_value(node,"gate", "type",i);
		my_gates.push_back(hp_gate(node.getChildNode("gate",i)));
	}
}
string hp_gates::get_lock(int gate_pos)
{
	if(gate_pos >= 0 && gate_pos < (int)my_gates.size()){
		//cout <<"vraciam: " << my_gates[gate_pos].get_ident() << " na pozicii: " << gate_pos<< endl;
		return my_gates[gate_pos].get_status();
	}
	return "";
}
string hp_gates::get_status(int gate_pos)
{
	if(gate_pos >= 0 && gate_pos < (int)my_gates.size()){
		//cout <<"vraciam: " << my_gates[gate_pos].get_ident() << " na pozicii: " << gate_pos<< endl;
		return my_gates[gate_pos].get_status();
	}
	return "";
}

string hp_gates::get_lock_ident(int gate_pos)
{
	if(gate_pos >= 0 && gate_pos < (int)my_gates.size()){
		//cout <<"vraciam: " << my_gates[gate_pos].get_ident() << " na pozicii: " << gate_pos<< endl;
		return my_gates[gate_pos].get_gui_lock_ident();
	}
	return "";
}
string hp_gates::get_status_ident(int gate_pos)
{
	if(gate_pos >= 0 && gate_pos < (int)my_gates.size()){
		//cout <<"vraciam: " << my_gates[gate_pos].get_ident() << " na pozicii: " << gate_pos<< endl;
		return my_gates[gate_pos].get_status_gui_ident();
	}
	return "";
}

hp_gate *hp_gates::is_gate_pin(string pin)
{
	hp_gate *res = NULL;
	for(unsigned int i=0; i<my_gates.size(); i++){
		//cout << my_gates[i].get_gui_lock_ident() <<"," <<  my_gates[i].get_ident_opened() << ", " <<  my_gates[i].get_ident_closed() <<endl;
		if(my_gates[i].get_gui_lock_ident() == pin || my_gates[i].get_ident_opened() == pin || my_gates[i].get_ident_closed() == pin || my_gates[i].get_move_ident("1") == pin || my_gates[i].get_move_ident("2") == pin){
			res = &my_gates[i];
			break;
		}
	}

	return res;
}
string hp_gates::get_report()
{
	string res ="";

	for(unsigned i=0; i<my_gates.size(); i++){
		res += my_gates[i].get_report();
	}
	res += "\n";
	return res;
}

void hp_gates::init_gate_status(string gate_gui_ident, int value)
{
	for(unsigned int i=0; i<my_gates.size(); i++){
		if(my_gates[i].get_status_gui_ident() == gate_gui_ident){
			my_gates[i].init_state_setup(value);
			break;
		}
	}
}

hp_gates::~hp_gates() {}
