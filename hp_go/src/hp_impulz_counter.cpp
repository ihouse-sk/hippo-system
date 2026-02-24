#include "../include/hp_impulz_counter.h"

hp_impulz_counter::hp_impulz_counter(XMLNode node, hp_db_data_t *data)
{
	//my_security_notification= xml_parser.get_node_value(node, "security_notification")=="yes"?true:false;
	for(int i=0; i<node.nChildNode("item"); i++){
		my_items.push_back(prepare_item(node.getChildNode("item",i)));
	}
	my_db_data = data;
}

impulz_items_t hp_impulz_counter::prepare_item(XMLNode node)
{
	impulz_items_t res;
	res.ident = xml_parser.get_node_value(node,"ident");
	res.off_interval = patch::string2int(xml_parser.get_node_value(node,"off_interval"));
	res.on_time = 0;
	res.pos = -1;
	res.active_value = "";
	res.impulz_len = 0;
	res.impulz_count = 0;

	return res;
}
void hp_impulz_counter::print_var()
{
	for(auto i = my_items.begin(); i != my_items.end(); i++){
		cout <<"Prvok: " << (*i).ident << ", pocet impulzov: " << (*i).impulz_count << ", dlzka impulzov: " << (*i).impulz_len << endl;
	}
}

void hp_impulz_counter::check_off_interval()
{
	for(auto i = my_items.begin(); i != my_items.end(); i++){
		if((*i).on_time != 0 && ( ((*i).on_time + (*i).off_interval) < time(NULL))) {
			(*i).impulz_len += (time(NULL)-(*i).on_time);
			(*i).on_time = 0;
		}
	}
}
void hp_impulz_counter::process_data()
{
	time_t rawtime;
	struct tm * ti;
	time ( &rawtime );
	ti= localtime ( &rawtime );
	string query_a = "";
	string query_h = "INSERT INTO impulz (ident, record_type, impulz_count, impulz_len,cas) VALUES ";
	string query_d = "INSERT INTO impulz (ident, record_type, impulz_count, impulz_len,cas) VALUES ";
	string query_m = "INSERT INTO impulz (ident, record_type, impulz_count, impulz_len,cas) VALUES ";

//	ti->tm_min = 0;
//	ti->tm_hour= 0;

	if(ti->tm_min == 0){
		CHECK();
		for(auto i = my_items.begin(); i != my_items.end(); i++){
			query_h += "('"+(*i).ident+"',"+RECORD_HOUR+","+patch::to_string((*i).impulz_count)+", "+patch::to_string((*i).impulz_len)+", DATE_ADD(NOW(), INTERVAL -1 HOUR)),";
			query_a = "UPDATE impulz set impulz_count = impulz_count + " + patch::to_string((*i).impulz_count)+ ", impulz_len = impulz_len + " + patch::to_string((*i).impulz_len)+ " where record_type = 0 AND ident = '"+(*i).ident+"'";
			if(ti->tm_hour == 0){
		CHECK();
				query_d += "('"+(*i).ident+"',"+RECORD_DAY+",(select value from (select sum(impulz_count) as value from impulz where record_type = "+RECORD_HOUR+" AND ident = '"+(*i).ident+"' AND cas < NOW() AND cas > DATE_ADD(NOW(),INTERVAL-1 DAY )) as i1),(select value from (select sum(impulz_len) as value from impulz where record_type = "+RECORD_HOUR+" AND ident = '"+(*i).ident+"' AND cas < NOW() AND cas > DATE_ADD(NOW(),INTERVAL-1 DAY )) as i2) , DATE_ADD(NOW(), INTERVAL -1 DAY)),";
				if(ti->tm_mday == 1){
					query_m += "('"+(*i).ident+"',"+RECORD_MONTH+",(select value from (select sum(impulz_count) as value from impulz where record_type = "+RECORD_DAY+" AND ident = '"+(*i).ident+"' AND cas < NOW() AND cas > DATE_ADD(NOW(),INTERVAL-1 MONTH)) as i1),(select value from (select sum(impulz_len) as value from impulz where record_type = "+RECORD_DAY+" AND ident = '"+(*i).ident+"' AND cas < NOW() AND cas > DATE_ADD(NOW(),INTERVAL-1 MONTH)) as i2) , DATE_ADD(NOW(), INTERVAL -1 MONTH)),";
	//				query_m += "('"+(*i).ident+"',"+RECORD_MONTH+",(select sum(impulz_count) from impulz where record_type = "+RECORD_DAY+" AND ident = '"+(*i).ident+"' AND cas < NOW() AND cas > DATE_ADD(NOW(),INTERVAL-1 MONTH)), (select sum(impulz_len) from impulz where record_type = "+RECORD_DAY+" AND ident = '"+(*i).ident+"' AND cas < NOW() AND cas > DATE_ADD(NOW(),INTERVAL-1 MONTH)), DATE_ADD(NOW(), INTERVAL -1 MONTH)),";
				}
			}
			(*i).impulz_len = 0;
			(*i).impulz_count = 0;
			if(query_a != ""){
				push_db_query(query_a);
				query_a = "";
			}
		}
		if(query_h[query_h.length()-1] != ' '){
			push_db_query(query_h.substr(0,query_h.length()-1));
		}
		if(query_d[query_d.length()-1] != ' '){
			push_db_query(query_d.substr(0,query_d.length()-1));
		}
		if(query_m[query_m.length()-1] != ' '){
			push_db_query(query_m.substr(0,query_m.length()-1));
		}
	}
}

