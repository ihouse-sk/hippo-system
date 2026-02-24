#include "../include/hp_dali.h"

hp_dali::hp_dali(XMLNode node,hp_db_data_t *db_data, hp_dali_sht *dd)
{
	my_socket =-1;
	testing = true;

	my_map_values.insert(std::pair<int,int>(0,0));
	my_map_values.insert(std::pair<int,int>(1,170));
	my_map_values.insert(std::pair<int,int>(2,195));
	my_map_values.insert(std::pair<int,int>(3,210));
	my_map_values.insert(std::pair<int,int>(4,220));
	my_map_values.insert(std::pair<int,int>(5,229));
	my_map_values.insert(std::pair<int,int>(6,235));
	my_map_values.insert(std::pair<int,int>(7,241));
	my_map_values.insert(std::pair<int,int>(8,246));
	my_map_values.insert(std::pair<int,int>(9,250));

	this->read_config(node);
	this->my_db_data = db_data;
	my_delay_id=0;
	this->my_shared_data = dd;
}

int hp_dali::read_config(XMLNode node)
{
	for(int i=0; i<node.nChildNode("dali_device"); i++){
		dali_devices_t tmp;
		tmp.name= my_xml_parser.get_node_value(node.getChildNode("dali_device",i), "ident");
		tmp.ip= my_xml_parser.get_node_value(node.getChildNode("dali_device",i), "ip");
		tmp.port= patch::string2int(my_xml_parser.get_node_value(node.getChildNode("dali_device",i), "port"));
		my_dali_devices.push_back(tmp);
	}
	for(int i=0; i<node.nChildNode("item"); i++){
		string item_type = my_xml_parser.get_node_value(node.getChildNode("item",i), "item_type");
		if(item_type  == "pwm"){
			my_dali_items.push_back(new dali_pwm_light(node.getChildNode("item",i))); 
		} else {
			my_dali_items.push_back(new dali_item(node.getChildNode("item",i))); 
		}
	}
	
	for(int i=0; i<(int)my_dali_devices.size(); i++){
		dali_data_t *tmp  = new dali_data_t();
		my_dalinet_devices_data.push_back(tmp);
		dalinet_device *dali = new dalinet_device(my_dali_devices[i].ip,my_dali_devices[i].port, tmp, my_dali_devices[i].name);
		my_dalinet_devices.push_back(dali);
	}

	/*
	for(int j=0; j<xml_node.nChildNode("group"); j++){
		my_dali_groups.push_back(new dali_group(tmp.name, (int)my_dali_devices.size(), xml_node.getChildNode("group",j),my_map_values));
	}
	*/
	for(int i=0; i<(int) my_dalinet_devices.size(); i++){
		thread *tmp = new thread(*my_dalinet_devices[i]);
		my_dalinet_threads.push_back(tmp);
	}
	for(uint16_t i=0; i<my_dali_items.size(); i++){
		for(int j=0; j<(int) my_dalinet_devices.size(); j++){
			//cout << my_dali_items[i]->get_dali_master_name() << " == " << my_dalinet_devices[j]->get_name() << endl;
			if(my_dali_items[i]->get_dali_master_name() == my_dalinet_devices[j]->get_name()){
				my_dali_items[i]->set_master_pos(j);
			}
		}
	}

	return 0;
}
void hp_dali::read_startup_values()
{
	unsigned int i;
	for(i=0; i<my_dali_items.size(); i++){
		int pos = my_dali_items[i]->get_dali_master_pos();
		string mess = my_dali_items[i]->get_id()+"_"+my_dali_items[i]->get_query_cmd();
		if(my_dali_items[i]->get_item_type() == "pwm" && my_dali_items[i]->get_item_function() != "group"){
			if(my_dalinet_devices_data[pos]->dali_commands.size() == 0){
				push_cmd(mess, pos);
			} else {
				this->add_timer_item(0,pos,mess, DELAY_MESS,my_delay_id);
			}
		}
	}
	//print_buffers();
}

