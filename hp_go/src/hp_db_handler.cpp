#include "../include/hp_db_handler.h"

hp_db_handler::hp_db_handler(string user, string passwd, string db, hp_db_data_t *data) : my_mysql_user(user), my_mysql_passwd(passwd), my_mysql_db(db)
{
	my_db_data = data;
	my_hbxs_counter = 0;
	my_all_counter = 0;
	my_open_counter = 0;
	my_log_level = 10;
	my_multicasts_state = false;

	my_query_all_statuses = "";
	my_query_hbxs = "";
	conn = NULL;

	DIR *dp;
	if((dp = opendir("logs")) == NULL){
		mkdir("logs",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	} else {
		closedir(dp);
	}

	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );	
	
	std::stringstream s;
	
	s<<timeinfo->tm_mday<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_year+1900<<"_____"<<timeinfo->tm_hour<<"-"<<timeinfo->tm_min<<"-"<<timeinfo->tm_sec;
	
	std::string time = s.str();

	my_current_folder = "logs/"+time;
	my_current_hour = timeinfo->tm_hour;

	if(SYSTEM_TESTING){
		if(system("rm -r logs/*")) {}
	}
	
	if((dp = opendir(my_current_folder.c_str())) == NULL){
		mkdir(my_current_folder.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	} else {
		closedir(dp);
	}

	my_comm_log = my_current_folder+"/comm_log____"+time;
	my_last_hour = timeinfo->tm_hour;
	my_last_min = timeinfo->tm_min;
}

void hp_db_handler::operator()()
{
	time_t rawtime;
	struct tm * timeinfo;
	bool finished= false;
	while(1){
		my_all_counter++;
		//my_open_counter++;
		while(my_db_data->queries.size() > 0){
	///		cout <<"V handler: " << my_db_data->queries[0].query << " type: " << my_db_data->queries[0].type << endl;
			switch (my_db_data->queries[0].type) {
				case DB_STATUSES_QUERY: 
					execute_query(my_db_data->queries[0].query);
					my_db_data->queries[0].type = DB_LOG_COM;
					write_message(my_db_data->queries[0]);
					break;
				case DB_QUERY: 
					execute_query(my_db_data->queries[0].query);
					my_db_data->queries[0].type = DB_LOG_COM;
					write_message(my_db_data->queries[0]);
					break;
				case DB_SYSTEM_STATUS:
					execute_query(my_db_data->queries[0].query);
					break;
				case DB_ALL_STATUSES: 
					my_query_all_statuses += my_db_data->queries[0].query;
					//execute_query(my_db_data->queries[0].query);
					break;
				case DB_TRANSACTION:
					if(my_db_data->queries[0].query.find("COMMIT") != std::string::npos){
						this->my_transaction_queries.push_back(my_db_data->queries[0].query);
						process_transaction();
					} else {
						this->my_transaction_queries.push_back(my_db_data->queries[0].query);
					}
					break;
				case DB_LOG_COM:
					write_message(my_db_data->queries[0]);
					break;
				case DB_STOP:
					if(my_multicasts_state){
						stop_multicasts();
					}
					if(conn != NULL){
						cout <<"Closing conn because stop signal" << endl;
						mysql_close(conn);
#ifndef AXON_SERVER 
						mysql_library_end();
#endif
						conn = NULL;
					}
					finished = true;
					break;
				default: 
					break;
			}
			/*
			if(str.find("stop_thread") != std::string::npos){
				if(conn != NULL){
					cout <<"Closing conn.... " << endl;
					mysql_close(conn);
					mysql_library_end();
					conn = NULL;
				}
				finished = true;
			} else if(str.find("all_statuses") != std::string::npos){
			} else if(str.find("diagnostic") != std::string::npos){
			} else {
				execute_query(str);
			}
			*/
			my_db_data->mtx.lock();
			my_db_data->queries.erase(my_db_data->queries.begin());
			my_db_data->mtx.unlock();

		}
		if(ALL_STATUSES_TIME/(DB_SLEEP/1000) < my_all_counter){
			my_all_counter = 0;
		}
		/*
		if(my_open_counter > CONN_OPEN_TIME/(DB_SLEEP/1000)){
			if(conn != NULL){
				cout <<"Closing conn.... " << endl;
				mysql_close(conn);
				mysql_library_end();
				conn = NULL;
			}
			my_open_counter = 0;
		}
		*/
		if(my_multicasts_state){
			if(my_multicasts_start+60*60*8 < time(NULL)){
				stop_multicasts();
				cout <<"Deactivating multicast" << endl;
			}
		}
		if(finished){
			break;
		}
		rawtime=time(NULL);
		timeinfo = localtime ( &rawtime );	
		if(timeinfo->tm_min!= my_last_min){
			my_last_min = timeinfo->tm_min;
			execute_all_statuses();
		}
		if(timeinfo->tm_hour != my_last_hour){
			create_new_folders();	
		}
		std::this_thread::sleep_for(chrono::microseconds(DB_SLEEP));
	}
}
void hp_db_handler::create_new_folders()
{
	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );

	std::stringstream s;
	
	s<<timeinfo->tm_mday<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_year+1900<<"_____"<<timeinfo->tm_hour<<"-"<<timeinfo->tm_min<<"-"<<timeinfo->tm_sec;
	
	std::string time = s.str();

	my_current_hour = timeinfo->tm_hour;

	DIR *dp;
	if((dp = opendir(my_current_folder.c_str())) == NULL){
		mkdir(my_current_folder.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	} else {
		closedir(dp);
	}
	backup_logs();

	my_comm_log = my_current_folder+"/comm_log____"+time;
	my_last_hour = timeinfo->tm_hour;
}

void hp_db_handler::backup_logs()
{
	DIR *dp;
	string file_name = "old_logs";
	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );
	std::stringstream s;

	if((dp = opendir(file_name.c_str())) == NULL){
		mkdir(file_name.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	} else {
		closedir(dp);
	}
	
	if(timeinfo->tm_hour == 0){
		rawtime=time(NULL)-60*60;
		timeinfo = localtime ( &rawtime );
		s<<timeinfo->tm_mday<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_year+1900;
	} else {
		s<<timeinfo->tm_mday<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_year+1900;
	}
	
	std::string folder2copy = file_name + "/"+ s.str();
	if((dp = opendir(folder2copy.c_str())) == NULL){
		mkdir(folder2copy.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	} else {
		closedir(dp);
	}
	string sys_call = "mv "+my_comm_log + " " + folder2copy+"/"+my_comm_log.substr(my_comm_log.rfind("/")+1);
	if(system(sys_call.c_str() )) {}
}

std::string hp_db_handler::time_now()
{
	std::stringstream s;

	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );	
		
	s<<"["<<timeinfo->tm_mday<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_year+1900<<" "<<timeinfo->tm_hour<<":"<<timeinfo->tm_min<<":"<<timeinfo->tm_sec<<"]\t";

	return s.str();
}

void hp_db_handler::write_message(hp_db_queries_t data)
{
	cout << data.query << endl;
	if(data.log_level <= my_log_level){
		std::ofstream file;
		if(data.type == DB_LOG_COM){
			file.open(my_comm_log.c_str(),std::ios::app);
		}
		if (!file.is_open()) {
			std::cout<<"ERROR LOG CANNOT OPEN, file: " << my_comm_log << "\n";
			return ;
		}
		file<<time_now()<<" "<<data.query<<std::endl;
		if(data.query.find("initMulticasts") != std::string::npos){
			my_multicasts_state = create_multicasts();
		}
		if(my_multicasts_state){
			send_multicasts_data(time_now()+ " " + data.query );
		}
		if(data.query.find("killMulticasts") != std::string::npos){
			stop_multicasts();
		}
		file.close();
	}
}

bool hp_db_handler::create_multicasts()
{
	if(my_multicasts_state){
		return true;
	}
	if ((my_fd_cast=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		return false;
	}
	
	/* set up destination address */
	memset(&my_cast_addr,0,sizeof(my_cast_addr));
	my_cast_addr.sin_family=AF_INET;
	my_cast_addr.sin_addr.s_addr=inet_addr(HELLO_GROUP);
	my_cast_addr.sin_port=htons(HELLO_PORT);
	my_multicasts_start = time(NULL);
	return true;
}
void hp_db_handler::stop_multicasts()
{
	if(my_multicasts_state){
		close(my_fd_cast);
		my_multicasts_state = false;
	}
}
bool hp_db_handler::send_multicasts_data(string data)
{
	//cout <<"Posielam mess s dlzkou: " << strlen(data.c_str()) << endl;
	/*
	if(data.length() > 254){
		data = data.substr(0,254);
	}
	while(data.length() < 254){
		data.append("_");
	}
	*/
	if(sendto(my_fd_cast,data.c_str(),strlen(data.c_str()),0,(struct sockaddr *) &my_cast_addr,sizeof(my_cast_addr)) < 0) {
		return false;
	}
	return true;
}

int hp_db_handler::execute_all_statuses()
{
	if(my_query_all_statuses != ""){
		string q2exe = "INSERT INTO all_statuses (ident, status, cas) VALUES " + my_query_all_statuses.substr(0,my_query_all_statuses.length()-1);
		my_query_all_statuses = "";
		return execute_query(q2exe);
	}
	return 0;
}

void hp_db_handler::process_transaction()
{
	if(my_transaction_queries.size() > 0){
		int query_state;
		conn = mysql_init(NULL);
		if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
			return ;
		} else {
			//query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			for(uint16_t i = 0; i< my_transaction_queries.size(); ++i){
				query_state = mysql_query(conn, my_transaction_queries[i].c_str());
				if (query_state != 0) {
					char bla[200];
					strcpy(bla, mysql_error(conn));
					//cout <<"Mysql error: " << bla << " query: " << query << endl;
					//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
					mysql_close(conn);
#ifndef AXON_SERVER 
					mysql_library_end();
#endif
					conn = NULL;
					return ;
				}
			}
			
		}
		my_transaction_queries.clear();
		if(conn != NULL){
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
		}
	}
}

int hp_db_handler::execute_query(string query)
{
	my_open_counter = 0;
	int query_state;
	if(query.find("all_statuses") != std::string::npos){
	//	cout << time(NULL) << "::query_thread: " << query << endl;
	}
	//cout <<"Real execute: " << query << endl;

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
	return 0;
}

hp_db_handler::~hp_db_handler() {}
