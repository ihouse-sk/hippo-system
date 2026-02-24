#include "../include/hp_watering.h"

hp_watering::hp_watering(XMLNode node)
{
	my_watering_enabled= true;
	my_sensor_ident = "";
	if(string(xml_parser.get_node_value(node,"sensor","active"))=="yes"){
		my_sensor_ident = node.getChildNode("sensor").getAttribute("ident");
	}
	for(int i=0; i<node.nChildNode("section"); i++){
		my_wat_zones.push_back(hp_watering_zone(node.getChildNode("section",i)));
	}
	my_all_zone_ident = xml_parser.get_node_value(node,"all_zone_ident");
	my_max_running_zones = patch::string2int(xml_parser.get_node_value(node,"maximum_running_zones"));
	if(my_all_zone_ident != ""){
		my_wat_zones.push_back(my_all_zone_ident);
	}
}
string hp_watering::get_report()
{
	string res;
	res = "Watering, sensor ident: " +my_sensor_ident + " watering_enabled: " + patch::to_string(my_watering_enabled) +"\n"; 
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		res += "Zone id: " + xml_parser.fill_str(my_wat_zones[i].get_id(),25) +" dlzka polievania: " + xml_parser.fill_str(patch::to_string(my_wat_zones[i].get_time_period()),4) + ", auto enabled: " + patch::to_string(my_wat_zones[i].get_auto_enabled())+", running:"+patch::to_string(my_wat_zones[i].is_running())+", typ: "+patch::to_string(my_wat_zones[i].get_zone_type()) +"\n";
	}

	return res;
}

string hp_watering::add_zero(string str, int len)
{
	while((int)str.length() < len){
		str = "0"+str;
	}

	return str;
}

Json::Value hp_watering::get_shm_data(bool v2)
{
	Json::Value res;
	if(v2){
		for(unsigned int i=0; i<my_wat_zones.size(); i++){
			res[i]["ident"] = my_wat_zones[i].get_id();
			res[i]["running"] = (my_wat_zones[i].is_running()?1:0);
			res[i]["auto"] = my_wat_zones[i].get_auto_enabled();
			res[i]["timer"] = my_wat_zones[i].get_time_period();
			vector<int> times = my_wat_zones[i].get_start_times();
			vector<int> times_on = my_wat_zones[i].get_time_enabled();
			vector<int> day_enabled = my_wat_zones[i].get_week_day_enabled();
			for(unsigned int j=0; j<times.size(); j++){
				Json::Value tmp;
				string time_str = "";
				time_str += add_zero(patch::to_string(times[j]/60))+":"+add_zero(patch::to_string(times[j]%60))+":00";
				tmp["cas"] = time_str;
				tmp["isActive"] = times_on[j];
				res[i]["scheduler"][j] = tmp;
			}
			for(unsigned int j=0; j<day_enabled.size(); j++){
				res[i]["activeDays"][j] = day_enabled[j];
			}
		}
	//	cout << res << endl;
	} else {
		vector<string> days;
		days.push_back("monday");
		days.push_back("tuesday");
		days.push_back("wednesday");
		days.push_back("thursday");
		days.push_back("friday");
		days.push_back("sathurday");
		days.push_back("sunday");
		for(unsigned int i=0; i<my_wat_zones.size(); i++){
			res[i]["ident"] = my_wat_zones[i].get_id();
			res[i]["running"] = (my_wat_zones[i].is_running()?1:0);
			res[i]["auto"] = my_wat_zones[i].get_auto_enabled();
			res[i]["timer"] = my_wat_zones[i].get_time_period();
			vector<int> times = my_wat_zones[i].get_start_times();
			vector<int> times_on = my_wat_zones[i].get_time_enabled();
			vector<int> day_enabled = my_wat_zones[i].get_week_day_enabled();
			for(unsigned int j=0; j<times.size(); j++){
				stringstream ss;
				ss << "time_" << (j+1);
				string time_str = "";
				time_str += add_zero(patch::to_string(times[j]/60))+":"+add_zero(patch::to_string(times[j]%60))+":00";
				res[i][ss.str()] = time_str;
				ss.str("");
				ss << "time" << (j+1) << "ON";
				res[i][ss.str()] = times_on[j];
	
			}
			if(days.size() == day_enabled.size()){
				for(unsigned int j=0; j<day_enabled.size(); j++){
					res[i][days[j]] = day_enabled[j];
				}
			}
		}
	}

	return res;
}