void hp_impulz_counter::process_pin(int pos, string value)
{
	for(auto i = my_items.begin(); i != my_items.end(); i++){
		if((*i).pos == pos){
			if(value == (*i).active_value){
				(*i).impulz_count++;
				(*i).on_time = time(NULL);
			} else {
				if((*i).on_time != 0){
					if(time(NULL) == (*i).on_time){
						(*i).impulz_len++;
					} else {
						(*i).impulz_len += (time(NULL)-(*i).on_time);
					}
					(*i).on_time = 0;
				}
			}
			break;
		}
	}
}

void hp_impulz_counter::setup_items_position(const vector<hp_virtual_pin *> pins)
{
	for(uint16_t j=0; j<this->my_items.size(); j++){
		for(auto i = pins.begin(); i != pins.end(); ++i){
			if((*i)->get_desc() == my_items[j].ident){
				my_items[j].pos = std::distance(pins.begin(), i), (*i)->get_status();
				my_items[j].active_value = (*i)->is_inverz()?"0":"1";
				break;
				//my_conditions[j].setup_item((*i)->get_desc(), std::distance(pins.begin(), i), (*i)->get_status());
			}
		}
	}
}

void hp_impulz_counter::process_db(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT ident from impulz where record_type = " + string(RECORD_ACTUAL);
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
		query = "INSERT INTO impulz (ident, record_type, impulz_count, impulz_len,cas) VALUES ";
		for(auto i = my_items.begin(); i != my_items.end(); i++){
			bool add_item = true;
			for(auto j=items_idents.begin(); j != items_idents.end(); j++){
				if((*j) == (*i).ident){
					add_item = false;					
				}
			}
			if(add_item){
				query += "('"+(*i).ident+"',"+RECORD_ACTUAL+",0,0,NOW()),";
			}
		}
		if(query[query.length()-1] == ','){
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
				conn = NULL;
				if(conn != NULL){
					cout <<"FATAL ERRORR !!!!!!!!!!!!!!!!!!!!!!!@@@@@@@@@@@@@@@@@@@@@@#####################" <<endl;
				}
			}

		}
	}
}

void hp_impulz_counter::push_db_query(string query, int type, int log_level)
{
	if(my_db_data != NULL){
		cout << query << endl;
		hp_db_queries_t tmp;
		tmp.query = query;
		tmp.type = type;
		tmp.log_level = log_level;
	
		my_db_data->mtx.lock();
		my_db_data->queries.push_back(tmp);
		my_db_data->mtx.unlock();
	}
}



hp_impulz_counter::~hp_impulz_counter() {}