void hp_dali::button_off(int pin_pos)
{
	unsigned int i;
	for(i=0; i<my_dali_timers.size(); i++){
		if(my_dali_timers[i].button_pin_pos == pin_pos){
			my_dali_timers.erase(my_dali_timers.begin()+i);
			i--;
		}
	}
}

void hp_dali::push_synchro_mess(vector<int> values, int pin_pos, string ident)
{
	int item_pos = find_dali_item(ident,"desc");
	if(item_pos != -1){
		int dalinet_pos = my_dali_items[item_pos]->get_dali_master_pos();
		for(uint16_t i=0; i<values.size(); i++){
			this->add_timer_item((i)*50,dalinet_pos,create_mess(patch::to_string(values[i]),item_pos), DALI_DELAY_TIME,my_delay_id,pin_pos);
		}
	}
}

vector<int> hp_dali::process_light_rule(hp_rule_light_t rule, int pin_pos)
{
	//cout << rule.ident << ", " << rule.to_value << ", " << rule.timer_time << ", " << rule.timer_type << endl;
	vector<int> res;
	int item_pos = find_dali_item(rule.ident,"desc");
	if(item_pos != -1){
		///// 0,1,9, fluently, change
		int dalinet_pos = my_dali_items[item_pos]->get_dali_master_pos();
		string fc = my_dali_items[item_pos]->get_item_type();
		string value = "0";
	//	cout << "fc: " << fc << ", current value: " << my_dali_items[item_pos]->get_actual_value() << endl;
		if(rule.to_value == "change"){
			if(my_dali_items[item_pos]->get_actual_value() != 0){
				value = "0";
			} else {
				if(fc == "pwm"){
					value = rule.my_pwm_on;
				} else {
					value = rule.to_value;
				}
			}
		} else if(rule.to_value == "fluently"){
			//cout << "start pin: " << pin_pos << ", dir: " << my_dali_items[item_pos]->get_last_direction()<< " val: " << my_dali_items[item_pos]->get_actual_value() << endl;
			string dir = my_dali_items[item_pos]->get_last_direction();
			int act_value = my_dali_items[item_pos]->get_actual_value();
			if(dir == "down"){
				int ms_counter=0;
				for(int i=act_value+2; i<10; i++){
					this->add_timer_item((++ms_counter)*50,dalinet_pos,create_mess(patch::to_string(i),item_pos), DALI_DELAY_TIME,my_delay_id,pin_pos);
					res.push_back(i);
				}
				my_dali_items[item_pos]->set_last_direction("up");
				if(act_value < 9){
					value = patch::to_string(act_value+1);
				}
			} else {
				int ms_counter=0;
				for(int i=act_value-2; i>=0; i--){
					this->add_timer_item((++ms_counter)*50,dalinet_pos,create_mess(patch::to_string(i),item_pos), DALI_DELAY_TIME,my_delay_id,pin_pos);
					res.push_back(i);
				}
				my_dali_items[item_pos]->set_last_direction("down");
				if(act_value > 0){
					value = patch::to_string(act_value-1);
				}
			}
		} else {
			value = rule.to_value;
		}

		string send_mess = create_mess(value,item_pos);
		if(send_mess != ""){
			push_cmd(send_mess,dalinet_pos);
			if(value != "0"){
				if(rule.timer_type == "timer" && rule.timer_time != "0"){
					this->add_timer_item(patch::string2int(rule.timer_time) * 100,dalinet_pos,create_mess("0",item_pos), DALI_DELAY_TIME,my_delay_id);
				}
			}
			res.insert(res.begin(), patch::string2int(value));
			//return patch::string2int(value);
		} else {
			cout << "Error nespravna sprava... :D " << endl;
		}
	} else {
		/*
		item_pos = find_dali_group(mess.substr(0,mess.find("_")),"desc");
		if(item_pos != -1){
			int dalinet_pos = my_dali_groups[item_pos]->get_dali_master_pos();
			if(mess.substr(mess.find("_")+1).find("0") != std::string::npos){
				push_cmd(my_dali_groups[item_pos]->get_id()+"_off",dalinet_pos);
			} else {
				push_cmd(my_dali_groups[item_pos]->get_id()+"_on",dalinet_pos);
			}
		} else {
			return -1;
		}
		*/
	}

	return res;
}

