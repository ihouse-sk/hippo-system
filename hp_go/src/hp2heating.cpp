#include "../include/hp2heating.h"

hp2heating::hp2heating(XMLNode node, hp_db_data_t *db_data)
{
	my_has_master_switch = false;
	my_all_ident = xml_parser.get_node_value(node, "all", "ident");
	my_def_temp= patch::string2float(xml_parser.get_node_value(node, "all", "default_temp"));
	my_def_tempering_value = 18.0;
	my_actual_temp = my_def_temp;
	my_all_query_temp = my_def_temp;
	my_auto_query_temp = my_def_temp;
	my_ref_temp= xml_parser.get_node_value(node, "all", "reference_temp");
	my_eco_ident= xml_parser.get_node_value(node, "eco", "ident");
	my_eco_temp= patch::string2float(xml_parser.get_node_value(node, "eco", "default_temp"));
	my_heating_treshold = patch::string2float(xml_parser.get_node_value(node, "diff_heating_on"));
	for(int i=0; i<node.nChildNode("section"); i++){
		if(xml_parser.get_node_value(node.getChildNode("section",i),"heating_type") == "modbus"){
			my_heating_section_mdb.push_back(hp_heating_section_mdb(node.getChildNode("section",i)));
		} else {
			this->my_heating_section.push_back(hp_heating_section(node.getChildNode("section",i), my_heating_treshold,i*HEATING_OSC_PERIOD/node.nChildNode("section")));
		}
	}
	if(xml_parser.get_node_value(node,"master_switcher") == "yes"){
		my_has_master_switch = true;
		my_master_switch = xml_parser.get_node_value(node, "master_switcher", "ident");
		my_master_cooler= xml_parser.get_node_value(node, "master_cooler", "ident");
	} else {
		my_master_switch = "";
		my_master_cooler = "";
	}
	my_heating_mode = HEATING_OFF;
	my_db_data = db_data;
	my_ws_data[0] = "";
	my_use_ws = true;
}

string hp2heating::get_report()
{
	string res = "";
	res+=xml_parser.fill_str(my_all_ident,20) +"actual:"+ xml_parser.fill_str(patch::to_string(my_actual_temp),10)+ "query:" +xml_parser.fill_str(patch::to_string(my_all_query_temp),10) + "mode:" +xml_parser.fill_str(patch::to_string(my_heating_mode),10) + "\n"; 
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		res += xml_parser.fill_str(my_heating_section[i].get_id(),20) + "actual: " +xml_parser.fill_str(patch::to_string(my_heating_section[i].get_actual_temp()),10)+ "query: " + xml_parser.fill_str(patch::to_string(my_heating_section[i].get_query_temp()),10) + "Heater state: " +xml_parser.fill_str(patch::to_string(my_heating_section[i].get_heater_state()),5) + "Cooler state: " +xml_parser.fill_str(patch::to_string(my_heating_section[i].get_cooler_state()),5) + "Tempering start: " +xml_parser.fill_str(patch::to_string(my_heating_section[i].get_tempering_start()),5)+ "Tempring enabled: " +xml_parser.fill_str(patch::to_string(my_heating_section[i].is_tempering_enabled()),5)+ " Cas: " +xml_parser.fill_str(patch::to_string(my_heating_section[i].get_tempering_time()),5)+"\n";;
	}
	return res;
}
Json::Value hp2heating::get_shm_data()
{
	Json::Value res;
	string set_temp = "";
	if(my_heating_mode == 0){
		set_temp = "-1";
	} else if (my_heating_mode == 2){
		set_temp = "-2";
	} else if (my_heating_mode == 3){
		set_temp = "-3";
	}
	
	res[0]["id"]=this->my_all_ident;
	res[0]["actual"]=this->my_actual_temp;
	if(set_temp != ""){
		res[0]["setTemperature"]=set_temp;
	} else {
		res[0]["setTemperature"]=patch::to_string(this->my_all_query_temp);
	}
	res[0]["mode"]=this->my_heating_mode;
	res[0]["actor"]=0;

	for(auto it= this->my_heating_section.begin(); it != this->my_heating_section.end(); ++it){
		int dist = std::distance(my_heating_section.begin(), it)+1;
		res[dist]["id"]=it->get_id();
		res[dist]["actual"]=patch::to_string(it->get_actual_temp());
		if(set_temp != ""){
			res[dist]["setTemperature"]=set_temp;
		} else {
	//		cout <<"Set temp: " << patch::to_string(it->get_query_temp()) << ", zone: " << it->get_id() << endl;
			res[dist]["setTemperature"]=patch::to_string(it->get_query_temp());
		}
		res[dist]["mode"]=this->my_heating_mode;
		res[dist]["actor"]=it->get_section_state();
	}
//query += "('HZ"+my_heating_section_mdb[i].get_id()+"', "+patch::to_string(my_heating_section_mdb[i].get_query_temp())+", NOW() ),";
	for(auto i:this->my_heating_section_mdb){
		res[res.size()]["id"] = i.get_id();
		res[res.size()-1]["actual"] = patch::to_string(i.get_actual_temp());
		res[res.size()-1]["setTemperature"] = patch::to_string(i.get_query_temp());
		res[res.size()-1]["mode"] = this->my_heating_mode;
		res[res.size()-1]["actor"] = i.get_heater_state();			
	}

	//cout << res << endl;

	return res;
}

