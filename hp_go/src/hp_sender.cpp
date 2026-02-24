#include "../include/hp_sender.h"

hp_sender::hp_sender(string xbee_socket) : my_xbee_socket(xbee_socket)
{
	my_client_socket = new hp_client_socket(AF_UNIX,SOCK_STREAM, 0, my_xbee_socket);
	for(int i=0; i<MAX_XBEE_MESS_ID; i++){
		my_xbee_mess_id.push_back(0);
	}
	for(int i=33; i<125; i++){
		if(i != 'A' && i != 'B'&& i != 'C'&& i != 'D'&& i != 'E'&& i != 'U'&& i != 'W'&& i != 'L' && i != 'H' && i != 'T'){
			hp_id_data_t tmp;
			tmp.id = i;
			tmp.free = true;
			my_uc_mess_id.push_back(tmp);
		}
	}
	my_start_xbee_id = 0;
	last_time = time(NULL);
	my_db_data = NULL;
}

hp_sender::hp_sender(string xbee_socket, hp_db_data_t *data) : my_xbee_socket(xbee_socket)
{
	my_client_socket = new hp_client_socket(AF_UNIX,SOCK_STREAM, 0, my_xbee_socket);
	for(int i=0; i<MAX_XBEE_MESS_ID; i++){
		my_xbee_mess_id.push_back(0);
	}
	for(int i=33; i<125; i++){
		if(i != 'A' && i != 'B'&& i != 'C'&& i != 'D'&& i != 'E'&& i != 'U'&& i != 'W'&& i != 'L' && i != 'H' && i != 'T'){
			hp_id_data_t tmp;
			tmp.id = i;
			tmp.free = true;
			my_uc_mess_id.push_back(tmp);
		}
	}
	my_start_xbee_id = 0;
	last_time = time(NULL);
	my_db_data = data;
}

void hp_sender::free_xbee_id(int id)
{
	//cout<<"set free xbee_id: " << id << endl;
	if(id > 0 && id < MAX_XBEE_MESS_ID){
		my_xbee_mess_id[id-1] = 0;
	}
}

void hp_sender::print_non_free_id()
{
	for(vector<hp_id_data_t>::iterator it = my_uc_mess_id.begin(); it != my_uc_mess_id.end(); ++it){
		if(it->free == false){
			cout << "Non free uc id: " << it->id << endl;
		}
	}
	for(vector<int>::iterator it = my_xbee_mess_id.begin(); it != my_xbee_mess_id.end(); ++it){
		if(*it != 0){
			cout << "Non free xbee id: " << *it << endl;
		}
	}
}
vector<int> hp_sender::get_non_free()
{
	vector<int> res;
	for(vector<hp_id_data_t>::iterator it = my_uc_mess_id.begin(); it != my_uc_mess_id.end(); ++it){
		if(it->free == false){
			res.push_back(it->id);
		}
	}
	res.push_back(-1);
	for(vector<int>::iterator it = my_xbee_mess_id.begin(); it != my_xbee_mess_id.end(); ++it){
		if(*it != 0){
			res.push_back(*it);
		}
	}
	return res;
}

void hp_sender::free_uc_id(string id)
{
	for(vector<hp_id_data_t>::iterator it = my_uc_mess_id.begin(); it != my_uc_mess_id.end(); ++it){
		//cout << it->id << " == " << id[0] << endl;
		if(it->id == id[0]){
		//	cout<<"set free uc_id: " << id[0] << endl;
			it->free = true;
			break;
		}
	}
}
bool hp_sender::is_free_xbee_id(int id)
{
	//cout <<"Checking id: " << id << " value: " << my_xbee_mess_id[id-1] << endl;
	if(id > 0 && id < MAX_XBEE_MESS_ID){
		return my_xbee_mess_id[id-1] == 0?true:false;
	} else {
		return true;
	}
}

void hp_sender::delete_timer_mess(string mess, string xbee_id)
{
	for(vector<hp_mess_data>::iterator it = my_messages.begin(); it != my_messages.end(); ++it){
		//cout << it->get_mess().substr(1,2) << " == " <<  mess.substr(0,2) << " && "<< it->get_xbee_id() << " == " <<  xbee_id << endl;
		if(it->get_mess().substr(1,2) == mess.substr(0,2) && it->get_xbee_id() == xbee_id && it->get_delay() != 0){
			if(it == my_messages.end()-1){
				my_messages.erase(it);
				break;
			} else {
				my_messages.erase(it);
				it--;
			}
		}
	}
}