void hp_dali::print_buffers()
{
	unsigned int i;
	for(i=0; i<my_dalinet_devices_data.size(); i++){
		for(unsigned int j=0; i<my_dalinet_devices_data[i]->dali_commands.size(); j++){
		}
	}
	cout <<endl << "!!!!!!!!  my_dali_commands  !!!!!!!!!!!!!!!!!!!!!!!!!!!" <<endl;
	for(i=0; i<my_dali_commands.size(); i++){
		cout << "Mess: " << my_dali_commands[i].mess << " id: " << my_dali_commands[i].id << endl;
	}
	cout <<endl << "!!!!!!!!  my_dali_timers    !!!!!!!!!!!!!!!!!!!!!!!!!!!" <<endl;
	for(i=0; i<my_dali_timers.size(); i++){
		cout << "mess:" << my_dali_timers[i].mess <<  " id: " << my_dali_timers[i].id << " after_id: " << my_dali_timers[i].after_id << endl;
	}
	cout << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  " <<endl;
}

Json::Value hp_dali::get_shm_data()
{
	Json::Value res;
	int cc=0;
	for(auto i: my_dali_items){
		res[cc]["ident"] = i->get_desc();
		res[cc++]["status"] = patch::to_string(i->get_actual_value());
	}
	//cout << res << endl;
	return res;
}
void hp_dali::process_dali_response() 
{
	//read_startup_values();
	for(vector<dali_data_t *>::iterator it = my_dalinet_devices_data.begin(); it != my_dalinet_devices_data.end(); ++it){
		while((*it)->dali_responses.size() != 0){
	//		cout <<"Response v hlavnom threade: " <<  (*it)->dali_responses[0]<< endl;
			process_response((*it)->dali_responses[0],std::distance(my_dalinet_devices_data.begin(), it));

			(*it)->mtx.lock();
			(*it)->dali_responses.erase((*it)->dali_responses.begin());
			(*it)->mtx.unlock();
		}
	}
	
	check_timers();
}

void hp_dali::check_commands()
{
	for(int i=0; i<(int)my_dali_commands.size(); i++){
		if(my_dali_commands[i].send_time + 10 < time(NULL)){
			cout << "Chyba::Sprava bez odpovedede: " << my_dali_commands[i].mess << endl;
			my_dali_commands.erase(my_dali_commands.begin() + i);
			i--;
		}
	}
}

void hp_dali::check_timers()
{
	unsigned int i;
	for(i=0; i<my_dali_timers.size(); i++){
		if(--my_dali_timers[i].delay_counter <= 0){
			if(my_dali_timers[i].delay_type == DALI_DELAY_TIME){
				push_cmd(my_dali_timers[i].mess, my_dali_timers[i].dalinet_pos);
				my_dali_timers.erase(my_dali_timers.begin()+i);
				i--;
			} else {
				cout << "Mess timer failed: " << my_dali_timers[i].mess << endl;
				my_dali_timers.erase(my_dali_timers.begin()+i);
				i--;
			}
		}
	}
}

string hp_dali::create_mess(string mess, int item_pos)
{
	string res = "";
	int value = -1;
	//cout << "Message to cast: " << mess << endl;
	/*
	try {
		value = boost::lexical_cast<int>(mess.c_str());
	} catch  (const boost::bad_lexical_cast &) {
		cout << "Bad lexical cast... on: "<< mess << " value: " << value << endl;
	}
	*/
	value = patch::string2int(mess);
	if(item_pos < (int )my_dali_items.size()){
		res = my_dali_items[item_pos]->create_mess(value);
	}

	return res;
}

