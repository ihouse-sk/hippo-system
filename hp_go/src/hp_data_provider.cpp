#include "../include/hp_data_provider.h"


hp_data_provider::hp_data_provider(hp_incoming *data, string cnf_data)
{
	my_data_incoming = data;
	my_cnf_data = cnf_data;
}

string hp_data_provider::process_mess(string out_message)
{
	Json::Reader jr;
	Json::Value jrq;
	Json::Value jresp;
	Json::Value jrn;
	Json::FastWriter fr;
	string resp = "";
	try {
		bool b = jr.parse(out_message,jrq);
		if(b){
			if(!jrq["request"].empty()){
				if(jrq["request"] == "guiCmd"){
					//cout << jrq << endl;
					if(!jrq["payload"].empty() && !jrq["device_id"].empty() && !jrq["messId"].empty()){
						string tmp_str = "GUI_"+jrq["device_id"].asString()+"_"+jrq["payload"].asString();
						my_data_incoming->mtx_incomming.lock();
						my_data_incoming->mess.push_back(tmp_str);
						my_data_incoming->mtx_incomming.unlock();
						if(!jrq["client_id"].empty()){
							jresp["client_id"] = jrq["client_id"];
						}
						jresp["purpose"]="srv";
						jresp["type"] = "confirmation";
						jresp["data"]["mess_status"] = "1";
						jresp["data"]["mess_info"] = "Sprava bola uspesne dorucena";
						jresp["data"]["messId"] = jrq["messId"].asString();
						//cout << jresp << endl;
					} else {
						throw invalid_argument("Neplatny request, chyba paramter1 alebo device_id alebo chyba mess id\n");
					}
				} else if(jrq["request"] == "news"){
					if(!jrq["payload"].empty()){
						string news_type = jrq["payload"].asString();
						bool b1 = jr.parse(this->my_shm_data, jrn);
						if(b1){
							jresp["type"] = news_type;
							if(news_type == "lights"){
								jresp["data"] = jrn["statuses"];
							} else if(news_type == "heating"){
								jresp["data"] = jrn["heating"];
							} else if(news_type == "security"){
								jresp["data"] = jrn["security"];
							} else if(news_type == "eko"){
								jresp["data"] = jrn["statuses"];
							} else if(news_type == "doorman"){
								jresp["data"] = jrn["doorman"];
							} else if(news_type == "watering"){
								jresp["data"] = jrn["wateringV2"];
							} else if(news_type == "rekuperacia"){
								jresp["data"] = jrn["rekuperacia"];
							} else if(news_type == "meteo"){
								jresp["data"] = Json::arrayValue;										
								std::vector<string> meteo_data;
								meteo_data.push_back("outsideTemperature");
								meteo_data.push_back("temperature");
								meteo_data.push_back("senzorCO2");
								meteo_data.push_back("windchill");
								meteo_data.push_back("windspeed");
								meteo_data.push_back("rhumidity");
								meteo_data.push_back("winddirection");
								meteo_data.push_back("rpressure");
								meteo_data.push_back("lightMeteo");
								meteo_data.push_back("rainMeteo");
								for(auto mit:meteo_data){											
									for(auto it:jrn["statuses"]){
										if(it["ident"] == mit){
											jresp["data"][jresp["data"].size()] = it;
											break;
										}
									}
								}
								for(auto it:jrn["heating"]){
									Json::Value tmp_h;
									tmp_h["ident"] = it["id"];
									tmp_h["status"] = it["actual"];
									jresp["data"][jresp["data"].size()] = tmp_h;
								}
							} else {
								jresp = jrn;
							}
						} else {
							throw invalid_argument("Chyba citania zdielanej pamate\n");
						}
					} else {
						throw invalid_argument("Neplatny request, chyba paramter1 alebo device_id alebo chyba mess id\n");
					}
				} else if(jrq["request"] == "consumption"){
					if(!jrq["payload"].empty() && !jrq["device_id"].empty() ){
						jresp =  get_conspumption_data(jrq["payload"].asString(), jrq["device_id"].asString());
					} else {
						throw invalid_argument("Neplatny request, chyba paramter1 alebo device_id\n");
					}
				} else if(jrq["request"] == "getScenarioableDevices"){
					jresp =  get_scenarioable_devices();
				} else if(jrq["request"] == "scenarios"){
					jresp =  get_scenarios();
				} else if(jrq["request"] == "getAutomatedDevices"){
					jresp =  get_automated_devices();
				} else if(jrq["request"] == "getAutomatableDevices"){
					jresp =  get_automatable_devices();
				} else if(jrq["request"] == "devicesInScenario"){
					if(!jrq["payload"].empty()){
						jresp =  get_device_in_scenario(jrq["payload"].asString());
					} else {
						throw invalid_argument("Neplatny request, chyba id scenaria\n");
					}
				} else if(jrq["request"] == "thermostatSettings"){
					jresp =  get_thermostat_settings();
				} else if(jrq["request"] == "historyData"){
					jresp =  get_history_data(jrq,"ident");
				} else if(jrq["request"] == "heatData"){
					jresp =  get_history_data(jrq,"zone");
				} else if(jrq["request"] == "fibaro"){
					jresp =  get_history_data(jrq,"fibaro");
				} else if(jrq["request"] == "fibaroRoom"){
					jresp =  get_history_data(jrq,"fibaroRoom");
				} else if(jrq["request"] == "virtualPin"){
					my_data_incoming->mtx_incomming.lock();
					my_data_incoming->mess.push_back(jrq["value"].asString());
					my_data_incoming->mtx_incomming.unlock();
				} else {
					cout <<"Unknown requeset: " << jrq << endl; 
				}
				if(jresp.empty()){
					throw invalid_argument("Neplatny request\n");
				}
					 
				//*response << "HTTP/1.1 200 OK\r\n" << "Content-Length: " << resp.length() << "\r\nContent-type: application/json\r\n\r\n"<< resp +"\r\n";
			} else {
				throw invalid_argument("Neplatny request\n");
			}
		} else {
			throw invalid_argument("Json parse error: " + jr.getFormattedErrorMessages()+", mess: " +out_message);
		}
	} catch (const exception &e) {
		jresp.clear();
		jresp["data"]["mess_status"] = "-1";
		jresp["data"]["mess_info"] = "Chyba parsovania jsonu: " + string(e.what());
		jresp["type"] = "error";
		//*response << "HTTP/1.1 200 OK\r\n" << "Content-Length: " << ss_resp.length() << "\r\n\r\n"	<< ss_resp;
		//*response << "HTTP/1.1 400 Bad Request\r\n" << "Content-Length: " << ss_resp.length() << "\r\nContent-type: application/json\r\n\r\n"<< ss_resp;
	}
	if(!jrq["client_id"].empty()){
		jresp["client_id"] = jrq["client_id"];
	}
	jresp["purpose"]="srv";
	resp = fr.write(jresp);
	return resp;
}