void hp_watering::setup_zone(string zone_ident,int running, int auto_enabled, int timer, vector<int> start_time, vector<int> time_enabled, vector<int> week_day_enabled)
{
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		if(my_wat_zones[i].get_id() == zone_ident ){
			my_wat_zones[i].setup_zone(running, auto_enabled, timer, start_time, time_enabled, week_day_enabled);
			break;
		}
	}
}
vector<string> hp_watering::get_zones_ident()
{
	vector<string> res;
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		if(my_wat_zones[i].get_zone_type() == ZONE_SINGLE){
			res.push_back(my_wat_zones[i].get_id());
		}
	}
	return res;
}
vector<hp_wat_data_t> hp_watering::check_start(int now)
{
	vector<hp_wat_data_t> res;
	if(!my_watering_enabled){
	//	cout <<"nespustam polievanie lebo prsalo.... " << endl;
		return res;
	}
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		if(my_wat_zones[i].is_start_time(now)){
			hp_wat_data_t tmp;
			if(my_wat_zones[i].get_zone_type() == ZONE_ALL){
				tmp.query = my_wat_zones[i].get_id();
				res.clear();
				res.push_back(tmp);
				break;
			}
			vector<string> out = my_wat_zones[i].get_actors();
			int timer = my_wat_zones[i].get_time_period();
			if(timer > 0){
				map<string,int> data;
				for(unsigned int j=0; j<out.size(); j++){
					data.insert(std::pair<string,int>(out[j],timer));
				}
				my_wat_zones[i].set_running(1);
				tmp.out = data;
				tmp.query = "UPDATE watering set running = 1 where ident = '"+my_wat_zones[i].get_id()+"';";
				res.push_back(tmp);
			}
		}
	}
	return res;
}
vector<hp_wat_data_t> hp_watering::setup_wat_enabled(string ident, int value)
{
	vector<hp_wat_data_t> res;
	//cout <<"setting up watering enabled ident: " << ident << " vs " << my_sensor_ident << ", value; " << value << endl;
	if(ident == my_sensor_ident){
		if(value == 1){
			my_watering_enabled = false;
			for(unsigned int i=0; i<my_wat_zones.size(); i++){
				if(my_wat_zones[i].is_running()){
					map<string,int> data;
					vector<string> out = my_wat_zones[i].get_actors();
					for(unsigned int j=0; j<out.size(); j++){
						data.insert(std::pair<string,int>(out[j],0));
					}
					my_wat_zones[i].set_running(0);
					hp_wat_data_t tmp;
					tmp.out = data;
					res.push_back(tmp);
				}
			}
		} else {
			my_watering_enabled = true;
		}
	}
	//cout <<"Res size: " << res.size() << endl;
	return res;
}

vector<string> hp_watering::check_end(int now)
{
	vector<string> res;
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		if(my_wat_zones[i].is_end_time(now)) {
			my_wat_zones[i].set_running(0);
			res.push_back("UPDATE watering set running = 0 where ident = '"+my_wat_zones[i].get_id()+"'");
		}
	}
	return res;
}

hp_watering_zone *hp_watering::find_wat_zone(string ident)
{
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		if(my_wat_zones[i].get_id() == ident ){
			return &my_wat_zones[i];
		}
	}

	return NULL;
}

void hp_watering::update_zone(vector<string> parsed)
{
	for(unsigned int i=0; i<my_wat_zones.size(); i++){
		//cout << my_wat_zones[i].get_id() << " == " << parsed[2] << endl;
		if(my_wat_zones[i].get_id() == parsed[2]){
			if(parsed[1] == "time"){
				string date = parsed[4];
				if(parsed[3] == "1"){
					//cout <<"cas: " << patch::string2int(date.substr(0,date.find(":")))*60+patch::string2int(date.substr(date.find(":")+1)) << endl;
					my_wat_zones[i].set_start_time(patch::string2int(date.substr(0,date.find(":")))*60+patch::string2int(date.substr(date.find(":")+1)), 0);
				} else if(parsed[3] == "2"){
					my_wat_zones[i].set_start_time(patch::string2int(date.substr(0,date.find(":")))*60+patch::string2int(date.substr(date.find(":")+1)), 1);
				} else if(parsed[3] == "3"){
					my_wat_zones[i].set_start_time(patch::string2int(date.substr(0,date.find(":")))*60+patch::string2int(date.substr(date.find(":")+1)), 2);
				} else if(parsed[3] == "4"){
					my_wat_zones[i].set_start_time(patch::string2int(date.substr(0,date.find(":")))*60+patch::string2int(date.substr(date.find(":")+1)), 3);
				}
			} else if(parsed[1] == "auto"){
				if(parsed[3] == "time1"){
					my_wat_zones[i].set_time_enabled(patch::string2int(parsed[4]), 0);
				} else if(parsed[3] == "time2"){
					my_wat_zones[i].set_time_enabled(patch::string2int(parsed[4]), 1);
				} else if(parsed[3] == "time3"){
					my_wat_zones[i].set_time_enabled(patch::string2int(parsed[4]), 2);
				} else if(parsed[3] == "time4"){
					my_wat_zones[i].set_time_enabled(patch::string2int(parsed[4]), 3);
				}
			} else if(parsed[1] == "period"){
				if(parsed[3] == "monday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 0);
				} else if(parsed[3] == "tuesday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 1);
				} else if(parsed[3] == "wednesday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 2);
				} else if(parsed[3] == "thursday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 3);
				} else if(parsed[3] == "friday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 4);
				} else if(parsed[3] == "sathurday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 5);
				} else if(parsed[3] == "sunday"){
					my_wat_zones[i].set_week_day_enabled(patch::string2int(parsed[4]), 6);
				}
			} else if(parsed[1] == "hold"){
				my_wat_zones[i].set_timer(patch::string2int(parsed[3]));
			} else if(parsed[1] == "allauto"){
				my_wat_zones[i].set_auto_enabled(patch::string2int(parsed[3]));
			}
			break;
		}
	}
}

string hp_watering::process_db(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT ident from watering";
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
		query = "INSERT INTO watering (ident, running, auto, timer, time_1, time_2, time_3, time_4, time1ON, time2ON, time3ON, time4ON, monday, tuesday, wednesday, thursday, friday, sathurday, sunday) VALUES ";
		for(unsigned int i=0; i<my_wat_zones.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_wat_zones[i].get_id() && my_wat_zones[i].get_id() != ""){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_wat_zones[i].get_id()+"',0,0,0,'00:00:00','00:00:00','00:00:00','00:00:00',0,0,0,0,0,0,0,0,0,0,0),";
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
				conn = NULL;
				cout << conn << endl;
				if(conn != NULL){
					cout <<"FATAL ERRORR !!!!!!!!!!!!!!!!!!!!!!!@@@@@@@@@@@@@@@@@@@@@@#####################" <<endl;
				}
				return "-1";
			}
		}
	}
	return "";
}

hp_watering::~hp_watering() {}