int hp_dali::process_gui_command(string mess)
{
	int item_pos = find_dali_item(mess.substr(0,mess.find("_")),"desc");
	//cout << "V hlavnom threade gui mess: " << mess << " prvok najdeny na pozicii: " << item_pos <<" query_value: " <<  mess.substr(mess.find("_")+1) <<  endl;
	if(item_pos != -1){
		int dalinet_pos = my_dali_items[item_pos]->get_dali_master_pos();
		string send_mess = create_mess(mess.substr(mess.find("_")+1),item_pos);
		if(send_mess != ""){
			push_cmd(send_mess,dalinet_pos);
		} else {
			cout << "Error nespravna sprava... :D " << mess << endl;
			return -1;
		}
	} else {
		item_pos = find_dali_group(mess.substr(0,mess.find("_")),"desc");
		if(item_pos != -1){
			int dalinet_pos = my_dali_groups[item_pos]->get_dali_master_pos();
			if(mess.substr(mess.find("_")+1).find("0") != std::string::npos){
				push_cmd(my_dali_groups[item_pos]->get_id()+"_off",dalinet_pos);
			} else {
				push_cmd(my_dali_groups[item_pos]->get_id()+"_on",dalinet_pos);
			}
		} else {
			return -1;
		}
	}
	return 0;
}

void hp_dali::push_cmd(string mess, int dalinet_pos, int mess_id)
{
	if(dalinet_pos >=0 && dalinet_pos < (int)my_dalinet_devices.size()){
		push_db_query("\t\t\t\t\t\t\t\t\tPosielam dali spravu: "+mess, DB_LOG_COM);
		my_dalinet_devices_data[dalinet_pos]->mtx.lock();
		my_dalinet_devices_data[dalinet_pos]->dali_commands.push_back(mess);
		my_dalinet_devices_data[dalinet_pos]->mtx.unlock();
		dali_commands_t tmp;
		tmp.send_time = time(NULL);
		tmp.dalinet_pos = dalinet_pos;
		if(mess_id == -1){
			tmp.id = ++my_delay_id;
		} else {
			tmp.id = mess_id;
		}
		tmp.mess = mess;
		my_dali_commands.push_back(tmp);
	} else {
		push_db_query("+++Chyba posielania dali spravy: " + mess + ", na dalinet: " + patch::to_string(dalinet_pos), DB_LOG_COM);
	}
	
}

unsigned int hp_dali::add_timer_item(int delay, int pos, string mess, int delay_type,unsigned int after_id, int button_pin_pos)
{
	bool add_mess = true;
	dali_timers_t tmp;
	tmp.id = ++my_delay_id;
	if(delay_type == DALI_DELAY_TIME){
		for(unsigned int i=0; i< my_dali_timers.size(); i++){
			if(my_dali_timers[i].dalinet_pos == pos && my_dali_timers[i].mess == mess){
				add_mess = false;
				my_dali_timers[i].delay_counter = delay;
				break;
			}
		}
		if(add_mess){
			tmp.delay_counter = delay;
			tmp.dalinet_pos = pos;
			tmp.mess = mess;
			tmp.delay_type = delay_type;
			tmp.button_pin_pos = button_pin_pos;
			my_dali_timers.push_back(tmp);
		}
	} else if(delay_type == DELAY_MESS){
		tmp.delay_counter = 60000;
		tmp.dalinet_pos = pos;
		tmp.mess = mess;
		tmp.delay_type = delay_type;
		tmp.after_id = after_id;
		tmp.button_pin_pos = button_pin_pos;
		//cout << "Pridavam spravu: " << mess << " after_id: " << after_id << endl;
		//for(unsigned int i=0; i< my_dali_timers.size(); i++){
		//	if(my_dali_timers[i].id == after_id){
		my_dali_timers.push_back(tmp);
		//		break;
		//	}
		//}
	}
	return tmp.id;
}

