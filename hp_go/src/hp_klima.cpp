#include "../include/hp_klima.h"

hp_klima::hp_klima(XMLNode node,hp_db_data_t *db_data)
{
	//my_all_ident = xml_parser.get_node_value(node, "all", "ident");
	for(int i=0; i<node.getChildNode("sections").nChildNode("section"); i++){
		my_sections.push_back(hp_klima_section(node.getChildNode("sections").getChildNode("section",i)));
	}
	for(unsigned int i=0; i<MAX_FRAME; i++){
		my_frame_ids.push_back(0);
	}
	my_frame_counter = 0;
	my_db_data = db_data;
}

void hp_klima::init_section(string ident, float set_temp, float actual_temp, int running, int fan_speed, int mode, int louvre, int force_thermo)
{
	for(unsigned int i=0; i<my_sections.size(); i++){
		if(ident == my_sections[i].get_ident()){
			my_sections[i].set_query_temp(set_temp);
			my_sections[i].set_actual_temp(actual_temp);
			my_sections[i].set_state(running);
			my_sections[i].set_fan_speed(fan_speed);
			my_sections[i].set_mode(mode);
			my_sections[i].set_louvre(louvre);			
			my_sections[i].set_force_thermo(force_thermo);
		}
	}
}

int hp_klima::get_free_id()
{
	if(my_frame_counter >= MAX_FRAME){
		my_frame_counter = 0;
	}

	for(unsigned int i=my_frame_counter; i<my_frame_ids.size(); i++){
		if(my_frame_ids[i] == 0){
			my_frame_ids[i] = 1;
			return i;
		}
	}
	return 0;
}

int hp_klima::process_mess(vector<string> data)
{
	string air_mess= "";
	for(unsigned int i=0; i<my_sections.size(); i++){
		if(data[1] == my_sections[i].get_ident()){
			string tmp = "";
			if(data[2] == "temp"){
				tmp = "setTemp";
			} else if (data[2] == "onoff"){
				tmp = "onoff";
			} else if (data[2] == "speed"){
				tmp = "speed";
			} else if (data[2] == "mode"){
				tmp = "mode";
			} else if (data[2] == "louvre"){
				tmp = "louvre";
			} else if (data[2] == "force"){
				tmp = "force";
			}
			air_mess= "AIR_"+patch::to_string(get_free_id())+"_"+my_sections[i].get_klima_ident() + "_"+tmp+"_"+data[3];
			send_air_mess(my_sections[i].get_modbus_ip(), my_sections[i].get_modbus_port(), air_mess);
		}
	}
	return 0;
}

int hp_klima::process_air_resp(string mess)
{
	push_db_query("\t\t\t\t\t\t\t\t\t\tMess from Moxa: " + mess, DB_LOG_COM);
	vector<string> m_parsed = parse_response(mess);
	bool found = false;
	for(unsigned int i=0; i<my_send_messages.size(); i++){
		vector<string> parsed = parse_response(my_send_messages[i]);
		if(m_parsed[2] == parsed[1]){
			for(unsigned int j=0; j<my_sections.size(); j++){
				if(my_sections[j].get_klima_ident() == parsed[2]){
					if(m_parsed[3] == "OK"){
						string tmp = "";
						if(parsed[3] == "setTemp"){
							tmp = "setTemp";
							my_sections[j].set_query_temp(patch::string2float(parsed[4]));
						} else if (parsed[3] == "onoff"){
							tmp = "mrunning";
							my_sections[j].set_state(patch::string2int(parsed[4]));
						} else if (parsed[3] == "speed"){
							tmp = "fanspeed";
							my_sections[j].set_fan_speed(patch::string2int(parsed[4]));
						} else if (parsed[3] == "mode"){
							tmp = "mmode";
							my_sections[j].set_mode(patch::string2int(parsed[4]));
						} else if (parsed[3] == "louvre"){
							tmp = "louvre";
							my_sections[j].set_louvre(patch::string2int(parsed[4]));
						} else if (parsed[3] == "force"){
							tmp = "mforce";
							my_sections[j].set_force_thermo(patch::string2int(parsed[4]));
						}
						if(tmp != ""){
							push_db_query("update air_condition set "+tmp+" = "+parsed[4] + " where ident = '" + my_sections[j].get_ident()+"'");
							check_section(i);
						}
					} else {
						push_db_query("+++++Chyba pri nastavovani klimi: " + my_sections[j].get_ident(), DB_LOG_COM);
					}
					my_send_messages.erase(my_send_messages.begin()+i);
					found = true;
					break;
				}
			}
		}
		if(found){
			break;
		}
	}
	return 0;
}
void hp_klima::process_temp_mess(string sensor, float value)
{
	for(unsigned int i=0; i<my_sections.size(); i++){
		if(my_sections[i].get_sensor() == sensor){
			my_sections[i].set_actual_temp(value);
			push_db_query("update air_condition set actualTemp = "+patch::to_string(value)+ " where ident = '" + my_sections[i].get_ident()+"'");
			check_section(i);
		}
	}
}

void hp_klima::check_section(int pos)
{
	if(my_sections[pos].get_query_temp() <= my_sections[pos].get_actual_temp()){
				if(my_sections[pos].get_state() == 0){
					send_air_mess(my_sections[pos].get_modbus_ip(), my_sections[pos].get_modbus_port(),"AIR_"+patch::to_string(get_free_id())+"_"+my_sections[pos].get_klima_ident() + "_onoff_1");
					//zapni klimu
				}
			} else {
				if(my_sections[pos].get_state() == 1){
					send_air_mess(my_sections[pos].get_modbus_ip(), my_sections[pos].get_modbus_port(),"AIR_"+patch::to_string(get_free_id())+"_"+my_sections[pos].get_klima_ident() + "_onoff_0");
					//vypni klimu
				}
			}
}

void hp_klima::push_db_query(string query, int type, int log_level)
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

vector<string> hp_klima::parse_response(string resp, string separator)
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

void hp_klima::send_air_mess(string ip, int port, string mess)
{
	push_db_query("\t\t\t\t\t\t\t\t\tPosielam moxa data: " + mess + " ip: " + ip + " port: " + patch::to_string(port), DB_LOG_COM);
	thread th_air(hp_threaded_socket(port, ip, mess));
	th_air.detach();
	my_send_messages.push_back(mess);
}

string hp_klima::process_db(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT ident from air_condition";
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
		query = "INSERT INTO air_condition(ident, actualTemp, setTemp, mrunning, fanspeed, mmode, louvre,mforce) VALUES ";

		for(unsigned int i=0; i<my_sections.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_sections[i].get_ident() && my_sections[i].get_ident() != ""){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_sections[i].get_ident()+"','"+patch::to_string(my_sections[i].get_actual_temp())+"','"+patch::to_string(my_sections[i].get_query_temp())+"',0,0,0,0,0),";
			}
		}
		if(query[query.length()-1] != ' '){
			query = query.substr(0,query.length()-1);
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state != 0){
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout << "query: "<< query <<", error:  "<< bla << endl;
				//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
				return "-1";
			}
		}
		
		query = "select * from STATUSES where ";
		for(unsigned int i=0; i< my_sections.size(); i++){
			string sensors = my_sections[i].get_sensor();
			query += " ITEM = '"+sensors + "' ||";
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
							for(unsigned int i=0; i< my_sections.size(); i++){
								if(patch::string2float(string(row[2])) > 0){
									my_sections[i].set_actual_temp(patch::string2float(string(row[2])));
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
	}
	return "";
}


hp_klima::~hp_klima() {}