void hp2heating::push_ws_data(string type, string ident, string value, string ex1, string ex2)
{
	if(my_use_ws){
		my_ws_data[my_ws_data.size()]["type"] = type;
		my_ws_data[my_ws_data.size()-1]["ident"] = ident;
		my_ws_data[my_ws_data.size()-1]["status"] = value;
		if(type == "security"){
			my_ws_data[my_ws_data.size()-1]["alarm_countdown"] = ex1;
		} 
	}
}

void hp2heating::push_zones_states()
{
	string query = "INSERT INTO all_statuses (ident,status, cas) VALUES ";
	bool valid_query = false;
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		query += "('HQ"+my_heating_section[i].get_id()+"', "+patch::to_string(my_heating_section[i].get_query_temp())+", NOW() ),";
		query += "('HA"+my_heating_section[i].get_id()+"', "+patch::to_string(my_heating_section[i].get_section_state())+", NOW() ),";
		query += "('HT"+my_heating_section[i].get_id()+"', "+patch::to_string(my_heating_section[i].get_actual_temp())+", NOW() ),";
		valid_query = true;
	}
	for(unsigned int i=0; i<this->my_heating_section_mdb.size(); i++){
		query += "('HZ"+my_heating_section_mdb[i].get_id()+"', "+patch::to_string(my_heating_section_mdb[i].get_query_temp())+", NOW() ),";
		valid_query = true;
	}
	if(valid_query){
		push_db_query(query.substr(0,query.length()-1));
	}
}

void hp2heating::set_auto_query_temp (float temp){
	if(temp > 0) {
		my_auto_query_temp = temp;
		for(unsigned int i=0; i<this->my_heating_section.size(); i++){
			my_heating_section[i].set_auto_query_temp(temp);
		}
		for(unsigned int i=0; i<this->my_heating_section_mdb.size(); i++){
			my_heating_section_mdb[i].set_auto_query_temp(temp);
		}
	}
}

void hp2heating::set_all_query_temp(float temp) //{ my_all_query_temp = temp; }
{
	my_all_query_temp = temp;
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		my_heating_section[i].set_query_temp(temp);
	}
	for(unsigned int i=0; i<this->my_heating_section_mdb.size(); i++){
		my_heating_section_mdb[i].set_query_temp(temp);
	}
}

bool hp2heating::deactivate_tempering (string actor_ident)
{
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		if(my_heating_section[i].get_heater_state() == 0 && my_heating_section[i].has_heater(actor_ident)){
			my_heating_section[i].set_tempering_active(false);
			return true;
		}
	}
	return false;
}
vector<hp_heating_data_t> hp2heating::check_windows(string ident, int value)
{
	vector<hp_heating_data_t> res;
	return res;
}