void hp_dali::process_Aresponse(string response, int dalinet_pos)
{
	push_db_query("\t\t\t\t\t\t\t\t\t\tDali response: " + response, DB_LOG_COM);
	int item_pos = find_dali_item(response.substr(0,response.find("_")), "id",dalinet_pos);
	if(item_pos != -1){
		int item_status = translate_resp_value(response,item_pos);
		if(item_status >= 0){
			//execute_query("UPDATE STATUSES set STATUS = '"+patch::to_string(item_status)+"' where ITEM = '"+my_dali_items[item_pos]->get_desc()+"'");
			push_db_query("UPDATE STATUSES set STATUS = '"+patch::to_string(item_status)+"' where ITEM = '"+my_dali_items[item_pos]->get_desc()+"'");
			if(my_shared_data != NULL){
				my_shared_data->mtx.lock();
				sht_mess_t tmp_sht;
				tmp_sht.type = "lightning";
				tmp_sht.ident = my_dali_items[item_pos]->get_desc();
				tmp_sht.status = patch::to_string(item_status);
				my_shared_data->mess.push_back(tmp_sht);
				my_shared_data->mtx.unlock();
			}
		} else {
			if(item_status == -1){
				//cout << "Neaktualizuje db, lebo hodnota je rovnaka... " << endl;
			} else if(item_status == -2){
				cout << "Item neodpoveda..." << endl;
			}
		}
	}
	if(response.find("A0") == std::string::npos){
		//add_timer_item(20, item_pos, response.substr(0, response.find("_")+1)+my_dali_items[item_pos]->get_query_cmd(), DALI_DELAY_TIME);
	}
}
void hp_dali::process_Gresponse(string response, int dalinet_pos)
{
	push_db_query("\t\t\t\t\t\t\t\t\t\tTOTOK nepouzivame !Dali response: " + response, DB_LOG_COM);
	int group_pos = find_dali_group(response.substr(0,response.find("_")), "id",dalinet_pos);
	if(group_pos != -1){
		vector<string> affected_items = my_dali_groups[group_pos]->get_group_items();
		//cout << "affected size: " << affected_items.size() << endl;
		if(affected_items.size() > 0){
			int value = -1;
			for(unsigned int i=0; i<affected_items.size(); i++){
				for(unsigned int j=0; j<my_dali_items.size(); j++){
					if(my_dali_items[j]->get_desc() == affected_items[i]){
						value = translate_resp_value(response, j);
					}
				}
			}
			value = calc_status4value(response);
			if(value != -1){
				string query = "UPDATE STATUSES set STATUS = '"+patch::to_string(value)+"' WHERE ITEM = '"+my_dali_groups[group_pos]->get_desc()+"' OR ";
				for(unsigned int i=0; i<affected_items.size(); i++){
					query += "ITEM = '"+affected_items[i]+"' OR ";
				}
				push_db_query(query.substr(0, query.length()-4));

			}
		}
	}
}
void hp_dali::process_BCresponse(string response, int dalinet_pos)
{
	int item_status = -1;
	string parsed_command = "";
	item_status= calc_status4value(response.substr(response.find_last_of("_")+1).c_str());
	
	if(item_status != -1){
		string query = "UPDATE STATUSES set STATUS = '"+patch::to_string(item_status)+"' where";//ITEM = '"+my_dali_items[item_pos]->get_desc()+"'");
		for(unsigned int i=0; i<my_dali_items.size(); i++){
			if(my_dali_items[i]->get_dali_master_pos() == dalinet_pos && my_dali_items[i]->get_item_function() == "light"){
				query += " ITEM = '"+my_dali_items[i]->get_desc()+"' OR";
			}
		}
		if(query[query.length()-1] == 'R'){
			query = query.substr(0, query.length()-3);
			execute_query(query.c_str());
		}
	}
}

