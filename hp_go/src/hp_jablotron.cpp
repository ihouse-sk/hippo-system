#include "../include/hp_jablotron.h"

hp_jablotron::hp_jablotron(XMLNode node,hp_db_data_t *db_data)
{
	for(int i=0; i<node.nChildNode("section"); i++){
		my_zones.push_back(new hp_jablotron_zone(node.getChildNode("section",i)));
	}
	my_simulation_enabled = xml_parser.get_node_value(node,"simulation", "enabled") == "yes"?true:false;
	if(my_simulation_enabled){
		my_simulation_ident = xml_parser.get_node_value(node,"simulation", "ident");
	} else {
		my_simulation_ident = "";
	}
	my_db_data = db_data;
}

Json::Value hp_jablotron::get_shm_data()
{
	Json::Value res;
	for(unsigned int i=0; i<my_zones.size(); i++){
	//	cout << my_zones[i].get_id() << ": " << my_zones[i].get_sec_status() << endl;
		res[0]["id"] = my_zones[i]->get_ident();
		res[0]["status"] = my_zones[i]->get_zone_status();
		res[0]["actor"] = my_zones[i]->get_zone_status();
		res[0]["alarmCountdown"] = "-1";
		//	cout <<  (my_zones[i].get_id() +"_"+ patch::to_string(my_zones[i].get_sec_status()) + "_"+ patch::to_string(my_zones[i].get_current_countdown()) +";") << endl;
	}

	return res;
}
bool hp_jablotron::has_armed_zone()
{
	bool res = false;
	for(uint16_t i=0; i<this->my_zones.size(); i++){
		if(my_zones[i]->get_zone_status() == 1){
			res = true;
			break;
		}
	}
	return res;
}

bool hp_jablotron::all_zones_armed()
{
	bool res = true;
	for(uint16_t i=0; i<this->my_zones.size(); i++){
		if(my_zones[i]->get_zone_status() == 0){
			res = false;
			break;
		}
	}
	return res;
}

vector<string> hp_jablotron::get_armed_zones()
{
	vector<string> res;
	for(uint16_t i=0; i<this->my_zones.size(); i++){
		if(my_zones[i]->get_zone_status() == 1){
			res.push_back(my_zones[i]->get_ident());
		}
	}
	return res;
}

hp_jablotron_zone *hp_jablotron::find_zone(string id, string type)
{
	hp_jablotron_zone *res = NULL;
	for(uint16_t i=0; i<this->my_zones.size(); i++){
		if(type == "ident"){
			if(my_zones[i]->get_ident() == id){
				return my_zones[i];
			}
		}
		if(type == "sensor"){
			if(my_zones[i]->get_state_ident() == id){
				return my_zones[i];
			}
		}
	}

	return res;
}

string hp_jablotron::process_jablotron(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT ID from SECURITY_ZONES";
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
		query = "INSERT INTO SECURITY_ZONES (ID, MODIFICATION,STATUS,SENSOR,ACTOR,ALARM_COUNTDOWN) VALUES ";

		bool add_item = true;
		for(unsigned int i=0; i<my_zones.size(); i++){
			add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_zones[i]->get_ident() && my_zones[i]->get_ident() != ""){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_zones[i]->get_ident() +"', NOW(),0,0,0,-1),";
			}
		}
		if(query[query.length()-1] != ' '){
			query_state = mysql_query(conn, query.substr(0,query.length()-1).c_str());
		}
		if(this->my_simulation_enabled){
			query = "SELECT ITEM FROM STATUSES";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			bool add_sim_ident = true;
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL){
							if(string(row[0]) == my_simulation_ident){
								add_sim_ident = false;
								break;
							}
						}
					}
				}
				if(result != NULL){
					mysql_free_result(result);
				}
				if(add_sim_ident){
					query = "INSERT INTO STATUSES (ITEM, MODIFICATION,STATUS) VALUES ('"+my_simulation_ident+"', NOW(), 0)";
					query_state = mysql_query(conn, query.c_str());
				}
			}
		}
	}

	return "";
}

hp_jablotron::~hp_jablotron() 
{
	for(uint16_t i=0; i<my_zones.size(); i++){
		delete my_zones[i];
	}
}