vector<hp_heating_data_t> hp2heating::check_heating(int min)
{
	vector<hp_heating_data_t> res;
	hp_heating_data_t tmp;

	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		if(this->my_heating_mode == HEATING_MANUAL || this->my_heating_mode == HEATING_AUTO){
			int tempering_time = my_heating_section[i].check_tempering_start(min, this->my_heating_mode);
			if(tempering_time != -1){
				//cout << "delay: " << tempering_time << " pre zonu: " << my_heating_section[i].get_id() << " v case: " <<min<< endl;
				vector<hp_heating_data_t> tmp = my_heating_section[i].get_tempering_data();
				res.insert(res.end(), tmp.begin(), tmp.end());
			}
		}
		if(my_heating_section[i].has_main_vents()){
			vector<string> ventils = my_heating_section[i].get_main_vents();
			string vent_value = my_heating_section[i].get_heater_state() == 1?"1":"0";
			for(unsigned int j=0; j<ventils.size(); j++){
				tmp.ident = ventils[i];
				tmp.value = vent_value;
				tmp.off_delay = 0;
				res.push_back(tmp);
			}
		}
	}
	if(my_has_master_switch){
		string to_value = "0";
		for(unsigned int i=0; i<this->my_heating_section.size(); i++){
			if(my_heating_section[i].get_heater_state() == 1 || my_heating_section[i].get_cooler_state() == 1){
				to_value = "1";
				break;
			}
		}
		if(this->my_heating_mode == HEATING_COOLING && my_master_cooler != ""){
			tmp.ident = this->my_master_cooler;
			tmp.value = to_value;
			tmp.off_delay = 0;
			res.push_back(tmp);

			tmp.ident = this->my_master_switch;
			tmp.value = "0";
			tmp.off_delay = 0;
			res.push_back(tmp);
		} else {
			tmp.ident = this->my_master_switch;
			tmp.value = to_value;
			tmp.off_delay = 0;
			res.push_back(tmp);

			if(my_master_cooler != ""){
				tmp.ident = this->my_master_cooler;
				tmp.value = "0";
				tmp.off_delay = 0;
				res.push_back(tmp);
			}
		}
	}

	return res;
}

int hp2heating::process_thermostat_mess(vector<string> mess)
{
	push_db_query("DELETE FROM termostat where day = "+mess[2]);
	string query = "INSERT INTO termostat (day, from_time, to_time, temp) VALUES ";
	for(unsigned int i=3; i< mess.size(); i++){
		//cout <<"mess " << i << ": " << mess[i] << endl;
		query += "(" + mess[2] + ",";
		vector<string> partial = parse_response(mess[i],"-");
		for(unsigned int j=0; j<partial.size(); j++){
			if(j != partial.size()-1){
				int hour =  patch::string2int(partial[j].substr(0, partial[j].find(":")));
				int min = patch::string2int(partial[j].substr(partial[j].find(":")+1));
				query += patch::to_string(hour*60+min)+",";
				//query += partial[j].substr(0, partial[j].find(":")) + "," + partial[j].substr(partial[j].find(":")+1) + ",";
			} else {
				query += partial[j] + "),";
			}
		}
	}
	if(query[query.length()-1] == ','){
		query = query.substr(0, query.length()-1);
		push_db_query(query);
	}

	return 0;
}

vector<hp_heating_data_t> hp2heating::process_section_temp(string sensor_name, string temp, hp_notification *nn)
{
	if(sensor_name == this->my_ref_temp){
		push_db_query("UPDATE HEATING_ZONES set ACTUAL = '"+temp+"', MODIFICATION = NOW() where ID= '"+this->my_all_ident+"'"); 
		this->push_ws_data("heatingActual",my_all_ident,temp);
	}
	vector<hp_heating_data_t> res;
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		int sensor_state = my_heating_section[i].has_sensor(sensor_name);
		if(sensor_state != SENSOR_NON){
			if(sensor_state == SENSOR_REFERENCE){
				push_db_query("UPDATE HEATING_ZONES set ACTUAL = '"+temp+"', MODIFICATION = NOW() where ID= '"+my_heating_section[i].get_id()+"'"); 
				this->push_ws_data("heatingActual",my_heating_section[i].get_id(),temp);
				if(nn != NULL){
					nn->check_pin_notification(my_heating_section[i].get_id(), temp,"Zóna "+ my_heating_section[i].get_id());
				}
				my_heating_section[i].set_actual_temp(patch::string2float(temp));
			}
			if(!my_heating_section[i].is_active_tempering()){
				my_heating_section[i].set_temp4section(patch::string2float(temp), sensor_name);
				vector<hp_heating_data_t> tmp = my_heating_section[i].process_section(my_heating_mode, my_eco_temp, sensor_name);
				res.insert(res.end(), tmp.begin(), tmp.end());
				
				string out_value = "0";
				if(my_heating_section[i].get_heater_state() == 1){
					out_value = "1";
				}
				if(my_heating_section[i].get_cooler_state() == 1){
					out_value = "2";
				}
				push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(),ACTOR = "+out_value+" where ID= '"+my_heating_section[i].get_id()+"'"); 
				this->push_ws_data("heatingActor",my_heating_section[i].get_id(),out_value);
			}
		}
	}

	return res;
}