void hp_dali::process_response(string response, int dalinet_pos)
{
	if(response[0] == 'A'){
		process_Aresponse(response, dalinet_pos);
	} else if(response[0] == 'G'){
		process_Aresponse(response, dalinet_pos);
	} else if(response.find("BC") != std::string::npos){
		process_BCresponse(response.substr(response.find("_")+1), dalinet_pos);
	}

	for(unsigned int i=0; i<my_dali_commands.size(); i++){
		//cout << my_dali_commands[i].mess << " vs " << response << " messid: " << my_dali_commands[i].id <<  endl;
		if(response.find(my_dali_commands[i].mess) != std::string::npos && dalinet_pos == my_dali_items[i]->get_dali_master_pos()){
			for(unsigned int j=0; j<my_dali_timers.size(); j++){
				if(my_dali_timers[i].after_id == my_dali_commands[i].id){
					push_cmd(my_dali_timers[i].mess, my_dali_timers[i].dalinet_pos, my_dali_timers[i].id);
					my_dali_timers.erase(my_dali_timers.begin()+j);
					break;
				}
			}
			my_dali_commands.erase(my_dali_commands.begin() + i);
			break;
		}
	}
}

int hp_dali::translate_resp_value(string resp, int item_pos)
{
	int res = -1;
	if(item_pos == -1){
		if(my_dali_items.size() > 0){
		}
	} else {
		if(resp.find("acr") != std::string::npos){
			/*
			try {
				res = my_dali_items[item_pos]->set_actual_value(boost::lexical_cast<int>(resp.substr(resp.find("acr")+4).c_str()));
			} catch  (const boost::bad_lexical_cast &) {
				//cout << "Bad lexical cast... on: "<< resp.substr(resp.find("acr")+3).c_str() << endl;
			}
			*/
			res = my_dali_items[item_pos]->set_actual_value(patch::string2int(resp.substr(resp.find("acr")+4)));
		} else if(resp.find("on") != std::string::npos){
			if(my_dali_items[item_pos]->get_item_type() == "pwm"){
				res = my_dali_items[item_pos]->set_actual_value(254);
			} else {
				res = my_dali_items[item_pos]->set_actual_value(1);
			}
		} else if(resp.find("off") != std::string::npos){
			res = my_dali_items[item_pos]->set_actual_value(0);
		} else if(resp.find("A0") != std::string::npos){
			if(resp.size() <= 5){
				return res = -2;
			}
			res = my_dali_items[item_pos]->set_actual_value(patch::string2int(resp.substr(resp.find("A0")+3)));
			/*
			try {
				res = my_dali_items[item_pos]->set_actual_value(boost::lexical_cast<int>(resp.substr(resp.find("A0")+3).c_str()));
			} catch  (const boost::bad_lexical_cast &) {
				//cout << "Bad lexical cast... on: "<< resp.substr(resp.find("A0")+3).c_str() << endl;
			}
			*/
		}
	}

	return res;
}

string hp_dali::calc_value4status(int status)
{
	string res= "0";
	if(status < (int)my_map_values.size()){
		res = patch::to_string(my_map_values[status]);
	}
	return res;
}

int hp_dali::calc_status4value(string value)
{
	//cout << "calc_status4valu: " << value << endl;
	int res = -1;
	if(value.find("acr") != std::string::npos){
		string cast = value.substr(value.find_last_of("_")+1);
		res = patch::string2int(cast);
		/*
		try {
			res = boost::lexical_cast<int>(cast);
		} catch  (const boost::bad_lexical_cast &) {
			//cout << "Bad lexical cast... on: "<< cast << endl;
		}
		*/
	} else if(value.find("on") != std::string::npos){
		res = 254;
	} else if(value.find("off") != std::string::npos){
		res = 0;
	} else if(value.find("A0") != std::string::npos){
		string cast = value.substr(value.find_last_of("_")+1);
		res = patch::string2int(cast);
		/*
		try {
			res = boost::lexical_cast<int>(cast);
		} catch  (const boost::bad_lexical_cast &) {
			//cout << "Bad lexical cast... on: "<< cast << endl;
		}
		*/
	}

	for(unsigned int i=0; i<my_map_values.size(); i++){
		if(i<my_map_values.size()-1){
			if(res >= my_map_values[i] && res < my_map_values[i+1]){
				res = i;
				break;
			}
		} else {
			res = i;
		}
	}
	return res;
}