void hp_data_provider::set_shm_data(string &data)
{
	my_shm_data = data;
}


Json::Value hp_data_provider::read_config_data()
{
	Json::Value obj;

	/*
	ifstream ifs;//("data.json");
	ifs.open("/opt/hp_cpp_server/data.json");
	if(!ifs.is_open()){
		ifs.open("data.json");
		if(!ifs.is_open()){
			return obj;
		}
	}
	*/

	Json::Reader reader;
	reader.parse(my_cnf_data, obj);
	
	return obj;
}

Json::Value hp_data_provider::get_scenarios()
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "SELECT id_scenaria,label from SCENARIA";
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL){
						js[row_counter]["id_scenaria"] = row[0];
						js[row_counter]["label"] = row[1];
						row_counter++;
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}

		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			cout << "query chyba " << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}
	Json::Value top;
	top["type"] = "scenarios";
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}

	return top;
}
Json::Value hp_data_provider::get_scenarioable_devices()
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "SELECT ident,label,type from SCENARIABLE_DEVICES";
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL && row[2] != NULL){
						js[row_counter]["ident"] = row[0];
						js[row_counter]["label"] = row[1];
						js[row_counter]["type"] = row[2];
						row_counter++;
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}

		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			cout << "query chyba " << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}

	Json::Value top;
	top["type"] = "getScenarioableDevices";
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}

	return top;
}
Json::Value hp_data_provider::get_device_in_scenario(string scen_id)
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "SELECT ident, to_value, (SELECT type FROM SCENARIABLE_DEVICES WHERE SCENARIA_CONFIG.ident = SCENARIABLE_DEVICES.ident) as TYPE, (SELECT label FROM SCENARIABLE_DEVICES WHERE SCENARIA_CONFIG.ident = SCENARIABLE_DEVICES.ident) as LABEL from SCENARIA_CONFIG WHERE id_scenaria = " +scen_id+ " ORDER BY TYPE ASC";
		cout << query << endl;
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL && row[2] != NULL && row[3] != NULL){
						js[row_counter]["ident"] = row[0];
						js[row_counter]["to_value"] = row[1];
						js[row_counter]["type"] = row[2];
						js[row_counter]["label"] = row[3];
						row_counter++;
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}

		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}

	Json::Value top;
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}


	return top;
}

Json::Value hp_data_provider::get_automatable_devices()
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "SELECT NAME,LABEL,TYPE from AUTOMATABLE_DEVICES";
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL && row[2] != NULL){
						js[row_counter]["name"] = row[0];
						js[row_counter]["label"] = row[1];
						js[row_counter]["type"] = row[2];
						row_counter++;
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}

		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}

	Json::Value top;
	top["type"] = "getAutomatableDevices";
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}


	return top;
}