bool hp_sender::add_message(hp_mess2send_t data, int resend_count, int priority, int delay)
{
	//cout << "Sendera: xbee_id: " << data.xbee_id << " mess: " << data.mess << " type; " << data.uc_mess_type <<" delay: " << delay << " resend count: " << resend_count <<endl;
	bool res = true;
	if(my_messages.size() == 0){
		my_messages.push_back(hp_mess_data(data, resend_count, priority, delay));
	} else {
		bool add_new = true;
		for(vector<hp_mess_data>::iterator it = my_messages.begin(); it != my_messages.end(); ++it){
			//cout << it->get_xbee_id() <<" == " <<  data.xbee_id << " && " <<  it->get_delay() << " == " << 0 << " && " <<  it->get_mess_type()<< " == " <<  data.uc_mess_type << endl;
			if(it->get_xbee_id() == data.xbee_id && it->get_delay() == delay && it->get_mess_type() == data.uc_mess_type && it->get_free_slot() > 0){
				//cout << "Pridavam spravu " << data[2] << ", vysledok: " << it->add_message(data[2]) << endl;
				if(it->check_message(data.mess)){
					add_new =false;
					break;
				}
				if(it->add_message(data.mess)){
					add_new = false;
				}
				break;
			}
			if(it->get_xbee_id() == data.xbee_id &&  it->get_mess().find(data.mess) != std::string::npos && data.uc_mess_type != "9" && data.mess != ""){
				it->set_delay(delay);
				add_new = false;
			}
		}
		if(add_new){
			my_messages.push_back(hp_mess_data(data, resend_count, priority, delay));
		}
	}
	/*
	cout << "Messages: " <<endl;
	for(vector<hp_mess_data>::iterator it = my_messages.begin(); it != my_messages.end(); ++it){
		cout << "hbx: "<< it->get_xbee_id() << ", mess: " <<  it->get_mess() << endl;
	}
	*/

	return res;
}

bool sort_function(hp_mess_data i, hp_mess_data j)
{
	return i.get_priority() > j.get_priority();
}