int hp_dali::find_dali_group(string find, string property,int dalinet_pos, int start)
{
	int res = -1;
	if(property == "desc"){
		for(unsigned int i=start; i<my_dali_groups.size(); i++){
			if(my_dali_groups[i]->get_desc() == find){
				res = i;
				break;
			}
		}
	} else if (property == "id"){
		for(unsigned int i=start; i<my_dali_groups.size(); i++){
			if(my_dali_groups[i]->get_id() == find && my_dali_groups[i]->get_dali_master_pos() == dalinet_pos){
				res = i;
				break;
			}
		}
	}

	return res;
}

int hp_dali::find_dali_item(string find, string property,int dalinet_pos, int start)
{
	int res = -1;
	if(property == "desc"){
		for(unsigned int i=start; i<my_dali_items.size(); i++){
			if(my_dali_items[i]->get_desc() == find){
				res = i;
				break;
			}
		}
	} else if (property == "id"){
		for(unsigned int i=start; i<my_dali_items.size(); i++){
			if(my_dali_items[i]->get_id() == find && my_dali_items[i]->get_dali_master_pos() == dalinet_pos){
				res = i;
				break;
			}
		}
	}

	return res;
}

int hp_dali::fill_db(MYSQL *conn)
{
	string query= "", query_del = "DELETE FROM STATUSES where ";
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;

	query = "SELECT ITEM from STATUSES";
	query_state = mysql_query(conn, query.c_str());
	if(query_state != -1){
		result = mysql_store_result(conn);
		if(result->row_count > 0){
			while ((row = mysql_fetch_row(result)) != NULL ) {
				if(row[0] != NULL){
					items_idents.push_back(row[0]);
				}
			}
		}
		if(result != NULL){
			mysql_free_result(result);
		}
		query = "INSERT INTO STATUSES (ITEM, STATUS, MODIFICATION) VALUES ";
		for(unsigned int i=0; i<my_dali_items.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_dali_items[i]->get_desc()){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_dali_items[i]->get_desc()+"', '0', NOW()),";
			}
		}
		for(unsigned int i=0; i<my_dali_groups.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_dali_groups[i]->get_desc()){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_dali_groups[i]->get_desc()+"', '0', NOW()),";
			}
		}
		if(query[query.length()-1] != ' '){
			execute_query(query.substr(0,query.length()-1), conn);
		}
	}
	return 0;
}

int hp_dali::execute_query(string query, MYSQL *conn)
{
	int query_state;
	//cout << "query: " << query << endl;
	if(conn != NULL){
		query_state = mysql_query(conn, query.c_str());
		if (query_state != 0) {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
			return -1;
		}
	} else {
		conn = mysql_init(NULL);
		if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
			return -1;
		} else {
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if (query_state != 0) {
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout << bla << endl;
				//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
				mysql_close(conn);
#ifndef AXON_SERVER 
				mysql_library_end();
#endif
				conn = NULL;
				return -1;
			}
		}
		if(conn != NULL){
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
		}
	}
	return 0;
}


void hp_dali::delete_data()
{
	unsigned int i;
	for(i=0;i<my_dali_items.size(); i++){
		delete my_dali_items[i];
	}
	for(i=0;i<my_dali_groups.size(); i++){
		delete my_dali_groups[i];
	}
	for(i=0;i<my_dalinet_devices.size(); i++){
		delete my_dalinet_devices[i];
	}
	for(i=0;i<my_dalinet_devices_data.size(); i++){
		delete my_dalinet_devices_data[i];
	}
	for(i=0;i<my_dalinet_threads.size(); i++){
		delete my_dalinet_threads[i];
	}

}

void hp_dali::push_db_query(string query, int type, int log_level)
{
	cout << query << endl;
	hp_db_queries_t tmp;
	tmp.query = query;
	tmp.type = type;
	tmp.log_level = log_level;

	my_db_data->mtx.lock();
	my_db_data->queries.push_back(tmp);
	my_db_data->mtx.unlock();
}


hp_dali::~hp_dali()
{

}