Json::Value hp_data_provider::get_automated_devices()
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "SELECT ID, NAME, LABEL, COMMAND, TIME, (select TYPE from AUTOMATABLE_DEVICES where AUTOMATED_DEVICES.NAME = AUTOMATABLE_DEVICES.NAME limit 1) as TYPE from AUTOMATED_DEVICES ORDER BY TYPE ASC";
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL && row[2] != NULL && row[3] != NULL && row[4] != NULL && row[5] != NULL){
						js[row_counter]["id"] = row[0];
						js[row_counter]["name"] = row[1];
						js[row_counter]["label"] = row[2];
						js[row_counter]["command"] = row[3];
						js[row_counter]["time"] = row[4];
						js[row_counter]["type"] = row[5];
						row_counter++;
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}

		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}
	Json::Value top;
	top["type"] = "automatedDevices";
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}

	return top;
}

Json::Value hp_data_provider::get_thermostat_settings()
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "Select day, from_time, to_time, temp from termostat order by day,from_time";
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				string current_day = "1";
				int record_count = 0;
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL && row[2] != NULL && row[3]){
						istringstream ss(row[1]),s_to(row[2]);
						int from, to;
						ss >> from;
						s_to >> to;
						if(!ss.good() && !s_to.good()){
							if(current_day != row[0]){
								row_counter++;
								current_day = row[0];
								record_count=0;
							}
							js[row_counter]["day"] = row[0];
							js[row_counter]["data"][record_count]["from"]=from;								
							js[row_counter]["data"][record_count]["temp"] = row[3];
							record_count++;
						}
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}

		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}
	Json::Value top;
	top["type"] = "thermostatSettings";
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}

	return top;
}

