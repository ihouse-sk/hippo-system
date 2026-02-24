#include "../include/hp_hbx.h"

hp_hbx::hp_hbx(XMLNode node, int hbx_pos, bool hbx_active) : my_pos(hbx_pos)
{
	my_location = node.getAttribute("location");
	if(my_location == "external"){
		my_ext_ip = xml_parser.get_node_value(node, "location", "ip");
		my_ext_port= patch::string2int(xml_parser.get_node_value(node, "location", "port"));
		cout <<"ip: " << my_ext_ip << " port: " << my_ext_port << endl;
	}
	my_mac = xml_parser.get_node_value(node, "mac");
	my_name= xml_parser.get_node_value(node, "name");
	my_type= xml_parser.get_node_value(node, "hbx_type");

	my_status = !xml_parser.has_error();
	my_mess_counter = 0;
	my_hbx_status = -1;
	my_restart_type = -1;
	my_internal_timer = time(NULL) - RESTART_TIME_TRESHOLD*1.1;
	my_mess_timer = 0;
	my_u_send = false;
	my_u_timer = time(NULL);
	my_restart_timer = time(NULL) - 2*PROTECT_INTERVAL;
	my_hbx_active = hbx_active;
	my_notification_send = false;
	//print_hbx();
}

bool hp_hbx::in_protect_interval()
{
	if(my_restart_timer + PROTECT_INTERVAL < time(NULL)){
		return true;
	} else {
		return false;
	}
}

bool hp_hbx::restart_enabled()
{
	if(my_restart_timer + PROTECT_INTERVAL < time(NULL)){
		my_restart_timer = time(NULL);
		return true;
	} else {
		return false;
	}
}
string hp_hbx::check_internal_timer(int check_time)
{
	string res = "";
	if(my_internal_timer + check_time < time(NULL)){
		if(!my_notification_send){
			res = "HBX: " + this->my_name +" nekomunikuje viac ako: " + patch::to_string(check_time) + " sekúnd.";
			my_notification_send = true;
		}
	} else {
		if(my_notification_send){
			res = "HBX:" + this->my_name +" je znova aktívny";
			my_notification_send = false;
		}
	}
	return res;
}

string hp_hbx::check_hbx_timer()
{
	if(my_u_timer +10 < time(NULL)){
		my_u_timer = 0;
		my_u_send = false;
	}
	if(my_mess_timer + MESS_TIME_THRESHOLD < time(NULL) && my_mess_counter > 0){
		this->decrease_mess_counter();
		return "+++++Niekde sa stala chyba a musel som decreasnut hbx mess counter pre hbx: " + this->my_mac;
	}
	return "";
}

void hp_hbx::set_restart_type(int type) 
{ 
	my_restart_type = type;
}
int  hp_hbx::get_restart_type()
{
	//cout <<"My internal timer: " << my_internal_timer << " and sec from timer : " << time(NULL)-my_internal_timer << " a restart type: " << my_restart_type<< endl;
	if(my_internal_timer + RESTART_TIME_TRESHOLD> time(NULL)){
		return my_restart_type;
	} else {
		my_restart_type = -1;
	}
	return -1;
}

void hp_hbx::print_hbx()
{
	cout <<"hbx_pos: " << my_pos << ", name: " << my_name << ", mac: " << my_mac << ", type: " << my_type << endl;
}

hp_hbx::~hp_hbx() {}