vector<hp_heating_data_t> hp2heating::process_section(string name, float query_temp)
{
	vector<hp_heating_data_t> res;
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		if(name == my_heating_section[i].get_id()){
			vector<hp_heating_data_t> tmp = my_heating_section[i].process_section(my_heating_mode, my_eco_temp);
			res.insert(res.end(), tmp.begin(), tmp.end());
			string out_value = "0";
			if(my_heating_section[i].get_heater_state() == 1){
				out_value = "1";
			}
			if(my_heating_section[i].get_cooler_state() == 1){
				out_value = "2";
			}
			push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(),ACTOR = "+out_value+", SET_TEMPERATURE = '"+patch::to_string(my_heating_section[i].get_query_temp())+"' where ID= '"+name+"'"); 
			this->push_ws_data("heatingActor",my_heating_section[i].get_id(),out_value);
			this->push_ws_data("heatingQueryTemp",my_heating_section[i].get_id(),patch::to_string(my_heating_section[i].get_query_temp()));
		}
	}
	return res;
}


vector<hp_heating_data_t> hp2heating::set_heating_mode(int mode, bool set_temp)
{
	if(mode != -1){
		my_heating_mode = mode;
	}
	vector<hp_heating_data_t> res;
	for(unsigned int i=0; i<this->my_heating_section.size(); i++){
		vector<hp_heating_data_t> tmp = my_heating_section[i].process_section(my_heating_mode, my_eco_temp);
		res.insert(res.end(), tmp.begin(), tmp.end());
		if(set_temp){
			push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = "+patch::to_string(my_heating_section[i].get_query_temp())+", ACTOR = "+my_heating_section[i].get_section_state()+" where ID= '"+my_heating_section[i].get_id()+"'"); 
			this->push_ws_data("heatingActor",my_heating_section[i].get_id(),my_heating_section[i].get_section_state());
			this->push_ws_data("heatingQueryTemp",my_heating_section[i].get_id(),patch::to_string(my_heating_section[i].get_query_temp()));
		 } else {
			push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), ACTOR = "+my_heating_section[i].get_section_state()+" where ID= '"+my_heating_section[i].get_id()+"'"); 
			this->push_ws_data("heatingActor",my_heating_section[i].get_id(),my_heating_section[i].get_section_state());
		}
	}
	return res;
}

hp_heating_section *hp2heating::find_section(string ident, string by)
{
	for(unsigned int i=0; i<my_heating_section.size(); i++){
		if(by == "ident"){
			if(my_heating_section[i].get_id() == ident){
				return &my_heating_section[i];
			}
		}
	}

	return NULL;
}

void hp2heating::process_mdb_response(hp_mdb_data_t *rx_data)
{
	rx_data->mtx.lock();
	unsigned char *ptr = rx_data->my_data[0].get_rx_ptr();
	//cout << "[TCP]>RX: ";
	for(int i=0; i<rx_data->my_data[0].get_data_len(); i++){
		//printf("%02x ",ptr[i]);
	}
	//cout << endl;
	//00 02 00 00 00 15 01 03 0c 00 00 00 c9 00 00 00 00 00 14 00 c8
	for(vector<hp_heating_section_mdb>::iterator it = my_heating_section_mdb.begin(); it != my_heating_section_mdb.end(); ++it){
		if(it->get_read_address() ==  rx_data->my_data[0].get_start_address()){
			float temp = 0,query=0;
			int actor=0;
			temp = (ptr[11] >> 8) + ptr[12];
			actor = (ptr[13] >> 8) + ptr[14];
			query = (ptr[19] >> 8) + ptr[20];
			it->set_actual_temp(temp/10);
			it->set_query_temp(query/10);
			push_db_query("UPDATE HEATING_ZONES set ACTOR= '"+patch::to_string(actor)+"',SET_TEMPERATURE = '"+patch::to_string(query/10)+"', ACTUAL = '"+patch::to_string(temp/10)+"', MODIFICATION = NOW() where ID= '"+it->get_id()+"'"); 
		}
	}
	rx_data->my_data[0].free_rx_data();
	rx_data->my_data.erase(rx_data->my_data.begin());
	rx_data->mtx.unlock();
}