Json::Value hp_data_provider::get_conspumption_data(string type, string period)
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}

	if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
		mysql_library_end();
		return "";
	} else {
		string query = "";
		if(period == "0") {
			query = "SELECT DATE, VALUE FROM `CONSUMPTION_DATA` WHERE ((DATE > DATE_SUB(NOW(), INTERVAL 25 HOUR)) AND ((FLAG = 4) AND TYPE = " + type + "))";
		} else if(period == "1") {
			query = "SELECT DATE, VALUE,(select SUM(VALUE) FROM CONSUMPTION_DATA where (DATE > DATE_FORMAT(NOW(), \"%y-%m-%d\")) AND FLAG = 4 AND type =" + type + ") as LATEST  FROM CONSUMPTION_DATA WHERE ((DATE > DATE_SUB(NOW(), INTERVAL 1 WEEK)) AND ((FLAG = 3) AND (TYPE = " + type + ")));";
		} else if(period == "2"){
			query = "SELECT DATE, VALUE,(select SUM(VALUE) FROM CONSUMPTION_DATA where (DATE > DATE_FORMAT(NOW(), \"%y-%m-%d\")) AND FLAG = 4 AND type =" + type + ") as LATEST FROM CONSUMPTION_DATA WHERE ((DATE > DATE_SUB(NOW(), INTERVAL 1 MONTH)) AND ((FLAG = 3) AND (TYPE = " + type + ")));";
		} else if(period == "3") {
			query = "SELECT DATE, VALUE FROM CONSUMPTION_DATA WHERE ((DATE > DATE_SUB(NOW(), INTERVAL 1 YEAR)) AND ((FLAG = 5) AND (TYPE = " + type + "))) UNION select DATE_FORMAT(NOW(),\"%Y-%m-01 00:00:29\") as DATE, SUM(VALUE) FROM CONSUMPTION_DATA where (DATE > DATE_FORMAT(NOW(), \"%Y-%m\")) AND FLAG = 4 AND type =" + type +";";
		}

		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		string latest_val = "";
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL ){
						js[row_counter]["date"] = row[0];
						if(row[1] != NULL){
							js[row_counter]["value"] = row[1];
						} else {
							js[row_counter]["value"] = "0";
						}
						if(period == "1" || period == "2"){
							if(row[2] != NULL){
								latest_val = row[2];
							} else {
								latest_val = "0";
							}
						}
						row_counter++;
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}
			if(row_counter > 0){
				if(period == "1" || period == "2"){
					time_t rawtime;
					struct tm * timeinfo;
					time ( &rawtime );
					timeinfo = localtime ( &rawtime );
					timeinfo->tm_hour = 0;
					timeinfo->tm_min = 0;
					timeinfo->tm_sec = 0;
					std::stringstream buffer;
					buffer << std::put_time(timeinfo, "%Y-%m-%d %T");
					js[row_counter]["date"] = buffer.str();
					js[row_counter]["value"] = latest_val;
				}
			}
		} else {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
		}
		if(conn != NULL){
			mysql_close(conn);
			mysql_library_end();
			conn = NULL;
		}
	}
	Json::Value top;
	if(!js.empty()){
		top["consumption"]=js;
	} else {
		top["consumption"][0]="";
	}

	return top;
}
Json::Value hp_data_provider::get_history_data(Json::Value jrq, string type)
{
	string res = "";
	Json::Value js;
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int row_counter=0;

	string db_user, db_passwd, db_name;
	Json::Value obj = read_config_data();
	if(obj["db_user"].empty() || obj["db_user"].empty() || obj["db_name"].empty()){
		return "";
	} else {
		db_user= obj["db_user"].asString();
		db_passwd= obj["db_passwd"].asString();
		db_name= obj["db_name"].asString();
	}
	if(jrq["period"].empty() || jrq["start"].empty() || jrq["ident"].empty()){
	} else {

		if((mysql_real_connect(conn,"localhost",db_user.c_str(),db_passwd.c_str(), db_name.c_str(),0,0,0)) == NULL){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			mysql_close(conn);
			mysql_library_end();
			return "";
		} else {
			string query = "";
			string q_time = "";
			time_t next_time = 0;
			if(jrq["period"].asString() == "day"){
				next_time = patch::string2int(jrq["start"].asString()) + 3600*24;
			} else if(jrq["period"].asString() == "week"){
				next_time = patch::string2int(jrq["start"].asString()) + 3600*24*7;
			} else if(jrq["period"].asString() == "month"){
				next_time = patch::string2int(jrq["start"].asString()) + 3600*24*31;
			}
			if(next_time == 0){
				return "";
			}
			q_time = " AND (select UNIX_TIMESTAMP(cas)) >= " +  jrq["start"].asString() + " AND (select UNIX_TIMESTAMP(cas)) <= " + patch::to_string(next_time);
			if(type == "zone"){
				query = "SELECT ident, status, cas, UNIX_TIMESTAMP(cas) as unix from all_statuses  where ident like 'H\%"+jrq["ident"].asString()+"' "+q_time+" order by id_all_statuses ASC;";
			} else if(type == "ident"){
				query = "SELECT ident, status, cas, UNIX_TIMESTAMP(cas) as unix from all_statuses  where ident = '"+jrq["ident"].asString()+"' "+q_time+" order by id_all_statuses ASC;";
			} else if(type == "fibaro"){
				q_time = " AND (select UNIX_TIMESTAMP(last_update)) >= " +  jrq["start"].asString() + " AND (select UNIX_TIMESTAMP(last_update)) <= " + patch::to_string(next_time);
				query = "SELECT ident, battery,value,target, device_type, last_update, UNIX_TIMESTAMP(last_update) as unix from fibaro_history where ident = '"+jrq["ident"].asString()+"' "+q_time+" order by id ASC;";
			} else if(type == "fibaroRoom"){
				q_time = " AND (select UNIX_TIMESTAMP(last_update)) >= " +  jrq["start"].asString() + " AND (select UNIX_TIMESTAMP(last_update)) <= " + patch::to_string(next_time);
				query = "SELECT ident, battery,value,target, device_type, last_update, UNIX_TIMESTAMP(last_update) as unix from fibaro_history where ident like '%"+jrq["ident"].asString()+"%' "+q_time+" order by id ASC;";
			}

			int query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL && row[1] != NULL && row[2] != NULL && row[3] != NULL){
							if(type != "fibaro"){
								js[row_counter]["ident"] = row[0];
								js[row_counter]["status"] = row[1];
								js[row_counter]["cas"] = row[2];
								js[row_counter]["unix"] = row[3];
								row_counter++;
							} else {
								js[row_counter]["ident"] = row[0];
								js[row_counter]["battery"] = row[1];
								js[row_counter]["value"] = row[2];
								js[row_counter]["target"] = row[3];
								js[row_counter]["device_type"] = row[4];
								js[row_counter]["cas"] = row[5];
								js[row_counter]["unix"] = row[6];
								row_counter++;
							}
						}
					}
				}
				if(result != NULL){
					mysql_free_result(result);
				}

			} else {
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout << bla << endl;
			}
			if(conn != NULL){
				mysql_close(conn);
				mysql_library_end();
				conn = NULL;
			}
		}
	}

	Json::Value top;
	if(!js.empty()){
		top["data"]=js;
	} else {
		top["data"]= Json::arrayValue;
	}

	return top;
}

string hp_data_provider::json_stringify(Json::Value val)
{
	Json::FastWriter fr;
	return fr.write(val);
}

hp_data_provider::~hp_data_provider(){}
