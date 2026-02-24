#include "../include/hp_sec_calc.h"

hp_sec_calc::hp_sec_calc(string user, string passwd, string db,  hp_sim_data_t *sim_data) : my_mysql_user(user), my_mysql_passwd(passwd), my_mysql_db(db)
{
	my_sim_data = sim_data;
}

void hp_sec_calc::operator()()
{
	//string query = "select * from all_statuses where cas like ";
	string day_minus;
	day_minus = patch::to_string(my_sim_data->day_minus);
	string query =" select ident,status,DATE_FORMAT(cas,\"%H:%i\") from all_statuses where cas like concat('%',(select DATE_SUB(DATE_FORMAT(now(),\"%Y-%m-%d\"),INTERVAL "+day_minus+" DAY)),'%') and ident NOT LIKE '%temp%' AND ident NOT LIKE '%pir%' AND ident != \"\" AND ident  NOT LIKE 'HZ%' ORDER BY cas;";
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	//cout << query << endl;

	conn = mysql_init(NULL);
	if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout <<"Mysql error: " << bla << ", query: " << query<< endl;
		mysql_close(conn);
#ifndef AXON_SERVER 
		mysql_library_end();
#endif
		conn = NULL;
		return ;
	} else {
		//query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if (query_state != 0) {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout <<"Mysql error: " << bla << " query: " << query << endl;
			//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
			return ;
		} else {
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL && row[2] != NULL){
						sim_data_t tmp;
						tmp.ident = row[0];
						tmp.to_value = row[1];
						string t_str = row[2];
						tmp.hour = patch::string2int(t_str.substr(0,2));
						tmp.min= patch::string2int(t_str.substr(3,2));
						my_sim_data->sim_data.push_back(tmp);
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}
		}
	}
	if(conn != NULL){
		mysql_close(conn);
#ifndef AXON_SERVER 
		mysql_library_end();
#endif
		conn = NULL;
	}
	my_sim_data->finished = true;
}

/*
int hp_sec_calc::execute_query(string query)
{
	my_open_counter = 0;
	int query_state;
	if(query.find("all_statuses") == std::string::npos){
		//cout << time(NULL) << "::query_thread: " << query << endl;
	}
	if(conn != NULL){
		query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if (query_state != 0) {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout <<"Mysql error: " << bla << ", query: " << query<< endl;
			//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
			return -1;
		}
	} else {
		conn = mysql_init(NULL);
		if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout <<"Mysql error: " << bla << ", query: " << query<< endl;
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
			return -1;
		} else {
			//query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if (query_state != 0) {
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout <<"Mysql error: " << bla << " query: " << query << endl;
				//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
				mysql_close(conn);
				mysql_library_end();
				conn = NULL;
				return -1;
			}
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}
	return 0;
}
*/

hp_sec_calc::~hp_sec_calc() {}