void hp2heating::set_mdb_master_pos(vector<modbus_devices_t> mdb)
{
	for(vector<hp_heating_section_mdb>::iterator it = my_heating_section_mdb.begin(); it != my_heating_section_mdb.end(); ++it){
		for(uint16_t i=0; i<mdb.size(); i++){
			if(it->get_master_name() == mdb[i].ident){
				it->set_master_pos(i);
				break;
			}
		}
	}
}

void hp2heating::update_mdb_data(hp_mdb_data_t *tx_data,vector<modbus_devices_t> mdb)
{
//hp_modbus_data(string master_ip, int master_port, int start_address_dec, unsigned char function_code, int coils_num,unsigned char slave_address=1, unsigned char *data = NULL, int data_len=0);
//my_mdb_tx_data->my_data.push_back(hp_modbus_data("87.197.189.243",502,2033,3,1));
	for(vector<hp_heating_section_mdb>::iterator it = my_heating_section_mdb.begin(); it != my_heating_section_mdb.end(); ++it){
		tx_data->mtx.lock();			
	 	tx_data->my_data.push_back(hp_modbus_data(mdb[it->get_master_pos()].ip,mdb[it->get_master_pos()].port, it->get_read_address(), it->get_funct_code(),it->get_coils_num()));
		tx_data->mtx.unlock();			
	}
}

void hp2heating::push_db_query(string query, int type, int log_level)
{
	hp_db_queries_t tmp;
	tmp.query = query;
	tmp.type = type;
	tmp.log_level = log_level;

	my_db_data->mtx.lock();
	my_db_data->queries.push_back(tmp);
	my_db_data->mtx.unlock();
}