void hp_sender::send_messages(vector<hp_hbx *> &hbxs, vector<hp_send_messages_t> &send_messages_vect)
{
	std::sort(my_messages.begin(), my_messages.end(), sort_function);
	for(vector<hp_mess_data>::iterator it = my_messages.begin(); it != my_messages.end(); ++it){
		if(last_time +10 < time(NULL)){
			if(it->get_delay() < 6000){
				push_db_query("\t\t????HBX: "+ it->get_xbee_id() + " uc_mess: " + it->get_mess() + ", ident: " + it->get_ident()+ ",delay: " + patch::to_string(it->get_delay()) +"0 [ms], pozicia in delayed mess: " + patch::to_string(std::distance(my_messages.begin(), it)+1) + " z " + patch::to_string(my_messages.size()), DB_LOG_COM);
			}
		}
		if(it->get_delay() > 6000 && it->get_delay()%6000 == 0){
			push_db_query("\t\t????HBX: "+ it->get_xbee_id() + " uc_mess: " + it->get_mess() + ", ident: " + it->get_ident()+",delay: " + patch::to_string(it->get_delay()) +"0 [ms], pozicia in delayed mess: " + patch::to_string(std::distance(my_messages.begin(), it)), DB_LOG_COM);
		}
		if(it->get_delay() == 0){
			int hbx_pos = atoi(it->get_xbee_id().c_str())-1;
			if(hbx_pos < 0 || (unsigned int)hbx_pos > hbxs.size()){
			//	cout << "hpx_pos: " << hbx_pos << " hbx_id: " << it->get_xbee_id() << endl;
				if(it == my_messages.end()-1){
					my_messages.erase(it);
					break;
				} else {
					my_messages.erase(it);
					it--;
				}
				continue;
			}
			if(hbxs[hbx_pos]->in_protect_interval() && hbxs[hbx_pos]->was_u_sent()){
				if(it->postpone_avaible()){
					it->set_delay(100);
				} else {
					if(it == my_messages.end()-1){
						my_messages.erase(it);
						break;
					} else {
						my_messages.erase(it);
						it--;
					}
					continue;
				}
			}
			if(!hbxs[hbx_pos]->is_active()){
				if(it == my_messages.end()-1){
					my_messages.erase(it);
					break;
				} else {
					my_messages.erase(it);
					it--;
				}
				continue;
			}
			
			if(hbxs[hbx_pos]->get_mess_counter() < HBX_BUFFER){
				int xbee_mess_id = get_free_xbee_mess_id();
				string uc_mess_id = "";
				if(it->get_mess_type() != "0"){
					uc_mess_id = get_free_uc_mess_id(); 
				}
				if(xbee_mess_id != -1 && (uc_mess_id != "" || it->get_mess_type() == "0")){
					string mess;
					if(it->get_mess_type() == "0"){
						mess = it->get_xbee_id()+"_"+patch::to_string(xbee_mess_id)+ "_" + it->get_mess();
					} else {
						mess = it->get_xbee_id()+"_"+patch::to_string(xbee_mess_id)+ "_" +uc_mess_id+ it->get_mess();
					}
					if(hbxs[hbx_pos]->get_location() == "external"){
						mess = "EXT_"+hbxs[hbx_pos]->get_mac()+"_"+mess;
						//hp_threaded_socket th(hbxs[hbx_pos]->get_ext_port(), hbxs[hbx_pos]->get_ext_ip(), mess);
						thread th2(hp_threaded_socket(hbxs[hbx_pos]->get_ext_port(), hbxs[hbx_pos]->get_ext_ip(), mess));
						th2.detach();
					} else {
						if(my_client_socket->send_socket(mess) == -1){
							push_db_query("\t\t++++Nemozem poslat spravu na socket: "+my_xbee_socket+ " socket buffer size: " + patch::to_string(my_messages.size()), DB_LOG_COM);
							free_uc_id(uc_mess_id);
							continue;
						}
					}
					push_db_query("\t\t\t\t\t\t\t\t\tPosielam: "+mess, DB_LOG_COM);
					hbxs[hbx_pos]->increase_mess_counter();

					hp_send_messages_t tmp_mess;
					tmp_mess.time_form_send = 0;
					tmp_mess.txstat = -1;
					tmp_mess.sent_count = it->get_resend_count();
					if(it->get_mess_type() == "0"){
						tmp_mess.mess = it->get_mess();
					} else {
						tmp_mess.mess = uc_mess_id+ it->get_mess();
					}
					tmp_mess.mess_type = it->get_mess_type();
	//				cout << "Pushing uc_id: " << uc_mess_id << " cela sprava: " << tmp_mess.mess << endl;
					tmp_mess.xbee_id = it->get_xbee_id();
					tmp_mess.frame_id = patch::to_string(xbee_mess_id);
					send_messages_vect.push_back(tmp_mess);

					if(it == my_messages.end()-1){
						my_messages.erase(it);//std::distance(my_messages.begin(),it));
						break;
					} else {
						my_messages.erase(it);//std::distance(my_messages.begin(),it));
						it--;
					}
				} else {
					if(last_time +10 < time(NULL)){
						cout << "xbee_mess_id: " << xbee_mess_id << " a uc_mess_id: " << uc_mess_id << endl;
					}
				}
			} else {
				if(last_time +10 < time(NULL)){
					//cout << "Neposielam spravu lebo buffer: " << hbxs[hbx_pos]->get_mess_counter() << " < " << HBX_BUFFER << " debugdata: " << it->get_debug_data() <<  endl;
					push_db_query("\t\t++++Neposielam spravu lebo buffer: " + patch::to_string(hbxs[hbx_pos]->get_mess_counter()) + " < " + patch::to_string(HBX_BUFFER) + " debugdata: " + it->get_debug_data() + " pre hbx: " + patch::to_string(hbxs[hbx_pos]->get_pos()) + " mac: " + hbxs[hbx_pos]->get_mac() , DB_LOG_COM);
					for(int i=0; i<MAX_XBEE_MESS_ID; i++){
						if(my_xbee_mess_id[i] != 0){
							cout << i << ": " << my_xbee_mess_id[i] << endl;
						}
					}
					/*
					for(unsigned int i=0; i<send_messages.size(); i++){
						cout <<i << " id xbee: " << send_messages[i].xbee_id << " mess: " << send_messages[i].mess << " frame: " << send_messages[i].frame_id << " adas: " << send_messages[i].txstat << endl;
					}
					*/
				}	
			}
		} else {
			it->cout_delay();
			//cout <<"delay: " << it->get_delay() << endl;
		}
	}
	if(last_time + 10 < time(NULL)){
		last_time = time(NULL);
	}
}

string hp_sender::get_free_uc_mess_id()
{
	unsigned int i;
	string res;
	for(i=0; i<my_uc_mess_id.size(); i++){
		if(my_uc_mess_id[i].free){
			my_uc_mess_id[i].free = false;
			res.push_back((char)my_uc_mess_id[i].id);
			break;
		}
	}
	return res;
}

int hp_sender::get_free_xbee_mess_id()
{
	int res = -1;
	my_start_xbee_id++;
	if(my_start_xbee_id > MAX_XBEE_MESS_ID/2){
		my_start_xbee_id = 0;
	}
	for(int i=my_start_xbee_id; i<MAX_XBEE_MESS_ID; i++){
		if(my_xbee_mess_id[i] == 0){
			my_xbee_mess_id[i] = 1;
			return i+1;
		}
	}
	return res;
}

void hp_sender::push_db_query(string query, int type, int log_level)
{
	if(my_db_data != NULL){
		if(DISPLAY_MESSAGES && type != DB_ALL_STATUSES){
		//	cout << query << endl;
		}
		hp_db_queries_t tmp;
		tmp.query = query;
		tmp.type = type;
		tmp.log_level = log_level;
	
		my_db_data->mtx.lock();
		my_db_data->queries.push_back(tmp);
		my_db_data->mtx.unlock();
	}
}


hp_sender::~hp_sender() 
{
	delete this->my_client_socket;
}