string hp2heating::process_db(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT ID from HEATING_ZONES";
	query_state = mysql_query(conn, "SET NAMES 'utf8'");
	query_state = mysql_query(conn, query.c_str());
	if(query_state == 0){
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
		query = "INSERT INTO HEATING_ZONES(ID, ACTUAL,SET_TEMPERATURE, MODIFICATION, MODE, ACTOR, COOLER) VALUES ";
		bool add_all_record = true;
		for(unsigned int j=0; j<items_idents.size(); j++){
			if(items_idents[j] == my_all_ident){
				add_all_record = false;
			}
		}
		if(add_all_record){
			query += "('"+my_all_ident+"',"+patch::to_string(my_actual_temp)+","+patch::to_string(my_def_temp)+",NOW(),0,0,0),";
		}

		for(unsigned int i=0; i<my_heating_section.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_heating_section[i].get_id() && my_heating_section[i].get_id() != ""){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_heating_section[i].get_id()+"','"+patch::to_string(my_heating_section[i].get_actual_temp())+"','"+patch::to_string(my_heating_section[i].get_query_temp())+"',NOW(),0,0,0),";
			}
		}
		for(unsigned int i=0; i<my_heating_section_mdb.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_heating_section_mdb[i].get_id() && my_heating_section_mdb[i].get_id() != ""){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_heating_section_mdb[i].get_id()+"','"+patch::to_string(my_heating_section_mdb[i].get_actual_temp())+"','"+patch::to_string(my_heating_section_mdb[i].get_query_temp())+"',NOW(),0,0,0),";
			}
		}
		if(query[query.length()-1] != ' '){
			query = query.substr(0,query.length()-1);
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state != 0){
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout << "query: "<< query <<",  "<< bla << endl;
				//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
				mysql_close(conn);
#ifndef AXON_SERVER 
				mysql_library_end();
#endif
				cout <<"Setting conn to NULL: " << conn <<  endl;
				conn = NULL;
				cout << conn << endl;
				if(conn != NULL){
					cout <<"FATAL ERRORR !!!!!!!!!!!!!!!!!!!!!!!@@@@@@@@@@@@@@@@@@@@@@#####################" <<endl;
				}
				return "-1";
			}
		}
		query = "SELECT * from termostat";
		query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		bool fill_termostat = true;
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				fill_termostat = false;
			}
			if(result != NULL){
				mysql_free_result(result);
			}
		}
		if(fill_termostat){
			query = "INSERT INTO termostat (day, from_time, to_time, temp) VALUES ";
			query += "(1,0,1439,20),";
			query += "(2,0,1439,20),";
			query += "(3,0,1439,20),";
			query += "(4,0,1439,20),";
			query += "(5,0,1439,20),";
			query += "(6,0,1439,20),";
			query += "(7,0,1439,20)";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state != 0){
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout << bla << endl;
				//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
				mysql_close(conn);
#ifndef AXON_SERVER 
				mysql_library_end();
#endif
				conn = NULL;
				return "-1";
			}
		}

		query = "select ID, ITEM, STATUS from STATUSES";
		string insert = "INSERT INTO STATUSES(ITEM, STATUS, MODIFICATION) VALUES ";
		vector<string>  cmp;
		query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[1] != NULL && row[2] != NULL){
						for(unsigned int i=0; i< my_heating_section.size(); i++){
							if(patch::string2float(string(row[2])) > 0){
								my_heating_section[i].set_temp4section(patch::string2float(string(row[2])), row[1]);
							}
						}
						if(string(row[1]) == "defTemperature"){
							my_def_temp = patch::string2float(string(row[2]));
						}
						if(string(row[1]) == "defTemperingValue"){
							my_def_tempering_value = patch::string2float(string(row[2]));
						}
						if(string(row[1]) == my_eco_ident){
							my_eco_temp = patch::string2float(string(row[2]));
						}
						cmp.push_back(row[1]);
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}
		}
		bool add = true;
		for(auto j: cmp){
			if("defTemperature" == j){
				add = false;
				break;
			}
		}
		if(add){
			insert += "('defTemperature', "+patch::to_string(my_def_temp)+",NOW()),";
		}

		add = true;
		for(auto j: cmp){
			if("defTemperingValue" == j){
				add = false;
				break;
			}
		}
		if(add){
			insert += "('defTemperingValue', "+patch::to_string(my_def_tempering_value)+",NOW()),";
		}

		add = true;
		for(auto j: cmp){
			if(my_eco_ident == j){
				add = false;
				break;
			}
		}
		if(add){
			insert += "('"+my_eco_ident+"', "+patch::to_string(my_eco_temp)+",NOW()),";
		}
		if(insert[insert.length()-1] == ','){
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, insert.substr(0,insert.length()-1).data());
		}
		/*
		//query = "INSERT INTO STATUSES(ITEM, STATUS, MODIFICATION) VALUES ('"+my_eco_ident+"', '"+patch::to_string(my_eco_temp)+"', NOW()) ON DUPLICATE KEY UPDATE MODIFICATION = NOW()";
		query = "INSERT INTO STATUSES(ITEM, STATUS, MODIFICATION) SELECT * FROM (SELECT '"+my_eco_ident+"', '"+patch::to_string(my_eco_temp)+"', NOW()) AS tmp WHERE NOT EXISTS (SELECT ITEM from STATUSES where ITEM = '"+my_eco_ident+"') LIMIT 1";
		query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state != 0){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
			return "-1";
		}

		query = "select * from STATUSES where ";
		for(unsigned int i=0; i< my_heating_section.size(); i++){
			vector<string> sensors = my_heating_section[i].get_sensors();
			for(unsigned j=0; j<sensors.size(); j++){
				query += " ITEM = '"+sensors[j] + "' ||";
			}
		}
		if(query[query.length()-1] == '|'){
			query = query.substr(0, query.length()-3);
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[1] != NULL && row[2] != NULL){
							for(unsigned int i=0; i< my_heating_section.size(); i++){
								if(patch::string2float(string(row[2])) > 0){
									my_heating_section[i].set_temp4section(patch::string2float(string(row[2])), row[1]);
								}
							}
						}
					}
				}
				if(result != NULL){
					mysql_free_result(result);
				}
			}
		}
		*/
	}
	return "";
}

vector<string> hp2heating::parse_response(string resp, string separator)
{
	vector<string> res;

	while(resp.find(separator) != std::string::npos){
		string tmp = resp.substr(0,resp.find(separator));
		res.push_back(tmp);
		resp = resp.substr(resp.find(separator)+1);
	}
	string tmp = boost::trim_right_copy(resp);
	//cout << "Pridamva: " << tmp<< " s dlzkou: " << tmp.length() << endl;
	if(tmp.length() > 0){
		res.push_back(tmp);
	}
	return res;
}


hp2heating::~hp2heating() {}
