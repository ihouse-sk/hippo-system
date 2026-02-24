#include "../include/hp2go.h"

hp_go::hp_go(string conf_file): 
	my_data_provider(NULL),
	my_lightning(NULL),
	my_shutters(NULL),
	my_security(NULL),
	my_jablotron(NULL),
	my2heating(NULL),
	my_watering(NULL),
	my_cards(NULL),
	my_conditions(NULL),
	my_impulz_counter(NULL),
	my_notification(NULL),
	my_gates(NULL),
	my_rekuperacia(NULL),
	my_turnikets(NULL),
	my2modbus (NULL),
	my_mysql_db(""),  
	my_mysql_passwd("pwd"),
	my_mysql_user("hippo")
{
	my_socket =-1;
	my_conf_file = conf_file;
	my_simulator = false;
	my_has_electro_cons = false;
	my_last_min = -1;
	my_last_hour = -1;
	my_check_trusted_devices = false;
	my_system_start = time(NULL);
	my_use_old_hbx_timer = false;
	my_ws_data[0] = "";

	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	my_last_hour = timeinfo->tm_hour;
	my_has_hpp_web = true;
	my_use_https = 0;
	my_cpp_webport = 8080;
	
	my_cin_scen = NULL;
	my_cout_scen = NULL;
	my_dali = NULL;
	my_modbus = NULL;
	my_shm = NULL;
	my_use_ws = true;
	my2mdb_rx_data = NULL;
	my2mdb_tx_data = NULL;
	
#ifdef SIMULATION
	my_sim_data = NULL;
	my_sim_thread =NULL;
#endif
#ifdef AXON_DEBUG
	my_axon_file = "logs/"+patch::to_string(timeinfo->tm_hour)+"-"+patch::to_string(timeinfo->tm_min)+".txt";
#endif
}


int hp_go::init(bool reread)
{
	srand (time(NULL));
	this->read_config(); // nacitanie z xml-ka
	
	if(!reread){
		if(init_db() == -1){ // inicializacia databaze
			return -1;
		}
	}
	if(fill_db() == -1){ // vyplnenie potrebnych tabuliek v db
		return -1;
	}

	get_trusted_devices();

	if(!reread){
		if(my_modbus_devices.size()){
			my_mdb_tx_data = new hp_mdb_data_t();
			my_mdb_rx_data = new hp_mdb_data_t();
		}
		m_incomming = new hp_incoming(); // struktura na komunikaciu pre pocuvaci thread od drivera
		m_listen_thread = new hp_listen_thread(m_incomming,my_cls_socket); 
		bug_socket= new hp_client_socket(AF_UNIX,SOCK_STREAM, 0, "bug_socket");

		if(my_has_hpp_web){
			my_shm = new hp_shared_memory("hippoSharec");
			if(my_shm != NULL){
				my_shm->create_shm_object();
			}
		}
	
		my_sender = new hp_sender(my_xbee_socket,&my_db_data); // trieda, ktora sa stara o odosielanie sprav do drivera
		my_db_handler = new hp_db_handler(my_mysql_user,my_mysql_passwd, my_mysql_db,&my_db_data);  // trieda beziaca v samostatnom threade, uklada udaje do DB
		my_timing = new hp_timing(&my_timing_data);
		my_data_provider = new hp_data_provider(m_incomming,this->generate_json_wss());
		
		my_modbus = new hp_modbus(my_mdb_rx_data ,my_mdb_tx_data);
#ifdef SIMULATION
		my_sim_data = new hp_sim_data_t();
		my_sim_data->finished = false;
		my_simulation_state = SIM_OFF;
		my_sec_handler = new hp_sec_calc(my_mysql_user,my_mysql_passwd, my_mysql_db,my_sim_data);
#endif
	}

	return 0;
}

void hp_go::delete_objects(bool reread)
{
	if(!reread){
		if(my_modbus_devices.size()){
			delete my_mdb_tx_data;
			delete my_mdb_rx_data;
		}
		delete m_incomming;
		delete m_listen_thread;
		delete my_sender;
		delete my_db_handler;
		delete my_timing;
		if(my_data_provider != NULL){
			delete my_data_provider;
			my_data_provider = NULL;
		}
		if(my_has_hpp_web && my_shm != NULL){
			delete this->my_shm;
		}
		if(bug_socket != NULL){
			delete bug_socket;
		}
#ifdef SIMULATION
		delete my_sim_data;
		delete my_sec_handler;
#endif
	}
	my_alarm_checker.clear();
	my_meteo_idents.clear();

	for(vector<hp_virtual_pin *>::iterator it = my_pins.begin(); it != my_pins.end(); ++it){ 
		delete *it;
	}
	my_pins.clear();
	/*
	for(vector<hp_ext_pin*>::iterator it = my_ext_pins.begin(); it != my_ext_pins.end(); ++it){ 
		delete *it;
	}
	my_ext_pins.clear();
	*/
	for(auto it=my_button_scen.begin(); it != my_button_scen.end(); ++it){
		delete *it;
	}
	my_button_scen.clear();
	for(unsigned int i=0; i<my_hbxs.size(); i++){
		delete my_hbxs[i];
	}
	my_hbxs.clear();
	if(my_lightning != NULL){
		delete my_lightning;
		my_lightning = NULL;
	}
	if(my2heating != NULL){
		delete my2heating;
		my2heating = NULL;
	}
	if(my_jablotron != NULL){
		delete my_jablotron;
		my_jablotron= NULL;
	}
	if(my_security != NULL){
		delete my_security;
		my_security = NULL;
	}
	if(my_shutters != NULL){
		delete my_shutters;
		my_shutters = NULL;
	}
	if(my_watering != NULL){
		delete my_watering;
		my_watering = NULL;
	}
	if(my_conditions != NULL){
		delete my_conditions;
		my_conditions = NULL;
	}
	if(my_gates != NULL){
		delete my_gates;
		my_gates = NULL;
	}
	if(my_rekuperacia!= NULL){
		delete my_rekuperacia;
		my_rekuperacia = NULL;
	}
	if(my_cin_scen != NULL){
		delete my_cin_scen;
		my_cin_scen= NULL;
	}
	if(my_cout_scen != NULL){
		delete my_cout_scen;
		my_cout_scen = NULL;
	}
	if(my_cards != NULL){
		delete my_cards;
		my_cards = NULL;
	}
	if(my_notification != NULL){
		delete my_notification;
		my_notification = NULL;
	}
	if(my_dali!= NULL){
		delete my_dali;
		delete m_dali_sht;
		my_dali = NULL;
		m_dali_sht = NULL;
	}
	if(my_modbus != NULL){
		delete my_modbus;
		my_modbus = NULL;
	}
	if(my2modbus != NULL){
		delete my2modbus;
	}
	if(my2mdb_rx_data != NULL){
		delete my2mdb_rx_data;
	}
	if(my2mdb_tx_data != NULL){
		delete my2mdb_tx_data;
	}

	if(my_impulz_counter  != NULL){
		delete my_impulz_counter;
		my_impulz_counter = NULL;
	}
}

void hp_go::print_report()
{
	string report ="";
	for(unsigned int i=0; i<my_pins.size(); i++){
		report += my_pins[i]->get_report();
	}
	if(my2heating != NULL) {
		report +="\n";
		report += my2heating->get_report();
	}
	if(my_gates != NULL){
		report +="\n";
		report += my_gates->get_report();
	}
	if(my_watering!= NULL){
		report +="\n";
		report += my_watering->get_report();
	}
	push_db_query(report,DB_LOG_COM);
}

int hp_go::reread_xml()
{
	delete_objects(true);
	if(init(true) == -1){
		push_db_query("Error rereading xml!", DB_LOG_COM);
		return -1;
	} else {
		if(sync_with_db() == -1){ 
			return -1;
		}
		if(my_lightning != NULL){
			process_dn_change();
		}
		push_db_query("Rereading OK!", DB_LOG_COM);
		return 0;
	}
}

int hp_go::check_force_restart()
{
	ifstream file("../restart_required", std::ifstream::in);
	if(file.is_open()){
		string line;
		while(getline (file,line)){
			if(line == "1"){
				cout <<"restart ... " << endl;
				file.close();
				ofstream fw("../restart_required", std::ofstream::out);
				if(fw.is_open()){
					fw << "0";
					fw.close();
				}
				return -1;
			}
		}
	}
	return 0;
}

string hp_go::generate_json_wss()
{
	Json::Value wval;
	wval["db_user"] = my_mysql_user;
	wval["db_passwd"] = my_mysql_passwd;
	wval["db_name"] = my_mysql_db;

	Json::FastWriter jw;

	return jw.write(wval);
}

void hp_go::generate_json_web()
{
	bool restart_web = false;
	Json::Value obj,wval;
	Json::Reader reader;

	ifstream ifs;//("data.json");
	ifs.open("/opt/hp_cpp_server/data.json");
	if(!ifs.is_open()){
		restart_web = true;
	} else {
		reader.parse(ifs, obj);
		if(!obj["socket"].empty()){
			if(obj["socket"].asString() != my_cls_socket){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["db_user"].empty()){
			if(obj["db_user"].asString() != my_mysql_user){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["db_passwd"].empty()){
			if(obj["db_passwd"].asString() != my_mysql_passwd ){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["db_name"].empty()){
			if(obj["db_name"].asString() != my_mysql_db){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["shm_name"].empty()){
			if(obj["shm_name"].asString() != "hippoSharec"){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["http_folder"].empty()){
			if(obj["http_folder"].asString() != "test"){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["https"].empty()){
			if(obj["https"].asInt() != my_use_https){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
		if(!obj["port"].empty()){
			if(obj["port"].asInt() != my_cpp_webport){
				restart_web =true;
			}
		} else {
			restart_web = true;
		}
	}
	
	if(restart_web){
		wval["socket"] = my_cls_socket;
		wval["db_user"] = my_mysql_user;
		wval["db_passwd"] = my_mysql_passwd;
		wval["db_name"] = my_mysql_db;
		wval["shm_name"] = "hippoSharec";
		wval["http_folder"] = "test";
		wval["https"] = my_use_https?1:0;
		wval["port"] = my_cpp_webport;

		ofstream ofs;//("data.json");
		ofs.open("/opt/hp_cpp_server/data.json");
		if(ofs.is_open()){
			Json::StyledWriter jwr;
			ofs << jwr.write(wval);
			ofs.close();
			if(system("killall hp_cpp_server")) {}
		}
	}
}

int hp_go::read_config()
{
	XMLNode xml_main_node;
	XMLNode xml_node; 

	xml_main_node = XMLNode::openFileHelper(my_conf_file.c_str(),"hippo");
	
	my_mysql_db = my_xml_parser.get_node_value(xml_main_node,"db_name");
	my_log_level = atoi(my_xml_parser.get_node_value(xml_main_node,"log_level").c_str());
	my_cls_socket= my_xml_parser.get_node_value(xml_main_node,"cls_socket");
	my_xbee_socket= my_xml_parser.get_node_value(xml_main_node,"xbee_socket");
	my_simulator = my_xml_parser.get_node_value(xml_main_node, "simulator")=="yes"?true:false;
	my_system_identifier = my_xml_parser.get_node_value(xml_main_node,"logging", "system_ident");
	my_wss_ident = my_xml_parser.get_node_value(xml_main_node,"wss_ident") ;
	my_has_hpp_web = my_xml_parser.get_node_value(xml_main_node, "has_hpp_web")=="yes"?true:false;
	my_mysql_user = my_xml_parser.get_node_value(xml_main_node, "db_username") != ""?my_xml_parser.get_node_value(xml_main_node, "db_username"):my_mysql_user;
	if(my_has_hpp_web){
		my_cpp_webport= patch::string2int(my_xml_parser.get_node_value(xml_main_node,"has_hpp_web", "web_port"));
		my_use_https = my_xml_parser.get_node_value(xml_main_node, "has_hpp_web","hpp_ssl")=="yes"?true:false;
	}
	my_use_old_hbx_timer= my_xml_parser.get_node_value(xml_main_node, "old_hbx_timer")=="yes"?true:false;
	if(my_xml_parser.get_node_value(xml_main_node,"meteo_stanica") == "yes"){
		my_meteo_idents.push_back(meteo_cnt_t{"rpressure",""});
		my_meteo_idents.push_back(meteo_cnt_t{"windspeed",""});
		my_meteo_idents.push_back(meteo_cnt_t{"windgust",""});
		my_meteo_idents.push_back(meteo_cnt_t{"winddirection",""});
		my_meteo_idents.push_back(meteo_cnt_t{"rhumidity",""});
		my_meteo_idents.push_back(meteo_cnt_t{"temperature","25"});
		my_meteo_idents.push_back(meteo_cnt_t{"windchill","30"});
	}
	
	for(int i=0; i<xml_main_node.getChildNode("hbxs").nChildNode("hbx"); i++){
		string hbx_active = my_xml_parser.get_node_value(xml_main_node.getChildNode("hbxs"),"hbx","status",i);
		//if(hbx_active == "active"){
			my_hbxs.push_back(new hp_hbx(xml_main_node.getChildNode("hbxs").getChildNode("hbx",i),i+1, hbx_active=="active")); // vytvorenie noveho objektu triedy HBX a vlozenie do vektora hbx
			xml_node =xml_main_node.getChildNode("hbxs").getChildNode("hbx",i);
			for(int j=0; j<xml_node.nChildNode("pin"); j++){ // iteracia cez vsetky piny daneho HBX a vlozenie novych objektov prislusnych pinov do vektora hp_virtual_pin, pricom vsetky vlozene triedy dedia od tejto triedy
				string pin_function = my_xml_parser.get_node_value(xml_node.getChildNode("pin",j),"desc_2","function");
			//	cout <<"Pin function: " << pin_function << ": " << my_xml_parser.get_node_value(xml_node.getChildNode("pin",j),"desc","value") << endl;
				if(pin_function == "status_light" || pin_function == "socket" || pin_function == "doorLock" || pin_function == "gate" || pin_function == "gateDoublePin"|| pin_function == "sprinkler" || pin_function == "pwmOnOff" || pin_function == "GSM" || pin_function == "fountain" || pin_function == "ventilator"){
					my_pins.push_back(new hp_pin_status_light(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				//	sset.insert(new hp_pin_status_light(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				} else if (pin_function == "pushButton" || pin_function == "multi_button" ||  pin_function == "pir"){
					my_pins.push_back(new hp_push_button(xml_node.getChildNode("pin",j),i+1));    // trieda hp_push_button
				//	sset.insert(new hp_push_button(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				} else if(pin_function == "pwm"){
					my_pins.push_back(new hp_pin_pwm(xml_node.getChildNode("pin",j),i+1)); 	// trieda hp_pin_pwn
				//	sset.insert(new hp_pin_pwm(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				} else if(pin_function.find("shutter")!= std::string::npos){
					my_pins.push_back(new hp_pin_shutter(xml_node.getChildNode("pin",j),i+1));   // trieda hp_pin_shutter
				//	sset.insert(new hp_pin_shutter(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				} else if(pin_function == "onoffButton" || pin_function == "magKontakt" || pin_function == "dayTime"){
					my_pins.push_back(new hp_onOffButton(xml_node.getChildNode("pin",j),i+1));   // trieda hp_onOffButton
				//	sset.insert(new hp_onOffButton(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				} else if(pin_function == "temperature"){
					my_pins.push_back(new hp_pin_temp(xml_node.getChildNode("pin",j),i+1));   // trieda hp_pin_temp
				//	sset.insert(new hp_pin_temp(xml_node.getChildNode("pin",j),i+1)); // trieda hp_pin_status_light
				} else if(pin_function == "heater" || pin_function == "cooler"){
					my_pins.push_back(new hp_pin_heater(xml_node.getChildNode("pin",j),i+1));   // trieda hp_pin_temp
				} else if(pin_function == "electro_cons" || pin_function == "gas_cons"){
					my_pins.push_back(new hp_pin_cons(xml_node.getChildNode("pin",j),i+1));   // trieda hp_pin_temp
					my_has_electro_cons = true;
				}
			}
		//}
	}
	for(uint16_t i=0; i<my_pins.size(); i++){
		if(my_pins[i]->get_periodicity() != -1){
			my_periodic_pins.push_back(i);
		}
	}
	create_driver_xml(xml_main_node.getChildNode("driver")); // vytvorenie xml pre driver
	generate_json_web();

	for(int i=0; i<xml_main_node.getChildNode("modbus_devices").nChildNode("modbus_device"); i++){  // spracovanie sluzieb
		xml_node = xml_main_node.getChildNode("modbus_devices").getChildNode("modbus_device",i);
		modbus_devices_t tmp;
		tmp.ident = my_xml_parser.get_node_value(xml_node,"id");
		tmp.type = my_xml_parser.get_node_value(xml_node,"modbus_type");
		tmp.ip= my_xml_parser.get_node_value(xml_node,"ip");
		tmp.port= patch::string2int(my_xml_parser.get_node_value(xml_node,"port"));
		my_modbus_devices.push_back(tmp);
	}

	for(int i=0; i<xml_main_node.getChildNode("services").nChildNode("service"); i++){  // spracovanie sluzieb
		xml_node = xml_main_node.getChildNode("services").getChildNode("service",i);
		string service = xml_node.getAttribute("id");
		if( service.find("lightning") != std::string::npos){
			my_lightning = new hp_lightning(xml_node); // vytvorenie triedy zodpovednej za ovladanie osvetlenia
		}
		if( service == "shutters"){
			my_shutters= new hp_shutters(xml_node, my_pins);    // vytvorenie triedy zodpovednej za ovladanie zaluzii
		}
		if( service == "security"){
			my_security = new hp_security(xml_node);    // vytvorenie triedy zodpovednej za ovladanie  zabezpecovacieho systemu
		}
		if( service == "jablotron"){
			my_jablotron= new hp_jablotron(xml_node);    // vytvorenie triedy zodpovednej za ovladanie  zabezpecovacieho systemu
		}
		if( service == "heat_s"){
			my2heating = new hp2heating(xml_node,&my_db_data);    // vytvorenie triedy zodpovednej za ovladanie  kurenia noveho
		}
		if( service.find("watering") != std::string::npos){
			my_watering= new hp_watering(xml_node);    // vytvorenie triedy zodpovednej za ovladanie  zabezpecovacieho systemu
		}
		if( service == "gates"){
			my_gates= new hp_gates(xml_node);    // vytvorenie triedy zodpovednej za ovladanie  bran
		}
		if( service == "rekuperacia"){
			my_rekuperacia = new hp_rekuperacia(xml_node);
		}
		if( service == "conditions"){
			my_conditions= new hp_conditions(xml_node,&my_db_data);
			my_conditions->setup_items_position(my_pins);
		}
		if( service == "impulz_counter"){
			my_impulz_counter = new hp_impulz_counter(xml_node,&my_db_data);
			my_impulz_counter->setup_items_position(my_pins);
			//my_conditions->setup_items_position(my_pins);
		}
		if( service == "notification"){
			my_notification= new hp_notification(xml_node,my_system_identifier, &my_db_data,this->my_simulator);
			my_notification->setup_guards_position(my_pins);
		}
		if( service == "turnikets"){
			my_turnikets= new hp_turnikets(xml_node);
		}
		if(service == "external_pins"){
			for(int j=0; j<xml_node.nChildNode("pin"); j++){
				string pin_function = my_xml_parser.get_node_value(xml_node.getChildNode("pin",j),"desc_2","function");
				if(pin_function == "status_light" || pin_function == "socket" || pin_function == "doorLock" || pin_function == "gate" || pin_function == "gateDoublePin"|| pin_function == "sprinkler" || pin_function == "pwmOnOff" || pin_function == "GSM" || pin_function == "fountain" || pin_function == "ventilator"){
					my_pins.push_back(new hp_pin_status_light(xml_node.getChildNode("pin",j),-1)); // trieda hp_pin_status_light
				} else if (pin_function == "pushButton" || pin_function == "multi_button" ||  pin_function == "pir"){
					my_pins.push_back(new hp_push_button(xml_node.getChildNode("pin",j),-1));    // trieda hp_push_button
				} else if(pin_function == "pwm"){
					my_pins.push_back(new hp_pin_pwm(xml_node.getChildNode("pin",j),-1)); 	// trieda hp_pin_pwn
				} else if(pin_function.find("shutter")!= std::string::npos){
					my_pins.push_back(new hp_pin_shutter(xml_node.getChildNode("pin",j),-1));   // trieda hp_pin_shutter
				} else if(pin_function == "onoffButton" || pin_function == "magKontakt" || pin_function == "dayTime"){
					my_pins.push_back(new hp_onOffButton(xml_node.getChildNode("pin",j),-1));   // trieda hp_onOffButton
				} else if(pin_function == "temperature"){
					my_pins.push_back(new hp_pin_temp(xml_node.getChildNode("pin",j),-1));   // trieda hp_pin_temp
				} else if(pin_function == "heater" || pin_function == "cooler"){
					my_pins.push_back(new hp_pin_heater(xml_node.getChildNode("pin",j),-1));   // trieda hp_pin_temp
				} else if(pin_function == "electro_cons" || pin_function == "gas_cons"){
					my_pins.push_back(new hp_pin_cons(xml_node.getChildNode("pin",j),-1));   // trieda hp_pin_temp
					my_has_electro_cons = true;
				}
				//my_ext_pins.push_back(new hp_ext_pin(xml_node.getChildNode("pin",j)));
			}
		}
		if(service == "button_scenarios"){
			for(int j=0; j<xml_node.nChildNode("scenario"); j++){
				my_button_scen.push_back(new hp_button_scenario(xml_node.getChildNode("scenario",j),my_xml_parser.get_node_value(xml_node, "scenario","ident",j),my_xml_parser.get_node_value(xml_node, "scenario","label",j)));
			}
		}
		if(service == "advanced_scenarios"){
			for(int j=0; j<xml_node.nChildNode("scenario"); j++){
				string scen_ident = my_xml_parser.get_node_value(xml_node,"scenario","ident",j);
				if(scen_ident == "cin" && my_cin_scen == NULL){
					my_cin_scen = new hp_advanced_scenario(xml_node.getChildNode("scenario",j), scen_ident);
				}
				if(scen_ident == "cout" && my_cout_scen == NULL){
					my_cout_scen = new hp_advanced_scenario(xml_node.getChildNode("scenario",j), scen_ident);
				}
			}
		}
	}
	if(!xml_main_node.getChildNode("dali").isEmpty()){
		m_dali_sht = new hp_dali_sht();
		my_dali = new hp_dali(xml_main_node.getChildNode("dali"), &my_db_data,m_dali_sht );
	}
	if(!xml_main_node.getChildNode("cards").isEmpty()){
		my_cards = new hp_cards(xml_main_node.getChildNode("cards"));
	}
	if(!xml_main_node.getChildNode("modbus").isEmpty()){
		my2mdb_tx_data = new hp2mdb_data_t();
		my2mdb_rx_data = new hp2mdb_data_t();
		my2modbus = new hp2modbus(my2mdb_rx_data, my2mdb_tx_data, xml_main_node.getChildNode("modbus"));
	}
	for(int i=0; i<xml_main_node.getChildNode("rooms").nChildNode("room"); i++){  // spracovanie sluzieb
		string name = my_xml_parser.get_node_value(xml_main_node.getChildNode("rooms").getChildNode("room",i),"name");
		string label = my_xml_parser.get_node_value(xml_main_node.getChildNode("rooms").getChildNode("room",i),"label");
		my_rooms.insert(std::make_pair<string,string>(name.c_str(),label.c_str()));
	}
	
	return 0;
}

int hp_go::process_dn_change()
{
	//cout <<"Process dn change, mode: " << my_lightning->get_actual_mode() << endl;
	if(my_lightning != NULL){
		push_db_query("Process dn change, mode: " + patch::to_string(my_lightning->get_actual_mode()), DB_LOG_COM);
		for(vector<hp_virtual_pin *>::iterator it = my_pins.begin(); it != my_pins.end(); ++it){ 
			vector<bool> rules;
			 rules = my_lightning->get_rules4pin((*it)->get_desc(),my_lightning->get_actual_mode());
			if(rules.size() == 0){
				for(int i=0; i<3; i++){
					rules.push_back(false);
				}
			}
			if(my_shutters != NULL){
				if(my_shutters->has_rule4pin((*it)->get_desc())){  
					rules[0] = true;
					rules[1] = true;
					rules.push_back(true);
				} else {
					rules.push_back(false);
				}
			} else {
				rules.push_back(false);
			}
			if(rules.size() == 4){
				(*it)->setup_input_rules(rules[0], rules[1], rules[2], rules[3]);// pre jednotlive piny sa nastavuje to, ci maju aktivne pravidla typu: short, long, double_click
			}
		}
	}
	return 0;
}

int hp_go::create_driver_xml(XMLNode node)
{
	XMLNode write_node = XMLNode::createXMLTopNode("driver");

	write_node.addChild("xbee_socket").addAttribute("value", my_xbee_socket.c_str());
	write_node.addChild("cls_socket").addAttribute("value", my_cls_socket.c_str());
	write_node.addChild("log_verbose").addAttribute("value","1");//my_xml_parser.get_node_value(node,"log_verbose").c_str());
	write_node.addChild("ND_command").addAttribute("value", "0");
	write_node.addChild("AES_enabled").addAttribute("value","0");
	write_node.addChild("AES_key_time").addAttribute("value","0");
	write_node.addChild("AES_init").addAttribute("value","0");
	write_node.addChild("xbee_count").addAttribute("value",patch::to_string(my_hbxs.size()).c_str());

	for(unsigned int i=0; i<my_hbxs.size(); i++){
		write_node.addChild("xbee").addChild("id").addAttribute("value",patch::to_string(i+1).c_str());
		string mac = my_hbxs[i]->get_mac().c_str(), res_mac="";
		for(int j=0; j<(int)mac.length(); j++){
			res_mac.push_back(mac[j]);
			if((j+1)%2 == 0){
				res_mac.push_back(' ');
			}
		}
		res_mac.append("ff fe");

		write_node.getChildNode("xbee",i).addChild("mac").addAttribute("value", res_mac.c_str());
	}
	string name = my_xbee_socket.substr(0,my_xbee_socket.find("/hp_"));
	name += "/driver/driver.xml";

	write_node.writeToFile(name.c_str());

	return 0;
}

void hp_go::fill_shared_memory()
{
	//static int sh_counter;
	//cout <<"Fill shared called: " << ++my_test_counter<< " times " << endl;
	Json::FastWriter wr;
	//Json::StyledWriter wr;
	Json::Value json,statuses,doorman;

	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	std::stringstream buffer;
	buffer << std::put_time(timeinfo, "%d-%m-%Y %T");

	hp_shared_command_t cmd;
	cmd.ident = "start";
	cmd.command_type = SHARED_PUSH_ALL;
	cmd.memory_value = "";
	int s_counter = 0;
	for(auto it = my_pins.begin(); it != my_pins.end(); ++it){ 
		string function = (*it)->get_function();
		string value = "";
		value = (*it)->get_status();
		if(value == ""){
			value = "0";
		}
		if(function == "onoffButton" || function == "doorLock" ||
		   function == "gate" || function == "gateDoublePin" || function == "magKontakt") {
			doorman[doorman.size()]["ident"]=(*it)->get_desc();
			doorman[doorman.size()-1]["status"]=value;
		}
			
		if(function == "pushButton" || function == "multi_button" || 
		   function == "heater" || function == "cooler" || function == "sprinkler"|| 
		   function == "shutterOnOff" || function == "shutter"|| function == "temperature"){
			continue;
		}
		//int dist = std::distance(my_pins.begin(), it);
		statuses[s_counter]["ident"]=(*it)->get_desc();
		statuses[s_counter++]["status"]=value;
	}
	for(auto it:my_meteo_idents){
		cout <<"Adding: " << it.ident << "," << it.value << endl;
		statuses[s_counter]["ident"]=it.ident;
		statuses[s_counter++]["status"]=it.value;
	}
	if(my_lightning != NULL){
		if(my_lightning->get_dn_ident() != ""){
			string value;
			if(my_lightning->get_actual_mode() == "night"){
				value = "1";
			} else {
				value = "0";
			}
			statuses[statuses.size()]["ident"]=my_lightning->get_dn_ident();
			statuses[statuses.size()-1]["status"]=value;
		}
	}
	if(my_dali != NULL){
		Json::Value mdb = my_dali->get_shm_data();
		for(auto i:mdb){
			statuses[statuses.size()]["ident"]= i["ident"];
			statuses[statuses.size()-1]["status"]=i["status"];
		}
	}
	if(my_gates != NULL){
		for(int i=0; i<my_gates->get_gates_size(); i++){
			doorman[doorman.size()]["ident"]= my_gates->get_lock_ident(i);
			doorman[doorman.size()-1]["status"]=my_gates->get_lock(i);
		}
	}
	if(my2modbus != NULL){
		Json::Value mdb = my2modbus->get_shm_data();
		for(auto i:mdb){
			statuses[statuses.size()]["ident"]= i["ident"];
			statuses[statuses.size()-1]["status"]=i["status"];
		}
	}
	if(my_rekuperacia != NULL){
		json["rekuperacia"] = my_rekuperacia->get_actual_state();
	}
	if(my2heating != NULL){
		json["heating"] = my2heating->get_shm_data();
		statuses[statuses.size()]["ident"]= my2heating->get_eco_ident();
		statuses[statuses.size()-1]["status"]=patch::to_string(my2heating->get_eco_temp());
	} else {
		json["heating"] = "heatingGetDataFalse";
	}
	if(my_security != NULL){
		json["security"]= my_security->get_shm_data();
		statuses[statuses.size()]["ident"]= my_security->get_simulation_ident();
		statuses[statuses.size()-1]["status"]=my_security->get_simulation_value();
	} else if(my_jablotron != NULL){
		json["security"]= my_jablotron->get_shm_data();
		statuses[statuses.size()]["ident"]= my_jablotron->get_simulation_ident();
		statuses[statuses.size()-1]["status"]=my_jablotron->get_simulation_value();
	} else {
		json["security"]["error"] = "securityGetDataFalse";
	}
	if(my_watering != NULL){
		//json["watering"]= my_watering->get_shm_data();
		json["wateringV2"] = my_watering->get_shm_data(true);
	} else {
		json["watering"]["error"] = "wateringGetDataFalse";
	}
	json["statuses"]=statuses;
	json["doorman"]=doorman;
	json["refresh_time"] = buffer.str();
	json["err_state"] = "0";
	json["err_mess"] = "";
	if(my_shm != NULL){
		//cout << json << endl;
		//cout << "len: " << wr.write(json).length() << endl;
		if(!my_shm->push_shm_data(wr.write(json))){
			Json::Value err;
			err["refresh_time"] = buffer.str();
			err["err_state"] = "1";
			err["err_mess"] = "Presiahnutie shared memory";
			my_shm->push_shm_data(wr.write(err));
		}
	}
	my_shm_ws_data = "";
	my_shm_ws_data = wr.write(json);
}

void hp_go::print_axon_report(string file, string line, string mess)
{
#ifdef AXON_DEBUG
	ofstream fw;
	fw.open(my_axon_file.c_str(),std::ios::app);
	if(fw.is_open()){
		time_t rawtime;
		struct tm * timeinfo;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		std::stringstream buffer;
		buffer << std::put_time(timeinfo, "%d-%m-%Y %T");
		fw << buffer.str() <<": " << file << ":" << line << "-" << mess<< endl;
		fw.close();
	}
#endif
}

int hp_go::run() 
{

	std::thread th2(*m_listen_thread);
	std::thread th3(*my_db_handler);
	if(my_modbus_devices.size()){
		std::thread th4(*my_modbus);
		th4.detach();
		if(my2heating != NULL){
			my2heating->set_mdb_master_pos(my_modbus_devices);
		}
	}
	if(my2modbus != NULL){
		std::thread th5(*my2modbus);
		th5.detach();
	}
	std::thread th6(*my_timing);

	if(sync_with_db() == -1){ // synchronizovanie prvkov systemu s databazou, zatial zabezpecovacka
		return -1;
	}

	if(my_lightning != NULL){
		process_dn_change();
	}
	fill_shared_memory(); 

	time_t rawtime;
	struct tm * timeinfo;
	int finish = 0;

	if(my_modbus_devices.size()){
		my_mdb_tx_data->mtx.lock();
	//hp_modbus_data(string master_ip, int master_port, int start_address_dec, unsigned char function_code, int coils_num,unsigned char slave_address=1, unsigned char *data = NULL, int data_len=0);
	//	my_mdb_tx_data->my_data.push_back(hp_modbus_data("87.197.189.243",502,2033,3,1));
		my_mdb_tx_data->mtx.unlock();
	}

	if(my_notification != NULL){
		my_notification->push_notification(NN_START,"Štart systému.");
	}

	my_sec_counter = 0;
	my_test_counter =0;

	hp_wss_server *my_ws = new hp_wss_server(my_data_provider);
	my_ws->init();
	thread server_thread([my_ws]() {
		my_ws->start_ws();
	});

	hp_wss_client *my_ws_client = new hp_wss_client(my_data_provider, my_wss_ident,false);
	my_ws_client->init();

	std::vector<std::thread> vec_clients;

	vec_clients.push_back(thread([my_ws_client] () {
		my_ws_client->start_ws();
	}));
	

	if(!my_simulator){
		push_delayed_event(prepare_event(EVENT_U_MESSAGE,4,"ucko"));
		if(my_shutters != NULL){
			if(my_shutters->has_direct_control()){
				push_delayed_event(prepare_event(EVENT_SETUP_BLIND_TIME,my_hbxs.size()+8,"Nastavujem cas pre zaluzie"));
			}
		}
	}
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	push_heating_state();
	if(my_dali != NULL){
		//my_dali->read_startup_values();
	}

	my_sec_counter = 0;
	finish = 0;
	push_db_statuses(true);
	cout <<"Up and running" << endl;
	int cc_counter = 0;
	while(1){
		cc_counter++;
		while(m_incomming->mess.size() >0 && finish == 0){
			// process response from driver
			if(m_incomming->mess[0].find("GUI") != std::string::npos){
				if(m_incomming->mess[0].find("drv_rec") == std::string::npos){
					finish = process_gui_mess(m_incomming->mess[0].c_str());   // spracovanie GUI spravy
					if(finish == -1){
						break;
					}
				} else {
					process_driver_resp(m_incomming->mess[0].substr(7));
				}
			} else {
				process_driver_resp(m_incomming->mess[0]);
			}
			m_incomming->mtx_incomming.lock();
			m_incomming->mess.erase(m_incomming->mess.begin());
			m_incomming->mtx_incomming.unlock();
		}
		if(finish == -1){
			break;
		}
		if(my2heating != NULL && my_modbus_devices.size()){
			while(this->my_mdb_rx_data->my_data.size() > 0){
				my2heating->process_mdb_response(my_mdb_rx_data);
			}
		}
		if(my_dali != NULL){
			my_dali->process_dali_response();
			fill_shared_memory();
		}		
		my_sender->send_messages(my_hbxs,my_send_messages);

		check_sent_messages();
		check_running_rules();
		check_repead_mess();
		check_modbus_mess();
		if(my2heating != NULL){
			Json::Value tmp_js = my2heating->get_ws_data();
			if(!tmp_js.isNull() && tmp_js.size() > 0){
				for(auto i_js : tmp_js){
					my_ws_data[my_ws_data.size()] = i_js;
				}
				my2heating->clear_ws_data();
			}
		}
		read_dali_sht();
		my_ws->push_mess(my_ws_data);
		my_ws_client->push_mess(my_ws_data);
		my_ws_data.clear();
		if(my_timing_data.new_sec){
	//		cout <<"CC counter: " << cc_counter << endl;
			cc_counter =0;
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
			if(my_ws_client != nullptr){
				if(timeinfo->tm_sec%15 == 0){ ///CHECKing ping message from server
					my_ws_client->check_heartbeat();
				}
	//			cout <<"My wss connect coutner: " << my_wss_connect_counter << ", isrunning: " << my_ws_client->is_running() << endl;
				if(!my_ws_client->check_active()){
					if (time(NULL) > wss_client_connect_time + wss_client_connect_delay) {
						wss_client_connect_time = time(NULL);
						wss_client_connect_delay = wss_client_connect_delay * 2;
						if (wss_client_connect_delay > MAX_RECCONECT_DELAY) {
							wss_client_connect_delay = MAX_RECCONECT_DELAY;
						}
						if(vec_clients.size() > 0){
							my_ws_client->stop_ws();
							vec_clients[0].join();
						}
						vec_clients.clear();
						vec_clients.push_back(thread([my_ws_client] () {
							my_ws_client->start_ws();
						}));
					} else {
						wss_client_connect_delay = 2;
					}
				}
					/*
				if(!my_ws_client->check_active()){
					if(my_wss_connect_counter < 2){
						if(vec_clients.size() > 0){
							my_ws_client->stop_ws();
							vec_clients[0].join();
						}
						vec_clients.clear();
						vec_clients.push_back(thread([my_ws_client] () {
							my_ws_client->start_ws();
						}));
					} else if(my_wss_connect_counter >= my_wss_try_reconnect_in){
						my_wss_connect_counter = -1;
					}
					my_wss_connect_counter++;
				} else {
					my_wss_connect_counter = 0;
				}
				*/
			}
			my_timing_data.mtx.lock();
			my_timing_data.new_sec = 0;
			my_timing_data.mtx.unlock();
			process_delayed_events();
			if(my_impulz_counter != NULL){
				my_impulz_counter->check_off_interval();
			}

			if(timeinfo->tm_min != my_last_min){
				//print_axon_report(__FILE__, patch::to_string(__LINE__));
				my_last_min = timeinfo->tm_min;
				if(my_last_min%5 == 0){
					if(my2heating != nullptr){
						my2heating->update_mdb_data(my_mdb_tx_data, my_modbus_devices);
					}
					push_db_statuses();
				}
				if(my_impulz_counter != NULL){
					//my_impulz_counter->print_var();
					my_impulz_counter->process_data();
				}
				check_alarm_mess(timeinfo->tm_hour, timeinfo->tm_min);
				check_heating(timeinfo->tm_min);
				check_watering(timeinfo->tm_hour, timeinfo->tm_min);
				if(my_lightning != NULL){
					std::pair<string, string> res = my_lightning->check_dn_change();
					if(res.first != ""){
						push_db_query("UPDATE STATUSES set STATUS = '"+res.second+"' where ITEM = '"+res.first+"'"); 
						process_dn_change();
					}
				}
				if(timeinfo->tm_min == 0){
					calculate_consumption(timeinfo);
					//push_db_query("Calculate consumption took: " +patch::to_string(msec) + " us", DB_LOG_COM);
					get_trusted_devices();
				}
				if(my_turnikets != NULL){
					vector<string> tq = my_turnikets->erase_old_cards();
					for(unsigned int i=0; i<tq.size(); i++){
						push_db_query(tq[i],DB_QUERY);
					}
				}
				if(my_last_hour != timeinfo->tm_hour){
					my_last_hour = timeinfo->tm_hour;
					push_system_state(timeinfo);
					push_heating_state();
				}
				if(my_notification != NULL){
					my_notification->push_notification(NN_CHECK,"Alive packet");
				}
				//finish = check_force_restart();
			}
			my_sec_counter++;
			if(my_sec_counter > DIAGNOSTIC_INTERVAL){
				send_diagnostic();
				my_sec_counter = 0;
			}
			process_running_zone();
			check_hbx_times();
			check_periodic_pins();
			if(my_notification != NULL){
				my_notification->check_periodic_guard(my_pins);
			}
#ifdef SIMULATION
			if(my_simulation_state == SIM_CALCULATION){
				if(my_sim_data->finished){
					my_simulation_control_counter=0;
					my_sim_data->finished = false;
					my_sim_thread->join();						
					if(my_sim_thread != NULL){
						delete my_sim_thread;
						my_sim_thread = NULL;
					}
					process_sim_data();
				}
			}
#endif
			my_data_provider->set_shm_data(my_shm_ws_data);
		}
		usleep(DELAY_TIME-70);
	}
	/// shut down thread for db
	
	push_db_statuses();

	stop_thread();
	th2.join();
	th3.join();
	th6.join();
	if(my_ws != nullptr){
		my_ws->stop_ws();
	}
	if(my_ws_client != nullptr){
		my_ws_client->stop_ws();
	}
	server_thread.join();
	if(vec_clients.size() > 0){
		vec_clients[0].join();
	}

	delete_objects();
	if(my_ws != nullptr){
		delete my_ws;
	}
	if(my_ws_client != nullptr){
		delete my_ws_client;
	}

	return 0;
}
void hp_go::stop_thread()
{
	push_db_query("stop_thread",DB_STOP); // posli prikaz pre ukoncenie log threadu

	my_timing_data.stop_thread = true;
	hp_client_socket client_socket(AF_UNIX,SOCK_STREAM, 0, my_cls_socket); /// posli prikaze pre ukoncenie listen thread
	client_socket.send_socket("exit____");
}
	
bool hp_go::has_delayed_event(int pin_pos)
{
	for(unsigned int i=0; i<my_delayed_events.size(); i++){
		if(my_delayed_events[i].event_type == EVENT_OFF_PIN){
			if(pin_pos == patch::string2int(my_delayed_events[i].var)){
				return true;
			}
		}
	}

	return false;
}

int hp_go::push_db_statuses(bool first_time)
{
	if(my_has_hpp_web){
		vector<string> queries;
		for(unsigned int i=0; i<my_pins.size(); i++){
			string tmp = my_pins[i]->get_update_string();
			if(tmp != ""){
				queries.push_back(tmp);
			}
		}
		if(my2heating != NULL){
			queries.push_back("UPDATE STATUSES set STATUS = '"+patch::to_string(my2heating->get_def_tempering())+"' where ITEM = 'defTemperingValue'");
			queries.push_back("UPDATE STATUSES set STATUS = '"+patch::to_string(my2heating->get_def_temp())+"' where ITEM = 'defTemperature'");
		}
		if(queries.size() > 0 && !first_time){
			queries.insert(queries.begin(), "START TRANSACTION");
			queries.push_back("COMMIT");
		//	cout << queries << endl;
			for(auto it = queries.begin(); it != queries.end(); ++it){
				push_db_query(*it,DB_TRANSACTION);
			}
		}
	}
	return 0;
}

void hp_go::process_delayed_events_itr()
{
	for(auto itr = my_delayed_events.begin(); itr != my_delayed_events.end();){
		if(itr->run_time < time(NULL)){
			push_db_query("Delayed event type: " + patch::to_string(itr->event_type) + " run after: " + patch::to_string(itr->run_time) + " event var: " + itr->var,DB_LOG_COM);
			if(itr->event_type == EVENT_UPDATE_ALARMS){
				process_alarm(NULL,true);
			}
			if(itr->event_type == EVENT_MESS_HEATING){
				send2temp_messages(itr->var);
				check_heating();
			}
			if(itr->event_type == EVENT_OFF_PIN){
				int pin_pos = patch::string2int(itr->var);
				if(pin_pos >= 0 && pin_pos < (int)my_pins.size()){
					my_pins[pin_pos]->set_status("0");
					push_db_query("UPDATE STATUSES set STATUS = 0 where ITEM = '" + my_pins[pin_pos]->get_desc() + "'");
				}
			}
			if(itr->event_type == EVENT_SETUP_BLIND_TIME){
				setup_blinds_time();
			}
			if(itr->event_type == EVENT_U_MESSAGE){
				send_U_messages();
			}
			if(itr->event_type == EVENT_SHUTTERS){
				if(my_shutters != NULL){
					process_gui_shutt_mess(itr->var);
				}
			}
			if(itr->event_type == EVENT_HEATING_CHECK){
				if(my2heating != NULL){
					my2heating->set_all_query_temp(patch::string2float(itr->var));
					check_heating_zones();
					check_heating();
				}
			}
			if(itr->event_type == EVENT_HEATING_TEMPERING){
				int pin_pos = find_item(itr->var,"desc");
				if(pin_pos != -1){
					if(my2heating != NULL){
						if(my2heating->deactivate_tempering(itr->var)){
							this->add_message(pin_pos, "0");
							push_db_query("Vypinam temperovanie pre: " + itr->var, DB_LOG_COM);
						}
					}
				}
			}
			if(itr->event_type == EVENT_JABL_LOCK_ZONE){
				int pin_pos = this->find_item(itr->var, "desc");
				if(pin_pos != -1){
					push_db_query("UPDATE STATUSES set STATUS = 0 where ITEM = '" + my_pins[pin_pos]->get_desc() + "'");
					this->add_message(pin_pos, "0");
					my_pins[pin_pos]->set_status("0");
				}
			}
			if(itr->event_type == EVENT_WAT_ALL_ZONE){
				if(my_watering != NULL){
					string wat_ident = itr->var.substr(0,itr->var.find(";"));
					hp_watering_zone *zone = my_watering->find_wat_zone(wat_ident);
					int ev_delay=0;
					if(zone != NULL){
						vector<string> out = zone->get_actors();
						int zone_timer = zone->get_time_period();
						if(zone_timer != 0){
							ev_delay = 60*zone_timer;
							for(unsigned int i=0; i<out.size(); i++){
								int pin_pos = find_item(out[i], "desc");
								if(pin_pos != -1){
									this->add_message(pin_pos, "1", false, 0, zone_timer*60, false);
								}
							}
						}
					}
					wat_ident = itr->var.substr(itr->var.find(";")+1);
					if(wat_ident != ""){
						push_delayed_event(prepare_event(EVENT_WAT_ALL_ZONE,ev_delay+5,wat_ident));
					} else {
						push_db_query("UPDATE watering set running = 0 where ident = '"+my_watering->get_all_ident()+"';",DB_QUERY);
						this->push_ws_data("watering", my_watering->get_all_ident(), "0");
					}
				}
			}
			if(itr->event_type == EVENT_JABL_SEC_COUNTDOWN){
				push_db_query(itr->var);
			}
			if(itr->event_type == EVENT_TIMER_CLOCK_MESS){
				this->add_message(patch::string2int(itr->var), "0");
			}
			itr = my_delayed_events.erase(itr);
/*
			if(i != my_delayed_events.size() -1){
				my_delayed_events.erase(my_delayed_events.begin()+i);
			} else {
				my_delayed_events.erase(my_delayed_events.begin()+i);
				break;
			}
			*/
		} else {
			itr++;
		}
	}
}

void hp_go::process_delayed_events()
{
	for(unsigned int i=0; i<my_delayed_events.size(); i++){
		if(my_delayed_events[i].run_time < time(NULL)){
			push_db_query("Delayed event type: " + patch::to_string(my_delayed_events[i].event_type) + " run after: " + patch::to_string(my_delayed_events[i].run_time) + " event var: " + my_delayed_events[i].var,DB_LOG_COM);
			if(my_delayed_events[i].event_type == EVENT_UPDATE_ALARMS){
				process_alarm(NULL,true);
			}
			if(my_delayed_events[i].event_type == EVENT_MESS_HEATING){
				send2temp_messages(my_delayed_events[i].var);
				check_heating();
			}
			if(my_delayed_events[i].event_type == EVENT_OFF_PIN){
				int pin_pos = patch::string2int(my_delayed_events[i].var);
				if(pin_pos >= 0 && pin_pos < (int)my_pins.size()){
					my_pins[pin_pos]->set_status("0");
					push_db_query("UPDATE STATUSES set STATUS = 0 where ITEM = '" + my_pins[pin_pos]->get_desc() + "'");
					this->push_ws_data("delay_event", my_pins[pin_pos]->get_gui_desc(), "0");
					if(my_gates != NULL){
						process_gate(pin_pos, "0");
					}
				}
			}
			if(my_delayed_events[i].event_type == EVENT_SETUP_BLIND_TIME){
				setup_blinds_time();
			}
			if(my_delayed_events[i].event_type == EVENT_U_MESSAGE){
				send_U_messages();
			}
			if(my_delayed_events[i].event_type == EVENT_SHUTTERS){
				if(my_shutters != NULL){
					process_gui_shutt_mess(my_delayed_events[i].var);
				}
			}
			if(my_delayed_events[i].event_type == EVENT_HEATING_CHECK){
				if(my2heating != NULL){
					my2heating->set_all_query_temp(patch::string2float(my_delayed_events[i].var));
					check_heating_zones();
					check_heating();
				}
			}
			if(my_delayed_events[i].event_type == EVENT_HEATING_TEMPERING){
				int pin_pos = find_item(my_delayed_events[i].var,"desc");
				if(pin_pos != -1){
					if(my2heating != NULL){
						if(my2heating->deactivate_tempering(my_delayed_events[i].var)){
							this->add_message(pin_pos, "0");
							push_db_query("Vypinam temperovanie pre: " + my_delayed_events[i].var, DB_LOG_COM);
						}
					}
				}
			}
			if(my_delayed_events[i].event_type == EVENT_JABL_LOCK_ZONE){
				int pin_pos = this->find_item(my_delayed_events[i].var, "desc");
				if(pin_pos != -1){
					push_db_query("UPDATE STATUSES set STATUS = 0 where ITEM = '" + my_pins[pin_pos]->get_desc() + "'");
					this->add_message(pin_pos, "0");
					my_pins[pin_pos]->set_status("0");
				}
			}
			if(my_delayed_events[i].event_type == EVENT_WAT_ALL_ZONE){
				delayed_event_t de = my_delayed_events[i];
				if(my_watering != NULL){
					string wat_ident = de.var.substr(0,de.var.find(";"));
					hp_watering_zone *zone = my_watering->find_wat_zone(wat_ident);
					int ev_delay=0;
					if(zone != NULL){
						vector<string> out = zone->get_actors();
						int zone_timer = zone->get_time_period();
						if(zone_timer != 0){
							ev_delay = 60*zone_timer;
							for(unsigned int i=0; i<out.size(); i++){
								int pin_pos = find_item(out[i], "desc");
								if(pin_pos != -1){
									this->add_message(pin_pos, "1", false, 0, zone_timer*60, false);
								}
							}
						}
					}
					wat_ident = de.var.substr(de.var.find(";")+1);
					if(wat_ident != ""){
						push_delayed_event(prepare_event(EVENT_WAT_ALL_ZONE,ev_delay+5,wat_ident));
					} else {
						push_db_query("UPDATE watering set running = 0 where ident = '"+my_watering->get_all_ident()+"';",DB_QUERY);
						this->push_ws_data("watering", my_watering->get_all_ident(), "0");
					}
				}
			}
			if(my_delayed_events[i].event_type == EVENT_JABL_SEC_COUNTDOWN){
				push_db_query(my_delayed_events[i].var);
			}
			if(my_delayed_events[i].event_type == EVENT_TIMER_CLOCK_MESS){
				this->add_message(patch::string2int(my_delayed_events[i].var), "0");
				int pos = patch::string2int(my_delayed_events[i].var);
				if(pos >= 0 && pos < (int)my_pins.size()){
					find_out_rule(my_pins[pos]->get_desc(), pos, "0");
				}
			}

			if(i != my_delayed_events.size() -1){
				my_delayed_events.erase(my_delayed_events.begin()+i);
			} else {
				my_delayed_events.erase(my_delayed_events.begin()+i);
				break;
			}
		}
	}
}

void hp_go::push_delayed_event(delayed_event_t event)
{
	for(unsigned int i=0; i<my_delayed_events.size(); i++){
		//cout <<"!!!Delayed event: " << i << " var: " << my_delayed_events[i].var << " type; " << my_delayed_events[i].event_type<< endl;
	}
	bool add_event = true;
	if(event.event_type == EVENT_MESS_HEATING || event.event_type == EVENT_TIMER_CLOCK_MESS){
		for(unsigned int i=0; i<my_delayed_events.size(); i++){
			if(event.var == my_delayed_events[i].var){
				my_delayed_events[i].run_time = event.run_time;
				add_event = false;
				break;
			}
		}
	}
	if(event.event_type == EVENT_HEATING_CHECK){
		for(unsigned int i=0; i<my_delayed_events.size(); i++){
			if(my_delayed_events[i].event_type == event.event_type){
				my_delayed_events[i].var = event.var;
				add_event = false;
				break;
			}
		}
	}

	if(add_event){
		push_db_query("###################New delayed event: " + patch::to_string(event.var) + ", " + patch::to_string(event.event_type),DB_LOG_COM);
		my_delayed_events.push_back(event);
	}
}

void hp_go::push_heating_state()
{
	if(my2heating != NULL){
		my2heating->push_zones_states();
	}
}

void hp_go::push_system_state(struct tm * info)
{
	stringstream res,query;;
	res << "\nStav systemu v case: " << info->tm_hour << ":"<<info->tm_min<<":"<<info->tm_sec << "\n";
	res << "Sent messages count: " << my_send_messages.size() << endl;
	if(my_send_messages.size() > 0){
		for(unsigned int i=0; i<my_send_messages.size(); i++){
			res << "Mess: " << my_send_messages[i].mess << ", " << my_send_messages[i].mess_type << ", " << my_send_messages[i].xbee_id << ", frame_id: " << my_send_messages[i].frame_id << endl;
		}
	}
	vector<int> id = my_sender->get_non_free();
	res << "Non free id: " << id.size()-1 << endl;
	if(id.size() > 1){
		unsigned int next = 0;
		if(id[0] == -1){
			res << "Non free xbee id: ";
			for(unsigned int i=0; i<id.size(); i++){
				res << id[i] << ",";
			}
		} else {
			res << "Non free uc id: ";
			for(unsigned int i=0; i<id.size(); i++){
				if(id[i] == -1){
					next = i;
					break;
				}
				res << id[i] << ",";
			}
			next++;
			if(next < id.size()-1){
				res << "\nNon free xbee id: ";
				for(unsigned int i=next; i<id.size(); i++){
					res << id[i] << ",";
				}
			}
		}
		res << endl;
	}
	push_db_query(res.str(), DB_LOG_COM);
}

int hp_go::set_auto_temp()
{
	string query;
	if(my2heating->get_heating_mode() == HEATING_AUTO){
		float auto_temp= 20;
		MYSQL *conn;
		MYSQL_RES *result;
		MYSQL_ROW row;
		int query_state;
		time_t rawtime;
		struct tm * timeinfo;

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		conn = mysql_init(NULL);
		if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			mysql_close(conn);
#ifndef AXON_SERVER
			mysql_library_end();
#endif
			conn = NULL;
			return -1;
		} else {
			string date_pos;
			if(timeinfo->tm_wday == 0){
				date_pos = "7";
			} else {
				date_pos = patch::to_string(timeinfo->tm_wday);
			}
			query = "select temp from termostat where day = "+date_pos+" AND from_time <= "+patch::to_string(timeinfo->tm_hour*60+timeinfo->tm_min)+" && to_time > "+patch::to_string(timeinfo->tm_hour*60+timeinfo->tm_min)+";";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						auto_temp = patch::string2float(string(row[0]));
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
#ifndef AXON_SERVER
				mysql_library_end();
#endif
				conn = NULL;
			}
		}

		if(my2heating->get_auto_query_temp() != auto_temp){
			my2heating->set_auto_query_temp(auto_temp);
			push_db_query("Nastavujem pozadovanu teplotu auto rezimu na: " + patch::to_string(auto_temp), DB_LOG_COM);
			check_heating_zones(HEATING_AUTO);
		}
	}
	
	return 0;
}

void hp_go::check_heating(int min)
{
	if(my2heating != NULL){
		set_auto_temp();
		vector<hp_heating_data_t> data = my2heating->check_heating(min);
		for(unsigned int i=0; i<data.size(); i++){
			int pin_pos = find_item(data[i].ident,"desc");
			if(pin_pos != -1){
				this->add_message(pin_pos, data[i].value, false);
				if(data[i].off_delay != 0){
					push_delayed_event(prepare_event(EVENT_HEATING_TEMPERING, data[i].off_delay*60, my_pins[pin_pos]->get_desc()));
					push_db_query("Zapinam temperovanie pre: " + my_pins[pin_pos]->get_desc(), DB_LOG_COM);
				}
			}
		}
		my2heating->update_mdb_data(my_mdb_tx_data,my_modbus_devices);
	}
}

void hp_go::check_hbx_times()
{
	for(unsigned int i=0; i<my_hbxs.size(); i++){
		string res = my_hbxs[i]->check_hbx_timer();
		if(res != ""){
			push_db_query(res, DB_LOG_COM);
		}
		if(my_notification != NULL){
			if(my_notification->get_hbx_check_time() != 0){
				res = my_hbxs[i]->check_internal_timer(my_notification->get_hbx_check_time());
				if(res != ""){
					push_db_query(res, DB_LOG_COM);
					my_notification->push_notification(NN_ERROR,res);
				}
			}
		}
	}
}

void hp_go::calculate_consumption(struct tm * cas)
{
	if(my_has_electro_cons){
		for(unsigned int i=0; i<my_pins.size(); i++){
			int type = my_pins[i]->get_int_type();
			if(type == PIN_ELECTRO_CONS || type == PIN_GAS_CONS){
				string cons_db_type = "";
				if(type == PIN_ELECTRO_CONS){
					cons_db_type = "1";
				}
				if(type == PIN_GAS_CONS){
					cons_db_type = "2";
				}

				push_db_query("INSERT INTO CONSUMPTION_DATA (`TYPE`, `VALUE`, `FLAG`, `DATE`) VALUES ('"+cons_db_type+"',(select value FROM (select sum(VALUE) as value from CONSUMPTION_DATA where TYPE = '"+cons_db_type+"' AND FLAG = 0 AND DATE < NOW() and DATE > DATE_ADD(NOW(), INTERVAL -1 HOUR)) as bala),'4',DATE_ADD(NOW(), INTERVAL -1 HOUR))",DB_QUERY); // denna spotreba
				if(cas->tm_hour == 20){
				}
				if(cas->tm_hour == 0){
					push_db_query("INSERT INTO CONSUMPTION_DATA (`TYPE`, `VALUE`, `FLAG`, `DATE`) VALUES ('"+cons_db_type+"',(select value FROM (select sum(VALUE) as value from CONSUMPTION_DATA where TYPE = '"+cons_db_type+"' AND FLAG = 4 AND DATE < NOW() and DATE > DATE_ADD(NOW(), INTERVAL -1 DAY)) as bala),'3',DATE_ADD(NOW(), INTERVAL -1 DAY))",DB_QUERY); // denna spotreba
					if(cas->tm_mday == 1){
						push_db_query("INSERT INTO CONSUMPTION_DATA (`TYPE`, `VALUE`, `FLAG`, `DATE`) VALUES ('"+cons_db_type+"',(select value FROM (select sum(VALUE) as value from CONSUMPTION_DATA where TYPE = '"+cons_db_type+"' AND FLAG = 3 AND DATE < NOW() and DATE > DATE_ADD(NOW(), INTERVAL -1 MONTH)) as bala),'5',DATE_ADD(NOW(), INTERVAL -1 MONTH))",DB_QUERY); // denna spotreba
					}
				}
			}
		}
	}

}

int hp_go::process_running_zone()
{
	string query;
	bool check_all_ident = false;
	for(unsigned i=0; i<my_running_zone.size(); i++){
		bool erase_zone = false;
		int state = my_running_zone[i]->decrease_counter();
		if(state == 0){
			check_all_ident = true;
			if(my_running_zone[i]->get_sec_status() == 0){
				if(my_notification != NULL){
					try {
						//cout << "at: " << my_running_zone[i]->get_id() << endl;
						if(my_rooms.find(my_running_zone[i]->get_id()) != my_rooms.end()){
							my_notification->push_security_notification(my_rooms.at(my_running_zone[i]->get_id()),1);
						}
					} catch (const std::exception& e) {
						push_db_query("A standard exception was caught, with message '" + string(e.what()) + ", room: "+ my_running_zone[i]->get_id()+"', file: " + __FILE__ +", line: " + patch::to_string(__LINE__), DB_LOG_COM);
					}
				}
				my_running_zone[i]->set_sec_status(1);
				query = "UPDATE SECURITY_ZONES SET ALARM_COUNTDOWN = -1, STATUS = 1 where ID = '"+my_running_zone[i]->get_id()+"'";
				this->push_ws_data("security",my_running_zone[i]->get_id(),"1");
				this->push_ws_data("securityCountDown",my_running_zone[i]->get_id(),"-1");
				start_simulation(1);
				erase_zone = true;
			} else {
				if(my_running_zone[i]->get_sec_status() != 2){
					try {
						if(my_rooms.find(my_running_zone[i]->get_id()) != my_rooms.end() && my_notification != NULL){
							my_notification->push_security_notification(my_rooms.at(my_running_zone[i]->get_id()),2);
						}
					} catch (const std::exception& e) {
						push_db_query("A standard exception was caught, with message '" + string(e.what()) + "', file: " + __FILE__ +", line: " + patch::to_string(__LINE__), DB_LOG_COM);
					}
					my_running_zone[i]->set_sec_status(2);
					vector<string> outputs = my_running_zone[i]->get_actors();
					for(vector<string>::iterator it =outputs.begin(); it != outputs.end(); ++it){
						int item_pos = find_item(*it, "desc");
						if(item_pos != -1){
							this->add_message(item_pos, "1");
						} else {
							push_db_query("Nenasiel som aktora pre zonu: " +my_running_zone[i]->get_id() + ", " +LOG_CH(), DB_LOG_COM);
						}
					}
					erase_zone = true;
					query = "UPDATE SECURITY_ZONES SET ALARM_COUNTDOWN = -1, STATUS = 2 where ID = '"+my_running_zone[i]->get_id()+"'";
					this->push_ws_data("security",my_running_zone[i]->get_id(),"2");
					this->push_ws_data("securityCountDown",my_running_zone[i]->get_id(),"-1");
				}
			}
		} else {
			query = "UPDATE SECURITY_ZONES SET ALARM_COUNTDOWN = "+patch::to_string(state)+" where ID = '"+my_running_zone[i]->get_id()+"'";
			this->push_ws_data("securityCountDown",my_running_zone[i]->get_id(),patch::to_string(state));
		}
		fill_shared_memory();
		if(erase_zone){
			push_db_query(query,DB_QUERY);
			if(i != my_running_zone.size() -1){
				my_running_zone.erase(my_running_zone.begin() +i);
			} else {
				my_running_zone.erase(my_running_zone.begin() +i);
				break;
			}
		} else {
			push_db_query(query);
		}
	}
	if(check_all_ident > 0){
		int all_zone_status = my_security->get_all_zone_status();
		check_sec_singnalization();
		push_db_query("UPDATE SECURITY_ZONES SET STATUS = "+patch::to_string(all_zone_status)+" where ID = '"+my_security->get_all_ident()+"'", DB_QUERY);
		this->push_ws_data("security",my_security->get_all_ident(),patch::to_string(all_zone_status));
	}
	///!!!
	return 0;
}
void hp_go::check_sec_singnalization()
{
	if(my_security->has_signalization()){
		int pin_pos = find_item(my_security->get_signalization_ident(),"desc");
		if(pin_pos != -1){
			this->add_message(pin_pos,my_security->check_signalization(), false);
		}
	}
}

void hp_go::check_watering(int hour, int min)
{
	if(my_watering != NULL){
		vector<hp_wat_data_t> outputs = my_watering->check_start(hour*60+min);
		if(outputs.size() > 0){
			if(outputs.size() == 1 && outputs[0].query == my_watering->get_all_ident() ){
				vector<string> zones = my_watering->get_zones_ident();
				string ev_var = "";
				for(uint16_t i=0; i<zones.size(); i++){
					ev_var += zones[i] + ";";
				}
				if(ev_var != ""){
					push_delayed_event(prepare_event(EVENT_WAT_ALL_ZONE,0,ev_var));
				}
				push_db_query("UPDATE watering set running = 1 where ident = '"+my_watering->get_all_ident()+"';",DB_QUERY);
				this->push_ws_data("watering", my_watering->get_all_ident(), "1");
			} else {
				for(unsigned int i=0; i < outputs.size(); i++){
					push_db_query(outputs[i].query,DB_QUERY);
					for(map<string,int>::iterator it = outputs[i].out.begin(); it != outputs[i].out.end(); ++it){
						int pin_pos = find_item(it->first, "desc");
						if(pin_pos != -1 && it->second != 0){
							this->add_message(pin_pos, "1",true,0, it->second*60);
						}
					}
				}
			}
		}
	}
}

void hp_go::check_alarm_mess(int hour, int min)
{
	int shutt_counter = 0;
	for(unsigned int i=0; i<my_alarm_checker.size(); i++){
		if(my_alarm_checker[i].hour == hour){
			if(my_alarm_checker[i].min == min){
				//cout << my_alarm_checker[i]
				if(my_alarm_checker[i].pin_pos == -1){
					process_gui_scen_mess(my_alarm_checker[i].scen_command);
				} else {
					if(my_pins[my_alarm_checker[i].pin_pos]->get_function() == "shutter"){
						string gui_mess = "shutt_"+my_pins[my_alarm_checker[i].pin_pos]->get_desc()+"_";
						string send_value = my_alarm_checker[i].send_value;
						if(send_value == "2"){
							gui_mess += "up_7";
						} else if(send_value == "3"){
							gui_mess += "down_7";
						} else if(send_value == "4"){
							gui_mess += "allup_1";
						} else if(send_value == "5"){
							gui_mess += "alldown_1";
						}
						push_delayed_event(prepare_event(EVENT_SHUTTERS, shutt_counter, gui_mess));
						shutt_counter++;
					} else {
						this->add_message(my_alarm_checker[i].pin_pos,my_alarm_checker[i].send_value);
					}
				}
			}
		}
	}
#ifdef SIMULATION
	if(hour == 0 && min == 0){
		if(my_simulation_state == SIM_ON){
			vector<string>send_ident;
			for(unsigned int i=my_simulation_control_counter; i<my_sim_control_data.size(); i++){
				bool send_mess = true;
				for(unsigned int j=0; j<send_ident.size(); j++){
					if(send_ident[j] == my_sim_control_data[i].id){
						send_mess = false;
					}
				}
				if(send_mess){
					send_ident.push_back(my_sim_control_data[i].id);
					this->add_message(my_sim_control_data[i].pin_pos,"0",false,40*send_ident.size());
				}
			}
			start_simulation(1);
		}
	}
	if(my_simulation_state == SIM_ON){
		int last_random = 0;
		for(unsigned int i=my_simulation_control_counter; i<my_sim_control_data.size(); i++){
			if(my_sim_control_data[i].hour == hour){
				if(my_sim_control_data[i].min == min){
					int random_element = rand()%30;
					if(i>0){
						if(my_sim_control_data[i-1].min == min){ //&& my_sim_control_data[i-1].id == my_sim_control_data[i].id){
							random_element = last_random + 5;
							if(random_element > 59){
								random_element = 59;
							}
						}
					}
					this->add_message(my_sim_control_data[i].pin_pos,my_sim_control_data[i].send_value,true, random_element*100, false,false, false, true);
					my_simulation_control_counter = i;
					last_random = random_element;
				}
			}
		}
	}
#endif
}
void hp_go::check_modbus_mess()
{
	if(my2modbus != NULL){
		while(my2mdb_rx_data->my_data.size() > 0){
			my2mdb_rx_data->mtx.lock();				
			hp2modbus_data t_data = my2mdb_rx_data->my_data[0];
			push_db_query("UPDATE STATUSES set MODIFICATION = NOW(), STATUS = '"+t_data.get_value()+"' where ITEM = '"+t_data.get_desc()+"'"); 
		//	push_db_query("Modbus mess: " + t_data.get_desc() + ": " + t_data.get_value() +", status: " + t_data.get_status(), DB_LOG_COM);
			if(t_data.get_status() == "OK"){
				if(my2heating != NULL){
					vector<hp_heating_data_t> data = my2heating->process_section_temp(t_data.get_desc(),t_data.get_value(),my_notification);
					for(unsigned int i=0; i<data.size(); i++){
						int pin_pos = find_item(data[i].ident,"desc");
						if(pin_pos != -1){
							this->add_message(pin_pos, data[i].value,false,0, false, false, false, FORCE_HEATING_MESS);
						} else {
							//!!!search in modbus devices
						}
					}
					fill_shared_memory();
				}
			}
		//	my2mdb_rx_data->my_data[0].print_data();
			my2mdb_rx_data->my_data.erase(my2mdb_rx_data->my_data.begin());
			my2mdb_rx_data->mtx.unlock();				
		}
	}
}

void hp_go::check_periodic_pins()
{
	if(!my_simulator){
		for(uint16_t i=0; i<my_periodic_pins.size(); i++){
			if(my_pins[my_periodic_pins[i]]->check_periodicity()){
				hp_mess2send_t mess = my_pins[my_periodic_pins[i]]->create_send_mess("");
				my_sender->add_message(mess,0,0,i*100);
			}
		}
	}
}
void hp_go::check_repead_mess()
{
	unsigned int i;
	for(i=0; i<my_repeat_mess.size(); i++){
		bool check_mess = true;
		for(vector<hp_send_messages_t>::iterator it = my_send_messages.begin(); it != my_send_messages.end(); ++it){ // kontrola toho, ci na predchadzajucu repeat mesage prisla odpoved, az potom ako prisla odpoved, sa spusti odpocitavanie pre spravu
			if(it->xbee_id == my_repeat_mess[i].get_xbee_id() && it->mess.find(my_repeat_mess[i].get_mess()) != std::string::npos ){
				check_mess = false;
			}
		}
		if(check_mess){
			if(my_repeat_mess[i].check_periodicity()){
				if(my_repeat_mess[i].get_service_type() == "repeat_mess"){
					if(my_repeat_mess[i].check_life_token()){	
						this->add_message(my_repeat_mess[i].get_pin_pos(), "1", true,0,0,false,false);
					} else {
						if(i != my_repeat_mess.size() -1){
							my_repeat_mess.erase(my_repeat_mess.begin()+i);
						} else {
							my_repeat_mess.erase(my_repeat_mess.begin()+i);
							break;
						}
					}
				} else {
					hp_mess2send_t mess = my_repeat_mess[i].create_mess();
					if(mess.mess.find("STOP") == std::string::npos){
						this->add_message(mess);
					} else {
						int shut_pos = this->find_item(my_repeat_mess[i].get_mess().substr(0,2),"id",HP_PIN_OUTPUT,patch::string2int(my_repeat_mess[i].get_xbee_id()));
						if(shut_pos != -1 && my_repeat_mess[i].get_token() != 10){
							if(my_shutters != NULL){
								my_shutters->setup_shut_tilt(my_pins[shut_pos]->get_gui_desc(), my_repeat_mess[i].get_token());
							}
						}
						int pin_pos = find_item(my_repeat_mess[i].get_mess().substr(0,2), "id",HP_PIN_OUTPUT, patch::string2int(my_repeat_mess[i].get_xbee_id()));
						if(my_pins[pin_pos]->get_onOff_ident() != ""){
							int on_off_pin = find_item(my_pins[pin_pos]->get_onOff_ident(), "desc");
							if(on_off_pin != -1){
								this->add_message(on_off_pin, my_pins[pin_pos]->get_status()!="0"?"1":"0");
							}
						}
						if(i != my_repeat_mess.size() -1){
							my_repeat_mess.erase(my_repeat_mess.begin()+i);
						} else {
							my_repeat_mess.erase(my_repeat_mess.begin()+i);
							break;
						}
		
					}
				}
			}
		}
	}
}

void hp_go::check_running_rules()
{
	unsigned int i;
	for(i=0; i<my_rule_checker.size(); i++){
		my_rule_checker[i].counter++;
		int counter = my_rule_checker[i].counter;
		if(counter > SHORT_PIN_RULE && my_rule_checker[i].inactive_count == 0){
			if(my_pins[my_rule_checker[i].pin_pos]->check_input("shutter")){
				if(counter == SHORT_PIN_RULE+1){
					my_rule_checker[i].short_active = true;
					this->process_rule(my_rule_checker[i].pin_pos, 1, "short");
				}
			} else {
				if(!my_pins[my_rule_checker[i].pin_pos]->check_input("long")){
					this->process_rule(my_rule_checker[i].pin_pos, 1, "short");
					if(i != my_rule_checker.size() -1){
						my_rule_checker.erase(my_rule_checker.begin() +i);
					} else {
						my_rule_checker.erase(my_rule_checker.begin() +i);
						break;
					}
				}
			}
		}
		if(counter > DOUBLE_PIN_THRESHOLD && my_rule_checker[i].inactive_count == 1){
			this->process_rule(my_rule_checker[i].pin_pos, 1, "normal");
			if(i != my_rule_checker.size() -1){
				my_rule_checker.erase(my_rule_checker.begin() +i);
			} else {
				my_rule_checker.erase(my_rule_checker.begin() +i);
				break;
			}
		}
		if(counter > LONG_PIN_RULE ){
			this->process_rule(my_rule_checker[i].pin_pos, 1, "long");
			if(i != my_rule_checker.size() -1){
				my_rule_checker.erase(my_rule_checker.begin() +i);
			} else {
				my_rule_checker.erase(my_rule_checker.begin() +i);
				break;
			}
		}
	}
}

hp_mess2send_t hp_go::prepare_mess(string id_xbee, string mess_type, string mess, string debug)
{
	hp_mess2send_t res;
	res.xbee_id = id_xbee;
	res.uc_mess_type = mess_type;
	res.mess = mess;
	res.debug = debug;
	return res;
}

delayed_event_t hp_go::prepare_event(int ev_type, int ev_delay, string ev_var)
{
	delayed_event_t ev;
	ev.event_type = ev_type;
	ev.run_time = time(NULL) + ev_delay;
	ev.var = ev_var;

	return ev;
}

void hp_go::check_sent_messages()
{
	for(vector<hp_send_messages_t>::iterator it = my_send_messages.begin(); it != my_send_messages.end(); ++it){
		if(it->time_form_send < MAX_DELAY_TIME){
			it->time_form_send += DELAY_TIME/1000;
		} else {
			int hbx_pos = atoi(it->xbee_id.c_str())-1;
			it->sent_count++;
			if(it->sent_count < MAX_RESEND_COUNT){
					if(it->mess_type == "8"){
						push_db_query("++++++Message timer elapsed, mess: " + it->mess +", id_xbee: "+it->xbee_id +","+my_hbxs[hbx_pos]->get_name() + " mac: " + my_hbxs[hbx_pos]->get_mac() + " nepreposielam spravu diagnostiky", DB_LOG_COM);
						my_sender->free_uc_id(it->mess.substr(0,1));
					} else {
						push_db_query("++++++Message timer elapsed, mess: " + it->mess +", id_xbee: "+it->xbee_id + " mac: " + my_hbxs[hbx_pos]->get_mac() + " resend: "+patch::to_string(it->sent_count+1), DB_LOG_COM);
						hp_mess2send_t mess;
						if(it->mess_type == "0"){
							mess = prepare_mess(it->xbee_id, "0", it->mess.substr(0),"check_send_messages() type 0");
						} else {
							my_sender->free_uc_id(it->mess.substr(0,1));
							mess = prepare_mess(it->xbee_id, it->mess.substr(1,1), it->mess.substr(2),"check_send_messages()");
						}
						this->add_message(mess, it->sent_count);
					}
			/*	} else {
					if(it->mess_type != "0"){
						my_sender->free_uc_id(it->mess.substr(0,1));
					}
					push_db_query("++++++Message timer elapsed, mess: " + it->mess +", typ: " + it->mess_type + " free_uc_id: " +it->mess.substr(0,1) + ", id_xbee: "+it->xbee_id + " mac: " + my_hbxs[hbx_pos]->get_mac() + ", nepreposielam spravu lebo txstat: "+patch::to_string(it->txstat), DB_LOG_COM);
				}*/
			} else {
				push_db_query("+++++++Message lost: " + it->mess, DB_LOG_COM);
				if(it->mess_type != "0") { 
					my_sender->free_uc_id(it->mess.substr(0,1));
					if(it->mess.find("8X") == std::string::npos){
						this->restart_uc(patch::string2int(it->xbee_id));
						/*
						if(it->mess_type == "1" && DB_EARLY_UPDATE){
							string data = it->mess;
							for(unsigned int j=0; j<(data.length()-2)/5; j++){
								string item_id = data.substr(j*5+2,2); 
								string value = data.substr(j*5+4,3);
							
								int pin_pos = find_item(item_id, "id",-1, hbx_pos+1);
								if(pin_pos != -1){
									//string new_value = my_pins[pin_pos]->set_status(value);
									string funct = my_pins[pin_pos]->get_function();
									if(funct == "status_light" || funct == "pwm" || funct == "socket"){
										push_db_query("UPDATE STATUSES set STATUS = '"+my_pins[pin_pos]->get_status()+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
									}
								} else {
									//cout << "Nenasiel som item, sprava: " << data << endl;
									push_db_query("++++++++++++Nenasiel som prvok v systeme: " + data, DB_LOG_COM);
								}
							}
						}
						*/
					}
				}
			}
			if(hbx_pos >= 0){
				my_hbxs[hbx_pos]->decrease_mess_counter();
				if(patch::to_string(my_hbxs[hbx_pos]->get_hbx_status()) != "37"){
				//	push_db_query("UPDATE hbxs set status = 37, modification = NOW() where mac = '"+my_hbxs[hbx_pos]->get_mac()+"'");
					my_hbxs[hbx_pos]->set_hbx_status(37);
				}
			}
			my_sender->free_xbee_id(patch::string2int(it->frame_id));
			
			if(it != my_send_messages.end()-1){
				my_send_messages.erase(it);
				it--;
			} else {
				my_send_messages.erase(it);
				break;
			}
		}
	}
}

int hp_go::process_driver_resp(string mess)
{
	vector<string> parsed_mess = parse_response(mess);
	if(parsed_mess.size() < 1){
		return -1;
	}
	push_db_query("\t\t\t\t\t\t\t\t\t\tMess from driver: "+mess, DB_LOG_COM);
	if(parsed_mess[0] == "drv"){
		if(parsed_mess[1] == "txstat"){
			process_txstat_mess(parsed_mess);
		}
		if(parsed_mess[1] == "rec"){
			if(parsed_mess[2] == "ack"){
				process_ack_mess(parsed_mess);
			} else if(parsed_mess[2] == "act"){
				process_act_mess(parsed_mess);
			} else if(parsed_mess[2] == "brc"){
				process_brc_mess(parsed_mess);
			} else {
				push_db_query("++++++++Nepodporovana sprava: "+mess, DB_LOG_COM);
			}
		} else if(parsed_mess[1] == "ack"){
			process_xbee_mess(parsed_mess);
		}
	} else if(parsed_mess[0] == "CLS") {
		if(parsed_mess[1] == "meteo"){
			process_meteo_mess(parsed_mess);
		}
	}

	return 0;
}

int hp_go::process_meteo_mess(vector<string> data)
{
	if(data[2] == "winddirection"){
		if (data[3] == "N") {
			data[3] = "1";
		} else if (data[3] == "NNE") {
			data[3] = "2";
		} else if (data[3] == "NE") {
			data[3] = "3";
		} else if (data[3] == "NEE") {
			data[3] = "4";
		} else if (data[3] == "E") {
			data[3] = "5";
		} else if (data[3] == "EES") {
			data[3] = "6";
		} else if (data[3] == "SE") {
			data[3] = "7";
		} else if (data[3] == "SSE") {
			data[3] = "8";
		} else if (data[3] == "S") {
			data[3] = "9";
		} else if (data[3] == "SSW") {
			data[3] = "10";
		} else if (data[3] == "SW") {
			data[3] = "11";
		} else if (data[3] == "SWW") {
			data[3] = "12";
		} else if (data[3] == "W") {
			data[3] = "13";
		} else if (data[3] == "NWW") {
			data[3] = "14";
		} else if (data[3] == "NW") {
			data[3] = "15";
		} else if (data[3] == "NNW") {
			data[3] = "16";
		}
	}
	if(data[2] == "windspeed"){
		if(my_shutters != NULL){
			vector<hp_shutter *> multi_shutter = my_shutters->check_critical_wind(patch::string2int(data[3]));	
			if(multi_shutter.size() != 0 ){
				if(my_shutters->get_move_enabled()){
					push_db_query("+++++++++Critical speed: "+ data[3]+ " vytiahni zaluzie!", DB_LOG_COM);
					if(my_notification != NULL){
						my_notification->push_notification(NN_INFO,"Vyťahujem žalúzie, vietor dosiahol kritickú hodnotu: " + data[3] + " m/s");
					}
					erase_delayed_event();
					for(unsigned int i=0; i<multi_shutter.size(); i++){
						string gui_mess = "shutt_"+multi_shutter[i]->get_gui_ident()+"_allup_1";
						push_delayed_event(prepare_event(EVENT_SHUTTERS, i, gui_mess));
					}
				}
				my_shutters->set_move_enabled(false);
			} else {
				if(!my_shutters->get_move_enabled()){
					if(my_notification != NULL){
						my_notification->push_notification(NN_INFO,"Rýchlosť vetra je v norme, hodnota: " + data[3] + " m/s");
					}
				}
				my_shutters->set_move_enabled(true);
			}
		}
	}
	for(auto &it:my_meteo_idents){
		if(it.ident == data[2]){
			it.value = data[3];
			break;
		}
	}
	fill_shared_memory();
	push_db_query("UPDATE STATUSES set STATUS = "+data[3]+" where ITEM = '"+data[2]+"'"); 
	return 0;
}

int hp_go::process_txstat_mess(vector<string> data)
{
	for(vector<hp_send_messages_t>::iterator it = my_send_messages.begin(); it != my_send_messages.end(); ++it){
		if(patch::string2int(it->frame_id) == patch::string2int(data[2], BASE_HEX) && patch::string2int(it->frame_id) != -1){
			int hbx_pos = atoi(it->xbee_id.c_str())-1;
			if(hbx_pos >= 0){
				my_hbxs[hbx_pos]->decrease_mess_counter();
				it->txstat = patch::string2int(data[3]);
				if(patch::to_string(my_hbxs[hbx_pos]->get_hbx_status()) != data[3]){
				//	push_db_query("UPDATE hbxs set status = "+data[3]+", modification = NOW() where mac = '"+my_hbxs[hbx_pos]->get_mac()+"'");
					my_hbxs[hbx_pos]->set_hbx_status(atoi(data[3].c_str()));
				}
				my_sender->free_xbee_id(patch::string2int(data[2],BASE_HEX));
			}
			break;
		}
	}
	return 0;
}
int hp_go::process_xbee_mess(vector<string> data)
{
	unsigned int i;
	int hbx_pos = -1;
	for(i=0; i<my_hbxs.size(); i++){
		if(data[4].find(my_hbxs[i]->get_mac()) != std::string::npos){
			hbx_pos = my_hbxs[i]->get_pos();
			break;
		}
	}
	if(hbx_pos == -1){
		cout << "Nenasiel som dany HBX: " << data[3] << endl;
		return -1;
	} else {
		for(vector<hp_send_messages_t>::iterator it = my_send_messages.begin(); it != my_send_messages.end(); ++it){
			if(patch::string2int(it->frame_id) == patch::string2int(data[2], BASE_HEX)){
				my_hbxs[hbx_pos-1]->decrease_mess_counter();
				my_sender->free_xbee_id(patch::string2int(data[2],BASE_HEX));
				if(it != my_send_messages.end() -1){
					my_send_messages.erase(it);
					it--;
				} else {
					my_send_messages.erase(it);
					break;
				}
			}
		}
	}
	return 0;
}
int hp_go::process_ack_mess(vector<string> data)
{
	unsigned int i, ballast_lenght;

	if(my_simulator){
		ballast_lenght = 1;
	} else {
		ballast_lenght = 2;
	}
	int hbx_pos = -1;
	string item_id; 
	string value; 
	string uc_mess_id = data[5].substr(0,1);
	bool u_mess =false;

	for(i=0; i<my_hbxs.size(); i++){
		if(data[4].substr(0,data[4].length()-4) == my_hbxs[i]->get_mac()){
			hbx_pos = my_hbxs[i]->get_pos();
			break;
		}
	}
	if(hbx_pos == -1){
		push_db_query("+++++++++++Nenasiel som dany HBX: " + data[4], DB_LOG_COM);
	} else {
		my_hbxs[hbx_pos-1]->set_internal_timer();
		if(data[5] == "U"){
			u_mess = true;
			if(my_hbxs[hbx_pos-1]->get_restart_type() == RESTART_NACK || my_hbxs[hbx_pos-1]->get_restart_type() == RESTART_WATCHDOG){
				push_db_query("\t\t++++Synchronizujem vystupy HBX: " + patch::to_string(hbx_pos) + ", "+my_hbxs[hbx_pos-1]->get_mac()+" s databazou po restarte typu: " + patch::to_string(my_hbxs[hbx_pos-1]->get_restart_type()) , DB_LOG_COM);
				sync_hbx(hbx_pos-1);
			}
		} else if(data[5] == "W"){
		} else {
			bool rek_pin = false;
			for(unsigned int j=0; j<my_repeat_mess.size(); j++){
			}
			if(data[5][1] == UC_CHECK_PORTS){
				for(unsigned int j=0; j<(data[5].length()-ballast_lenght)/3; j++){
					if(my_simulator){
						item_id = data[5].substr(j*3+2,2); 
						value = data[5].substr(j*3+4,1);
						break;
					} else {
						item_id = data[5].substr(j*3+2,2); 
						value = data[5].substr(j*3+4,1);
					}
					int pin_pos = find_item(item_id, "id",-1, hbx_pos);
					if(pin_pos != -1){
						if(my_lightning != NULL){
							std::pair<string, string> res = my_lightning->check_dn_change(my_pins[pin_pos]->get_desc(), patch::string2int(value));
							if(res.first != ""){
								push_db_query("UPDATE STATUSES set STATUS = '"+res.second+"' where ITEM = '"+res.first+"'"); 
								process_dn_change();
							}
						}
						process_jabl_pin(pin_pos, value);
						if(my_pins[pin_pos]->get_status() != value){
							this->process_rule(pin_pos,1,"normal");
							my_pins[pin_pos]->set_status(value);
							if(my_notification != NULL){
								my_notification->check_pin_notification(my_pins[pin_pos]->get_desc(),value,my_pins[pin_pos]->get_desc_2());
							}
						}
					}
				}
			} else if(data[5][1] == UC_BLIND_TIME){
			} else {
				for(unsigned int j=0; j<(data[5].length()-ballast_lenght)/5; j++){
					if(my_simulator){
						item_id = data[5].substr(j*5+1,2); 
						value = data[5].substr(j*5+3,3);
					} else {
						item_id = data[5].substr(j*5+2,2); 
						value = data[5].substr(j*5+4,3);
					}
					if(value == "088"){
						push_db_query("+++++Nuteny restart HBX, pricina 088", DB_LOG_COM);
						this->restart_uc(hbx_pos);
						break;
					}
					int pin_pos = find_item(item_id, "id",-1, hbx_pos);
					if(pin_pos != -1){
						string new_value = my_pins[pin_pos]->set_status(value);
	//					cout << my_pins[pin_pos]->get_id() << " to val: " << new_value << ", " << my_pins[pin_pos]->get_int_type()<<endl;
						if(my_pins[pin_pos]->get_function().find("shutter") == std::string::npos){
							if(my_pins[pin_pos]->get_int_type() == PIN_GATES_ALL){
								this->push_ws_data("gatePin", my_pins[pin_pos]->get_gui_desc(), new_value);
							}
							if(my_gates != NULL){
								process_gate(pin_pos, new_value);
							} 
							if(!has_delayed_event(pin_pos)){
								push_db_query("UPDATE STATUSES set STATUS = '"+new_value+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
								this->push_ws_data("lightning", my_pins[pin_pos]->get_gui_desc(), new_value);
							}
							if(my_watering != NULL){
								hp_watering_zone *zone = my_watering->find_wat_zone(my_pins[pin_pos]->get_desc());
								if(zone != NULL){
									zone->set_running(patch::string2int(new_value));
									push_db_query("UPDATE watering set running = '"+new_value+"' where ident = '"+zone->get_id()+"';",DB_QUERY);
									this->push_ws_data("watering", zone->get_id(), new_value);
								}
							}
						}
						push_all_statuses(my_pins[pin_pos]->get_gui_desc(), new_value);
						if(my_rekuperacia != NULL){
							if(my_rekuperacia->has_rek_pin(my_pins[pin_pos]->get_desc())){
								rek_pin = true;
							}
						}
						if(my_notification != NULL){
							my_notification->check_pin_notification(my_pins[pin_pos]->get_desc(),new_value,my_pins[pin_pos]->get_desc_2());
						}
					} else {
						push_db_query("+++++++++Nenasiel som prvok v systeme: " + data[5], DB_LOG_COM);
					}
				}
			}
			if(my_rekuperacia != NULL && rek_pin){
				vector<string> rek_q = my_rekuperacia->sync_rek(my_pins);
				for(unsigned int k=0; k<rek_q.size(); k++){
					push_db_query(rek_q[k]);
				}
				this->push_ws_data("rekuperacia",my_rekuperacia->get_actual_state() , "");
			}
			fill_shared_memory();
		}
	}
	for(vector<hp_send_messages_t>::iterator it = my_send_messages.begin(); it != my_send_messages.end(); ++it){
		if(uc_mess_id == it->mess.substr(0,1) || (atoi(it->xbee_id.c_str()) == hbx_pos && u_mess) ){
			if(u_mess){
				my_sender->free_uc_id(it->mess.substr(0,1));
			} else {
				my_sender->free_uc_id(uc_mess_id);
			}
			if(!my_sender->is_free_xbee_id(patch::string2int(it->frame_id))){
				my_sender->free_xbee_id(patch::string2int(it->frame_id));
				my_hbxs[hbx_pos-1]->decrease_mess_counter();
			}
			if(it != my_send_messages.end() -1){
				my_send_messages.erase(it);
				it--;
			} else {
				my_send_messages.erase(it);
				break;
			}
		}
	}

	return 0;
}
void hp_go::process_gate(int pin_pos, string value)
{
	hp_gate *gate = my_gates->is_gate_pin(my_pins[pin_pos]->get_desc());
	if(gate != NULL){
		if(gate->get_lock_control_type() != GATE_LOCK_NON){
			if(gate->get_lock_control_type() == GATE_LOCK_INVERZ){
				if(value == "1"){
					value = "0";
				} else {
					value = "1";
				}
			}
			push_db_query("UPDATE STATUSES set STATUS = '"+value+"' where ITEM = '"+gate->get_gui_lock_ident()+"'"); 
			this->push_ws_data("gates",gate->get_gui_lock_ident(), value);
		}
	}
}

void hp_go::process_dia_mess(int hbx_pos)
{
	for(vector<hp_send_messages_t>::iterator it = my_send_messages.begin(); it != my_send_messages.end(); ++it){
		if(hbx_pos == patch::string2int(it->xbee_id)){
			my_sender->free_uc_id(it->mess.substr(0,1));
			if(!my_sender->is_free_xbee_id(patch::string2int(it->frame_id))){
				my_sender->free_xbee_id(patch::string2int(it->frame_id));
				my_hbxs[hbx_pos-1]->decrease_mess_counter();
			}
			if(it != my_send_messages.end() -1){
				my_send_messages.erase(it);
				it--;
			} else {
				my_send_messages.erase(it);
				break;
			}
		}
	}
}

string hp_go::send2temp_messages(string section_name, float query_temp)
{
	if(my2heating != NULL){
		vector<hp_heating_data_t> data = my2heating->process_section(section_name, query_temp);
		for(unsigned int i=0; i<data.size(); i++){
			int pin_pos = find_item(data[i].ident,"desc");
			if(pin_pos != -1){
				this->add_message(pin_pos, data[i].value,false,0, false, false, false, FORCE_HEATING_MESS);
				push_db_query(data[i].debug_data, DB_LOG_COM);
			}
		}
		fill_shared_memory();
	}

	return "";
}

void hp_go::process_act_temp(int hbx_pos,string data)
{
	int pin_pos = this->find_item(data.substr(2,data.length()-8), "id",-1, hbx_pos);
	if(pin_pos != -1){
		string value = data.substr(data.length()-6);
		string temp_value = my_pins[pin_pos]->set_status(value);
		if(my2heating != NULL){ 
			vector<hp_heating_data_t> data = my2heating->process_section_temp(my_pins[pin_pos]->get_desc(),temp_value,my_notification);
			for(unsigned int i=0; i<data.size(); i++){
				int pin_pos = find_item(data[i].ident,"desc");
				if(pin_pos != -1){
					this->add_message(pin_pos, data[i].value,false,0, false, false, false, FORCE_HEATING_MESS);
				}
			}
		}
		
		push_db_query("UPDATE STATUSES set STATUS = '"+temp_value+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
		this->push_ws_data("temperature", my_pins[pin_pos]->get_gui_desc(), temp_value);
		push_all_statuses(my_pins[pin_pos]->get_gui_desc(), temp_value);
		if(my_notification != NULL){
			my_notification->check_pin_notification(my_pins[pin_pos]->get_desc(),value,my_pins[pin_pos]->get_desc_2());
		}
	} else {
		push_db_query("Nenasiel som temlomer: " +data.substr(2,data.length()-8)+ ", " +LOG_CH(), DB_LOG_COM);
	}
}

void hp_go::process_act_cons(int hbx_pos,string data)
{
//	drv_rec_act_0_0013a20040a69ee5fffe_A100012_ok
	int pin_pos = this->find_item(data.substr(0,3), "id",-1, hbx_pos);
	if(pin_pos != -1){
		if(my_has_electro_cons){
			int act_value = (int)patch::string2float(my_pins[pin_pos]->set_status(data.substr(3,5)));
			string cons_db_type = "";
			if(my_pins[pin_pos]->get_int_type() == PIN_ELECTRO_CONS){
				cons_db_type = "1";
			}
			if(my_pins[pin_pos]->get_int_type() == PIN_GAS_CONS){
				cons_db_type = "2";
			}
			push_db_query("UPDATE STATUSES set STATUS = '"+patch::to_string(act_value)+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
			push_db_query("INSERT INTO CONSUMPTION_DATA (`TYPE`, `VALUE`, `FLAG`, `DATE`) VALUES ('"+cons_db_type+"','"+patch::to_string(act_value)+"','0',NOW())",DB_QUERY);
			if(my_notification != NULL){
				my_notification->check_pin_notification(my_pins[pin_pos]->get_desc(),patch::to_string(act_value),my_pins[pin_pos]->get_desc_2());
			}
	
		}
	} else {
		push_db_query("Nenasiel som pi: " +data.substr(0,3)+ " na hbx:  " + patch::to_string(hbx_pos) +"," +LOG_CH(), DB_LOG_COM);
	}
}
void hp_go::process_act_card(int hbx_pos,string data)
{
//	drv_rec_act_0_0013a200409723b2fffe_CARD4ada69328_ok
	if(my_turnikets != NULL){
		card_valid_t valid_card = my_turnikets->is_valid_card(data.substr(5,data.find("_")-5));
		if(valid_card.valid_card){
			if(valid_card.query != ""){
				push_db_query(valid_card.query,DB_QUERY);
			}
			string mac="";
			for(unsigned int i=0; i<my_hbxs.size(); i++){
				if(my_hbxs[i]->get_pos() == hbx_pos){
					mac = my_hbxs[i]->get_mac();
					break;
				}
			}
			if(mac != ""){
				hp_turniket *turniket = my_turnikets->find_turniket(mac);
				if(turniket != NULL){
					vector<turniket_actor_t> actors = turniket->get_actors();
					for(unsigned int i=0; i<actors.size(); i++){
						int pin_pos = find_item(actors[i].ident, "desc");
						if(pin_pos != -1){
							string to_value;
							if(actors[i].hold == 0){
								to_value = "1";
							} else {
								to_value = patch::to_string(actors[i].hold/100+11);
								while(to_value.length() < 3){
									to_value.insert(to_value.begin(), '0');
								}
							}
							this->add_message(pin_pos, to_value);
						}
					}
				}
			} else {
				cout <<"Nenasiel som mac adresu pre hbx: " << hbx_pos << endl;
			}
						
		} else {
			push_db_query("Nenasiel som kartu: " +data+ ", " +LOG_CH(), DB_LOG_COM);
		}
	}
	if(my_cards != NULL){
		hp_card *card = my_cards->find_card(data.substr(5,data.find("_")-5));
		if(card != NULL){
			vector<hp_rule_card_t> res = card->get_valid_rules(hbx_pos);
			for(unsigned int i=0; i<res.size(); i++){
				int pin_pos = find_item(res[i].ident, "desc");
				if(pin_pos != -1){
					this->add_message(pin_pos, res[i].to_value);
				}
			}
		} 
	}
}
	
void hp_go::process_sht(int hbx_pos, string data)
{
}

int hp_go::process_pin_conn(hp_conn_pin_t pin, string to_value, bool active_value) // !!!
{
	//cout << "Processing conn pin, tovalue: " << pin.on_value << " pre pin: " << pin.ident << endl;
	int pin_pos = find_item(pin.ident, "desc");
	if(pin_pos != -1){
		if(pin.on_value == "change"){
			string new_value = pin.to_value;
			if(pin.to_value == "synchro"){
				new_value = to_value;
			} 
			this->add_message(pin_pos, new_value);
		} else {
			if(active_value){
				if(pin.on_value == "1"){
					this->add_message(pin_pos, pin.to_value);
				}
			} else {
				if(pin.on_value == "0"){
					this->add_message(pin_pos, pin.to_value);
				}
			}
			//cout <<"on_value: " << pin.on_value << " output to: " << to_value << endl;
		}
	} else {
		return -1;
	}
	return 0;
}

void hp_go::process_jabl_pin(int pin_pos, string value)
{
	if(my_jablotron != NULL){
		hp_jablotron_zone *zone = my_jablotron->find_zone(my_pins[pin_pos]->get_gui_desc(),"sensor");
		if(zone != NULL){
			if(my_pins[pin_pos]->is_inverz()){
				value = value=="1"?"0":"1";
			}
			zone->set_zone_status(value=="1"?1:0);
			push_db_query("UPDATE SECURITY_ZONES set ALARM_COUNTDOWN = -1, STATUS = "+value+" where ID = '"+zone->get_ident()+"'", DB_QUERY);
			this->push_ws_data("security",zone->get_ident(),value);
			this->push_ws_data("securityCountDown",zone->get_ident(),"-1");
			if(my_notification != NULL){
				try {
					if(my_rooms.find(zone->get_ident()) != my_rooms.end()){
						my_notification->push_security_notification(my_rooms.at(zone->get_ident()),1);
					}
				} catch (const std::exception& e) {
					push_db_query("A standard exception was caught, with message '" + string(e.what()) + "', file: " + __FILE__ +", line: " + patch::to_string(__LINE__), DB_LOG_COM);
				}
			}
			int ja_pos=find_item(zone->get_actor_ident(), "desc");
			if(ja_pos != -1){
				this->add_message(ja_pos, value=="1"?"0":"1",true,0,0,false,true,true);
			}
			start_simulation(1);
		}
	}
}

int hp_go::process_act_mess(vector<string> data)
{
//	drv_rec_act_0_0013a20040a69ef2fffe_D21_ok
	unsigned int i;
	int hbx_pos = -1;
	string item_id; 
	string value; 

	for(i=0; i<my_hbxs.size(); i++){
		if(data[4].find(my_hbxs[i]->get_mac()) != std::string::npos){
			hbx_pos = my_hbxs[i]->get_pos();
			break;
		}
	}
	if(hbx_pos != -1){
		my_hbxs[hbx_pos-1]->set_internal_timer();
		if(data[5][0] == 'X'){
			process_dia_mess(hbx_pos);
		} else if (data[5][0] == 'L' && data[5][1] == '1'){
			process_act_temp(hbx_pos, data[5]);
		} else if (data[5][0] == 'A' && (data[5][1] == '1' || data[5][1] == '2')){
			process_act_cons(hbx_pos, data[5]);
		} else if(data[5].find("CARD") != std::string::npos){
			process_act_card(hbx_pos, data[5]);
		} else if(data[5].find("SHT") != std::string::npos){
			process_sht(hbx_pos, data[5]);
		} else if(data[5].length() >= 3){
			int pin_pos = this->find_item(data[5].substr(0,2), "id",-1, hbx_pos);
			if(pin_pos != -1){
				value = data[5].substr(2,1);
				vector<hp_conn_pin_t> conn_pins = my_pins[pin_pos]->get_conn_pins();
				string pin_funct = my_pins[pin_pos]->get_function();
				
				if(pin_funct == "onoffButton" || pin_funct == "magKontakt" || pin_funct  == "dayTime"){
					if(my2heating != NULL){
						/*
						vector<hp_heating_data_t> data = my2heating->process_section(section_name, query_temp);
						for(unsigned int i=0; i<data.size(); i++){
							int pin_pos = find_item(data[i].ident,"desc");
							if(pin_pos != -1){
								this->add_message(pin_pos, data[i].value,false,0, false, false, false, FORCE_HEATING_MESS);
								push_db_query(data[i].debug_data, DB_LOG_COM);
							}
						}
						*/
					}
					if(my_impulz_counter != NULL){
						my_impulz_counter->process_pin(pin_pos,value);
					}
					if(my_gates != NULL){
						hp_gate *gate = my_gates->is_gate_pin(my_pins[pin_pos]->get_desc());
						if(gate != NULL){
							int pin_value = patch::string2int(data[5].substr(2));
							pin_value = my_pins[pin_pos]->is_inverz()?(!pin_value):pin_value;
							int gate_state = gate->setup_state(pin_value, my_pins[pin_pos]->get_desc());
							push_db_query("UPDATE STATUSES set STATUS = '"+patch::to_string(gate_state)+"' where ITEM = '"+gate->get_status_gui_ident()+"'"); 
							this->push_ws_data("gates", gate->get_status_gui_ident(), patch::to_string(gate_state));
						}
					}
					if(my_watering != NULL){
						vector<hp_wat_data_t> outputs =	my_watering->setup_wat_enabled(my_pins[pin_pos]->get_desc(),patch::string2int(my_pins[pin_pos]->set_status(data[5].substr(2))));
						if(my_watering->is_wat_sensor(my_pins[pin_pos]->get_desc())){
							push_db_query("UPDATE STATUSES set STATUS = '"+my_pins[pin_pos]->set_status(value)+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
						}
						for(unsigned int i=0; i < outputs.size(); i++){
							for(map<string,int>::iterator it = outputs[i].out.begin(); it != outputs[i].out.end(); ++it){
								int pin_pos = find_item(it->first, "desc");
								if(pin_pos != -1 && it->second == 0){
									this->add_message(pin_pos, "0");
								}
							}
						}
					}
					process_jabl_pin(pin_pos, value);
					
					if(my_security != NULL){
						hp_security_zone *zone = NULL;
						zone = my_security->chech_pir(my_pins[pin_pos]->get_gui_desc());
						if(zone != NULL){
							if(zone->get_sec_status() == 1){
								zone->init_countdown();
								my_running_zone.push_back(zone);
							}
							if(zone->get_sec_status() == 2){
								if(zone->repeat_action()){
									vector<string> outputs = my_running_zone[i]->get_actors();
									for(vector<string>::iterator it =outputs.begin(); it != outputs.end(); ++it){
										int item_pos = find_item(*it, "desc");
										if(item_pos != -1){
											this->add_message(item_pos, "1");
											if(my_pins[item_pos]->get_internal_timer() != 0){
												zone->disable_repead(my_pins[item_pos]->get_internal_timer());
											}
										} else {
											push_db_query("Nenasiel som aktora pre zonu: " +my_running_zone[i]->get_id() + ", " +LOG_CH(), DB_LOG_COM);
										}
									}
								}
							}
						}
					}
					if(conn_pins.size() > 0){
						for(i=0; i<conn_pins.size(); i++){
							process_pin_conn(conn_pins[i], value);
						}
					}
					//// check day_night pin
					if(my_lightning != NULL){
						std::pair<string, string> res = my_lightning->check_dn_change(my_pins[pin_pos]->get_desc(), patch::string2int(value));
						if(res.first != ""){
							process_dn_change();
							push_db_query("UPDATE STATUSES set STATUS = '"+res.second+"' where ITEM = '"+res.first+"'"); 
						}
					}
					if(my_turnikets != NULL){
						hp_turniket *turn = my_turnikets->check_turniket(my_pins[pin_pos]->get_desc());
						if(turn != NULL){
							vector<turniket_actor_t> t_actor = turn->check_control(my_pins[pin_pos]->get_desc(), value);
							if(t_actor.size() > 0){
								for(unsigned int i=0; i<t_actor.size(); i++){
									int out_pos = this->find_item(t_actor[i].ident, "desc");
									if(out_pos != -1){
										this->add_message(out_pos, t_actor[i].off_value, false, t_actor[i].off_delay/10);
									}
								}
							}
						}
					}
					process_rule(pin_pos, 1, "normal", true, value);
				} else {
					if(value == my_pins[pin_pos]->get_active_value()){
					//	cout << value << " == " << my_pins[pin_pos]->get_active_value() << endl;
						if(conn_pins.size() > 0){
							for(i=0; i<conn_pins.size(); i++){
								process_pin_conn(conn_pins[i], value);
							}
						}
						bool add_new = true;
						for(i=0; i<my_rule_checker.size(); i++){
							if(my_rule_checker[i].pin_pos == pin_pos){
								add_new = false;
								process_rule(pin_pos, 2, "multi");
								if(i != my_rule_checker.size() -1){
									my_rule_checker.erase(my_rule_checker.begin() +i);
								} else {
									my_rule_checker.erase(my_rule_checker.begin() +i);
									break;
								}
							}
						}
						if(add_new){
							if(my_pins[pin_pos]->check_input("normal")){
								process_rule(pin_pos, 1, "normal");
							} else {
								hp_rule_checker_t tmp_checker;
								tmp_checker.pin_pos = pin_pos;
								tmp_checker.counter = 0;
								tmp_checker.short_active = false;
								tmp_checker.inactive_count = 0;
								my_rule_checker.push_back(tmp_checker);
							}
						}
					} else {
						/*
						if(conn_pins.size() > 0){
							for(i=0; i<conn_pins.size(); i++){
								process_pin_conn(conn_pins[i], value, false);
							}
						}
						*/
						for(i=0; i<my_rule_checker.size(); i++){
							if(my_rule_checker[i].pin_pos == pin_pos){
								if(!my_pins[pin_pos]->check_input("double") || my_rule_checker[i].short_active){
									int counter = my_rule_checker[i].counter;
									if(counter <= SHORT_PIN_RULE){
										process_rule(pin_pos, 1, "normal");
									} else if (counter > SHORT_PIN_RULE && counter < LONG_PIN_RULE){
										process_rule(pin_pos, 1, "short", false);
									} else {
										process_rule(pin_pos, 1, "long");
									}
									if(i != my_rule_checker.size() -1){
										my_rule_checker.erase(my_rule_checker.begin() +i);
									} else {
										my_rule_checker.erase(my_rule_checker.begin() +i);
										break;
									}
								} else {
									if(my_rule_checker[i].counter < DOUBLE_PIN_THRESHOLD){
										my_rule_checker[i].inactive_count++;
									} else {
										if(i != my_rule_checker.size() -1){
											my_rule_checker.erase(my_rule_checker.begin() +i);
										} else {
											my_rule_checker.erase(my_rule_checker.begin() +i);
											break;
										}
									}
								}
							}
						}
						for(i=0; i<my_repeat_mess.size(); i++){
							if(pin_pos == my_repeat_mess[i].get_pin_pos()){
								int shut_pos = this->find_item(my_repeat_mess[i].get_mess().substr(0,2),"id",-1,patch::string2int(my_repeat_mess[i].get_xbee_id()));
								if(shut_pos != -1 && my_repeat_mess[i].get_token() != 10){
									if(my_shutters != NULL){
										my_shutters->setup_shut_tilt(my_pins[shut_pos]->get_gui_desc(), my_repeat_mess[i].get_token());
									}
								}
								if(my_repeat_mess[i].get_service_type() == "lightning"){
									if(my_repeat_mess[i].get_mess().find("255") != std::string::npos){
										int off_pos = find_item(my_repeat_mess[i].get_mess().substr(0,2), "id",HP_PIN_OUTPUT, patch::string2int(my_repeat_mess[i].get_xbee_id()));
										if(my_pins[off_pos]->get_onOff_ident() != ""){
											int on_off_pin = find_item(my_pins[off_pos]->get_onOff_ident(), "desc");
											if(on_off_pin != -1){
												this->add_message(on_off_pin, my_pins[off_pos]->get_status()!="0"?"1":"0");
											}
										}
									}
								}
								my_repeat_mess.erase(my_repeat_mess.begin()+i);
								i--;
							}
						}
						if(my_dali != NULL){
							my_dali->button_off(pin_pos);	
						}
					}
					if(my_pins[pin_pos]->get_function() == "pir"){
						if(my_pins[pin_pos]->get_active_value() == value){
							hp_security_zone *zone = NULL;
							if(my_security != NULL){
								zone = my_security->chech_pir(my_pins[pin_pos]->get_gui_desc());
							}
							if(zone != NULL){
								if(zone->get_sec_status() == 1 && (zone->get_current_countdown() == 0 ||zone->get_current_countdown() == -1) ){
									zone->init_countdown();
									my_running_zone.push_back(zone);
								}
								if(zone->get_sec_status() == 2){
									if(zone->repeat_action()){
										vector<string> outputs = my_running_zone[i]->get_actors();
										for(vector<string>::iterator it =outputs.begin(); it != outputs.end(); ++it){
											int item_pos = find_item(*it, "desc");
											if(item_pos != -1){
												this->add_message(item_pos, "1");
												if(my_pins[item_pos]->get_internal_timer() != 0){
													zone->disable_repead(my_pins[item_pos]->get_internal_timer());
												}
											} else {
												push_db_query("Nenasiel som aktora pre zonu: " +my_running_zone[i]->get_id() + ", " +LOG_CH(), DB_LOG_COM);
											}
										}
									}
								}
							}
						}
					}
				}
				if(pin_funct != "pushButton"  && pin_funct != "multi_button"){
					string db_value = my_pins[pin_pos]->set_status(value);
					push_db_query("UPDATE STATUSES set STATUS = '"+db_value+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
					if(pin_funct ==  "onoffButton") { 
						this->push_ws_data("onoffButton", my_pins[pin_pos]->get_gui_desc(), db_value);
					} else{
						this->push_ws_data("lightning", my_pins[pin_pos]->get_gui_desc(), db_value);
					}
					push_all_statuses(my_pins[pin_pos]->get_gui_desc(), db_value);
				}				
				if(my_notification != NULL){
					my_notification->check_pin_notification(my_pins[pin_pos]->get_desc(),value,my_pins[pin_pos]->get_desc_2());
				}
				fill_shared_memory();
			}
		}
	}
	return 0;
}

void hp_go::check_conditions(int pos, string to_value)
{
	if(my_conditions != NULL){
		vector<cond_struc_t> cond = my_conditions->check_condition(pos, to_value);
		for(uint16_t i=0; i<cond.size(); i++){
			if(cond[i].pin_pos != -1){
				hp_mess2send_t tmp2send = my_pins[cond[i].pin_pos]->create_send_mess(cond[i].to_value);
				my_sender->add_message(tmp2send);
			}
		}
	}
}

void hp_go::add_message(int pin_pos, string to_value, bool check_timer_hold, int delay, int create_off_mess , bool delete_timer_mess, bool add_repeat, bool force_add)
{
	hp_mess2send_t m1,m2,m3; 
	if(pin_pos > -1 && pin_pos < (int)my_pins.size()){
		push_db_query("Vytvaram spravu pre pin: " + my_pins[pin_pos]->get_desc() + " to_value: " + to_value + " check_timer_hold: " + patch::to_string(check_timer_hold) + " delay: " + patch::to_string(delay) + " create off mess: " + patch::to_string(create_off_mess), DB_LOG_COM);
		vector<hp_conn_pin_t> conn_pins = my_pins[pin_pos]->get_conn_pins();
		if(my_pins[pin_pos]->is_inverz() || my_pins[pin_pos]->get_internal_timer() > 10 || my_use_old_hbx_timer){
			m1 = my_pins[pin_pos]->create_send_mess(to_value,false);
			if(m1.to_value != my_pins[pin_pos]->get_status() || my_pins[pin_pos]->get_function() == "gate" || force_add){// || my_pins[pin_pos]->get_function() == "heater"){
				my_sender->add_message(m1,0,0,delay);
			}
			if(check_timer_hold && to_value != "0"){
				m2 = my_pins[pin_pos]->create_send_mess("0",true);
				if(m2.timer_hold != -1){
					my_sender->add_message(m2,0,0,m2.timer_hold*100);
				}
			}
		} else {
			m1 = my_pins[pin_pos]->create_send_mess(to_value,true);
			if(m1.to_value != my_pins[pin_pos]->get_status() || my_pins[pin_pos]->get_function() == "gate" || force_add){// || my_pins[pin_pos]->get_function() == "heater"){
				my_sender->add_message(m1,0,0,delay);
				if(m1.timer_hold != -1){
					push_delayed_event(this->prepare_event(EVENT_OFF_PIN, m1.timer_hold,patch::to_string(pin_pos)));
				}
			}
		}
		if(m1.to_value == "1"){
		//	cout << m1.repeat_timer << " !=  -1 && " << m1.service_type <<" == \"repeat_mess\" && " << add_repeat << endl;
			if(m1.repeat_timer != -1 && m1.service_type == "repeat_mess" && add_repeat){
				bool add_r = true;
				for(unsigned int j=0; j<my_repeat_mess.size(); j++){
					if(pin_pos == my_repeat_mess[j].get_pin_pos() && my_repeat_mess[j].get_service_type() == "repeat_mess"){
						add_r = false;
						break;
					}
				}
				if(add_r){
					my_repeat_mess.push_back(hp_repeat_mess(m1, 100*m1.repeat_timer, "1", pin_pos, m1.max_triggers));
				}
			}
		} else {
			for(unsigned int j=0; j<my_repeat_mess.size(); j++){
				if(pin_pos == my_repeat_mess[j].get_pin_pos() && my_repeat_mess[j].get_service_type() == "repeat_mess"){
					my_repeat_mess.erase(my_repeat_mess.begin()+j);
					j--;
				}
			}
		}
		check_conditions(pin_pos, m1.to_value);
		if(create_off_mess){
			push_delayed_event(this->prepare_event(EVENT_TIMER_CLOCK_MESS, create_off_mess,patch::to_string(pin_pos)));
			/*
			m3 = my_pins[pin_pos]->create_send_mess("0");
			my_sender->add_message(m3,0,0,create_off_mess*100);
			*/
		}
		if(delete_timer_mess){
			my_sender->delete_timer_mess(m1.mess, m1.xbee_id);
		}
		if(conn_pins.size() > 0){
			for(unsigned int i=0; i<conn_pins.size(); i++){
				process_pin_conn(conn_pins[i], m1.to_value, my_pins[pin_pos]->get_active_value() == m1.to_value ? true:false); /// tutok este porozmyslat ako to spravne spravit
			}
		}
		string on_off_pin = my_pins[pin_pos]->get_onOff_ident();
		if(on_off_pin != ""){
			int conn_pos = find_item(on_off_pin, "desc");
			if(conn_pos != -1){
				hp_mess2send_t conn_mess = my_pins[conn_pos]->create_send_mess(m1.mess.find("255") == std::string::npos?"1":"0");
				check_conditions(conn_pos, m1.mess.find("255") == std::string::npos?"1":"0");
				my_sender->add_message(conn_mess);
			}
		}
	} else {
		cout <<"Pruser pre: " << pin_pos << endl;
	}
}

void hp_go::add_message(hp_mess2send_t mess, int resend_count, int priority, int delay )
{
#ifdef YIT
	regex rx("P.255");
	if(std::distance(std::sregex_iterator(mess.mess.begin(), mess.mess.end(), rx),std::sregex_iterator()) != 0){
		int pin_pos = find_item(mess.mess.substr(0,2),"id",-1,patch::string2int(mess.xbee_id));
		if(pin_pos != -1){
			my_pins[pin_pos]->set_status("0");
			push_db_query("UPDATE STATUSES set STATUS = 0 where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
		}
	} else {
		my_sender->add_message(mess,resend_count,priority,delay);
	}
#else
	my_sender->add_message(mess,resend_count,priority,delay);
	int pin_pos = find_item(mess.mess.substr(0,2),"id",-1,patch::string2int(mess.xbee_id));
	if(pin_pos != -1){
		if(my_lightning != NULL){
			hp_lightning_rule *out_rule = my_lightning->find_rule(my_pins[pin_pos]->get_desc(), 1, "normal");
			if(out_rule != NULL){
				vector<hp_rule_light_t> outputs2 = out_rule->get_outputs();
				string to_value = mess.mess;
				for(unsigned int j=0; j<outputs2.size(); j++){
					int out_pos2 = this->find_item(outputs2[j].ident, "desc");
					if(out_pos2 != -1){
						if(outputs2[j].to_value == "synchro"){
							if(mess.mess.substr(2,3) == "255"){
								this->add_message(out_pos2, "0");
							} else {
								this->add_message(out_pos2, patch::to_string(patch::string2int(mess.mess.substr(2,3))));
							}
						} 
					}
				}
			}
		}
	}
#endif
}
void hp_go::add_message(string hbx_pos, string mess_type, string mess, int resend_count, int priority, int delay)
{
	my_sender->add_message(prepare_mess(hbx_pos,mess_type,mess),resend_count,priority,delay);
}

void hp_go::add_mess2sender(hp_mess2send_t mess, int pin_pos, int delay)
{
	if(mess.to_value != my_pins[pin_pos]->get_status() || my_pins[pin_pos]->get_function() == "gate"){
		if(delay != 0){
			my_sender->add_message(mess,0,0,delay);
		} else {
			my_sender->add_message(mess);
		}
		hp_mess2send_t timer_mess = my_pins[pin_pos]->create_send_mess("0",true);
		if(timer_mess.timer_hold != -1){
			my_sender->add_message(timer_mess,0,0,timer_mess.timer_hold*100);
		}
	} 
	
	vector<hp_conn_pin_t> conn_pins = my_pins[pin_pos]->get_conn_pins();
	if(conn_pins.size() > 0){
		for(unsigned int i=0; i<conn_pins.size(); i++){
			process_pin_conn(conn_pins[i], mess.to_value, my_pins[pin_pos]->get_active_value() == mess.to_value ? true:false);
		}
	}
	string on_off_pin = my_pins[pin_pos]->get_onOff_ident();
	if(on_off_pin != ""){
		int conn_pos = find_item(on_off_pin, "desc");
		if(conn_pos != -1){
			hp_mess2send_t conn_mess = my_pins[conn_pos]->create_send_mess(mess.mess.find("255") == std::string::npos?"1":"0");
			my_sender->add_message(conn_mess);
		}
	}
}

void hp_go::find_out_rule(string ident,int actor_pos, string pwm_value,hp_mess2send_t *mess)
{
	if(my_lightning != NULL){
		hp_lightning_rule *out_rule = my_lightning->find_rule(ident, 1, "normal");
		if(out_rule != NULL){
		//cout << "Out rule mess: " << mess->mess << ", " << mess->service_type << ", " << mess->to_value << endl;
			vector<hp_rule_light_t> outputs2 = out_rule->get_outputs();
			string to_value;
			if(mess != NULL){
				to_value = mess->mess;
			} else {
				to_value = pwm_value;
			}
			for(uint16_t j=0; j<outputs2.size(); j++){
				int out_pos2 = this->find_item(outputs2[j].ident, "desc");
				if(out_pos2 != -1){
					if(outputs2[j].to_value == "synchro"){
						if(my_pins[actor_pos]->get_function().find("pwm")  != std::string::npos){
							this->add_message(out_pos2,pwm_value);
						} else {
							if(mess != NULL){
								this->add_message(out_pos2,mess->to_value,true,30+j*10);
							} else {
								this->add_message(out_pos2,to_value,true,30+j*10);
							}
						}
					} else {
						this->add_message(out_pos2, outputs2[j].to_value,true,30+j*10);
					}
				}
			}
		}
	}
}

int hp_go::process_rule(int pin_pos, int pushed, string hold, bool is_active_value, string on_value)
{
	unsigned int i;
	bool rule_found = false;
	hp_lightning_rule *rule = NULL;
	if(my_lightning != NULL){ 
		rule = my_lightning->find_rule(my_pins[pin_pos]->get_desc(),pushed,hold, on_value);
		string pin_function = my_pins[pin_pos]->get_function();
		if(pin_function == "onoffButton" || pin_function == "magKontakt" || pin_function == "dayTime"){
			if(rule != NULL){
				if(rule->get_on_value() != on_value && rule->get_on_value() != ""){
					rule = NULL;
				}
			}
		}
	}
	vector<hp_shutter *> multi_shutter;
	if(my_shutters != NULL){
		multi_shutter= my_shutters->find_multi_rule(my_pins[pin_pos]->get_desc());
	}
	if(rule != NULL){
		bool first_mess = true;
		rule_found = true;
		vector<hp_rule_light_t> outputs = rule->get_outputs();
		push_db_query("\tProcessing pravidlo osvetlenia, actor: "+rule->get_actor()+ ", pushed: "+patch::to_string(rule->get_pushed()) + ",hold: " + rule->get_hold() + ", priorita: " + patch::to_string(rule->get_rule_priority()) + " on_value: " + on_value, DB_LOG_COM);		
		for(i=0; i<outputs.size(); i++){
			int out_pos = this->find_item(outputs[i].ident, "desc");
			if(out_pos != -1){
				if(my_pins[out_pos]->get_priotity_state() && !rule->get_rule_priority()){
					push_db_query("\t\tPin: "+my_pins[out_pos]->get_gui_desc()+" ma stav: " +my_pins[out_pos]->get_status()+" a je prioritny, ignorujem pravidlo pinu: "+my_pins[pin_pos]->get_desc() , DB_LOG_COM);
					continue;
				}
				if(outputs[i].to_value != "0"){
					my_pins[out_pos]->set_priority_state(rule->get_rule_priority());
				}
				
				hp_mess2send_t mess = my_pins[out_pos]->create_send_mess(outputs[i].to_value);
				if(outputs[i].to_value == "fluently"){
					if(mess.mess.find("STOP") != std::string::npos){
						continue;
					}
				}
				if(outputs[i].to_value == "fluently"){
					my_repeat_mess.push_back(hp_repeat_mess(mess, 60, mess.last_direction, pin_pos));
				}
				if(my_pins[out_pos]->get_function() == "pwm"){
					if(outputs[i].to_value != "fluently"){
						if(outputs[i].my_pwm_on != "9" && mess.to_value == "9"){
							outputs[i].to_value = outputs[i].my_pwm_on;
						} 
						if(outputs[i].my_pwm_off != "0" && mess.to_value == "0"){
							outputs[i].to_value = outputs[i].my_pwm_off;
						} 
					}
				}
				if(outputs[i].timer_type == "timer" && outputs[i].timer_time != "0" && patch::string2int(outputs[i].to_value) > 0){ //(outputs[i].to_value == "1" || outputs[i].to_value == "9"))
					this->add_message(out_pos,outputs[i].to_value, false, first_mess?0:(10+i*DELAY_CONSTANT),(atoi(outputs[i].timer_time.c_str())));
					first_mess = false;
				} else {
					if(my_pins[out_pos]->get_function() == "pwm"){
						this->add_message(out_pos, outputs[i].to_value);
					} else {
						this->add_message(out_pos, outputs[i].to_value, true, first_mess?0:(10+i*DELAY_CONSTANT));
						first_mess = false;
					}
				}
				if(outputs[i].timer_type == "clock" && outputs[i].timer_time != "0"){
					/// toto by som mal spravit cez delyed event
					mess = my_pins[out_pos]->create_send_mess("0");
					time_t rawtime;
					struct tm * timeinfo;
					time ( &rawtime );
					timeinfo = localtime ( &rawtime );
					int delay_value = 0, current_value = 0;
					current_value = timeinfo->tm_min + timeinfo->tm_hour*60;
					if(current_value < patch::string2int(outputs[i].timer_time)){
						delay_value = patch::string2int(outputs[i].timer_time) - current_value;
					}
					if(delay_value > 0){
						push_delayed_event(this->prepare_event(EVENT_TIMER_CLOCK_MESS, delay_value*60-timeinfo->tm_sec,patch::to_string(out_pos)));
					}
				}
				//void hp_go::find_out_rule(string ident,int actor_pos, string pwm_value,hp_mess2send_t *mess)
				find_out_rule(outputs[i].ident, out_pos, outputs[i].to_value, &mess);
/*
				hp_lightning_rule *out_rule = my_lightning->find_rule(outputs[i].ident, 1, "normal");
				if(out_rule != NULL){
					//cout << "Out rule mess: " << mess.mess << ", " << mess.service_type << ", " << mess.to_value << endl;
					vector<hp_rule_light_t> outputs2 = out_rule->get_outputs();
					string to_value = mess.mess;
					for(j=0; j<outputs2.size(); j++){
						int out_pos2 = this->find_item(outputs2[j].ident, "desc");
						if(out_pos2 != -1){
							if(outputs2[j].to_value == "synchro"){
								if(my_pins[out_pos]->get_function().find("pwm")  != std::string::npos){
									this->add_message(out_pos2,outputs[i].to_value);
								} else {
									this->add_message(out_pos2,mess.to_value,true,100+j*10);
								}
							} else {
								this->add_message(out_pos2, outputs2[j].to_value,true,100+j*10);
							}
						}
					}
				}
				*/
			} else {			
				hp_button_scenario *b_scen = NULL;
				for(uint16_t j=0; j<my_button_scen.size(); j++){
					if(outputs[i].ident.find(my_button_scen[j]->get_scen_ident()) != std::string::npos){
						b_scen = my_button_scen[j];
					}
				}
				if(b_scen != NULL){
					process_gui_comm("",b_scen);
				} else if(my_dali != NULL){
					vector<int> res = my_dali->process_light_rule(outputs[i], pin_pos);
					if(res.size() == 0){
						push_db_query("\t\t----Nenasiel som vystupny pin: " + outputs[i].ident + " pravidlo pinu: "+my_pins[pin_pos]->get_desc() , DB_LOG_COM);
					} else {
						if(my_lightning != NULL){
							hp_lightning_rule *out_rule = my_lightning->find_rule(outputs[i].ident, 1, "normal");
							if(out_rule != NULL){
								int dali_button = pin_pos;
								if(outputs[i].to_value != "fluently"){
									dali_button = -1;
								}
								for(uint16_t i=0; i<out_rule->get_outputs().size(); i++){
									if(out_rule->get_outputs()[i].to_value == "synchro"){
										my_dali->push_synchro_mess(res, dali_button, out_rule->get_outputs()[i].ident);
										//my_dali->process_gui_command(out_rule->get_outputs()[i].ident + "_" +patch::to_string(res[0]));
									}
								}
							}
						}
					}
					//cout << outputs[i].ident << " to val: " << outputs[i].my_pwm_on << endl;
				} else {
					push_db_query("\t\t----Nenasiel som vystupny pin: " + outputs[i].ident + " pravidlo pinu: "+my_pins[pin_pos]->get_desc() , DB_LOG_COM);
				}
			}
		}
	} else {
		if(my_pins[pin_pos]->get_function() != "pir"){
			//push_db_query("\t\t----Nenasiel som ziadne platne pravidlo osvetlenia pre: "+my_pins[pin_pos]->get_desc() , DB_LOG_COM);
		}
	}
	if(multi_shutter.size() > 0 && (hold == "short" || hold == "long")){
		rule_found = true;
		for(i=0; i<multi_shutter.size(); i++){
			if(hold == "short"){
				if(is_active_value){
					multi_shutter[i]->set_move_direction(multi_shutter[i]->get_last_move_direction() == SHUT_UP?SHUT_DOWN:SHUT_UP);
					string last_dir = multi_shutter[i]->get_last_move_direction();
		
					if(multi_shutter[i]->get_pin_on_off() != -1 && multi_shutter[i]->get_pin_on_off() < (int)my_pins.size() && multi_shutter[i]->get_pin_dir() != -1 && multi_shutter[i]->get_pin_dir() < (int)my_pins.size()){
						int move_steps = multi_shutter[i]->get_mevement_steps(last_dir);
						if( move_steps != 0){
							int added_time = last_dir==SHUT_DOWN?10:0;
							int move_time = 0;
	
							if(SHUTTER_CONTROL == 0){
								move_time = SHUTTER_STEP_TIME+added_time;
							} else if(SHUTTER_CONTROL == 1){
								move_time = 1;
							}
							this->add_message(multi_shutter[i]->get_pin_on_off(), patch::to_string(move_time), false, 0, 0, true);
							if(SHUTTER_CONTROL == 0){
								hp_mess2send_t mess = my_pins[multi_shutter[i]->get_pin_on_off()]->create_send_mess(patch::to_string(move_time));
								my_repeat_mess.push_back(hp_repeat_mess(mess, SHUTTER_STEP_TIME*SHUTTER_STEP_INDEX, last_dir,pin_pos,move_steps-1));
							}
				
							this->add_message(multi_shutter[i]->get_pin_dir(), last_dir);
							if(SHUTTER_CONTROL == 0){
								my_repeat_mess.push_back(hp_repeat_mess(my_pins[multi_shutter[i]->get_pin_dir()]->create_send_mess(last_dir), SHUTTER_STEP_TIME*SHUTTER_STEP_INDEX, last_dir, pin_pos,move_steps-1));
							}
						}
					} else {
						cout << "Neplatna pozicia pre shutter pin,  onoffpin: " << multi_shutter[i]->get_pin_on_off() << "\t dir pin: " << multi_shutter[i]->get_pin_dir() << endl;
					}
				} else {
					if(multi_shutter[i]->get_pin_on_off() != -1 && multi_shutter[i]->get_pin_on_off() < (int)my_pins.size() && multi_shutter[i]->get_pin_dir() != -1 && multi_shutter[i]->get_pin_dir() < (int)my_pins.size()){
						if(SHUTTER_CONTROL == 1){
							this->add_message(multi_shutter[i]->get_pin_on_off(),"0", false, 0, 0, true);
						}
					}
				}
			} else if (hold == "long"){
				if(SHUTTER_CONTROL == 0){
					for(unsigned int j=0; j<my_repeat_mess.size(); j++){
						if(pin_pos == my_repeat_mess[j].get_pin_pos()){
							my_repeat_mess.erase(my_repeat_mess.begin()+j);
							j--;
						}
					}
				}
				if(multi_shutter[i]->get_pin_on_off() != -1 && multi_shutter[i]->get_pin_on_off() < (int)my_pins.size() && multi_shutter[i]->get_pin_dir() != -1 && multi_shutter[i]->get_pin_dir() < (int)my_pins.size()){
					string last_dir = multi_shutter[i]->get_last_move_direction();
					multi_shutter[i]->set_direct_tilt(last_dir == SHUT_UP?0:10);
					this->add_message(multi_shutter[i]->get_pin_on_off(),"0",false,multi_shutter[i]->get_my_timer()*100);
		
				} else {
					cout << "Neplatna pozicia pre shutter pin,  onoffpin: " << multi_shutter[i]->get_pin_on_off() << "\t dir pin: " << multi_shutter[i]->get_pin_dir() << endl;
				}
			}
		}
	}

	if(!rule_found){
		if(my_pins[pin_pos]->get_function() != "pir"){
			push_db_query("\t\t----Nenasiel som ziadne platne pravidlo pre: "+my_pins[pin_pos]->get_desc() , DB_LOG_COM);
		}
	}
	
	return true;
}

int hp_go::process_brc_mess(vector<string> data)
{
	unsigned int i;
	int hbx_pos = -1;
	string item_id; 
	string value; 

	for(i=0; i<my_hbxs.size(); i++){
		if(data[4].find(my_hbxs[i]->get_mac()) != std::string::npos){
			hbx_pos = my_hbxs[i]->get_pos();
			my_hbxs[i]->set_restart_type(RESTART_WATCHDOG);
			break;
		}
	}
	if(hbx_pos != -1){
		setup_hbx_outputs(hbx_pos);
		send_U_message(hbx_pos,100);
		if(data.size() >= 6){
			if(data[5] != "U"){
				process_ack_mess(data);
			}
		}
	}

	return 0;
}

int hp_go::process_gui_mess(string mess)
{
	std::map<string,string>::iterator it_td = my_trusted_devices.end();

	if(mess == ""){
		push_db_query("Gui sprava chyba: " + mess, DB_LOG_COM);
		return 0;
	}

	while(mess[mess.length()-1] == '_' || mess[mess.length()-1] == 0x0a){
		mess = mess.substr(0, mess.length()-1);
	}
	if(my_check_trusted_devices){
		string tmp = mess.substr(0,mess.find("_"));
		it_td = my_trusted_devices.find(tmp.substr(0,tmp.find("_")));
		if(it_td == my_trusted_devices.end()){
			push_db_query("!!!Neautorizovane zariadenie, sprava: " + mess, DB_LOG_COM);
			return 0;
		}
	}
	push_db_query("Gui sprava: " + mess + (my_check_trusted_devices?(" \tzo zariadenia: "+it_td->second):""), DB_LOG_COM);
	push_all_statuses(mess.substr(0,mess.length()-1), "0");
	mess = mess.substr(mess.find("_")+1);
	mess = mess.substr(mess.find("_")+1);

	if(mess.find("hp_go_exit") != std::string::npos){
		return -1;
	}
	if(mess.find("rereadXML") != std::string::npos){
		reread_xml();
		return 0;
	}
	if(mess.find("printReport") != std::string::npos){
		print_report();
		return 0;
	}

	string ident="", query_value = "";


	if(mess.find("restartHBX") != std::string::npos){
		query_value = mess.substr(mess.find("_")+1);
		int hbx_pos = patch::string2int(query_value);
		if(hbx_pos > 0 && hbx_pos <= (int)my_hbxs.size()){
			this->restart_uc(hbx_pos);
		}
		return 0;
	}
	if(mess.find("alarm_") != std::string::npos){
		return process_gui_alarm_mess(mess);
	} else if(mess.find("shutt_") != std::string::npos){
		return process_gui_shutt_mess(mess);
	} else if(mess.find("scen_") != std::string::npos){
		return process_gui_scen_mess(mess);
	} else if(mess.find("secur_") != std::string::npos){ 
		return process_gui_secur_mess(mess);
	} else if(mess.substr(0,5) == "temp_"){
		return process_gui_temp_mess(mess);
	} else if( mess.substr(0,5) == "comm_"){
		return process_gui_comm(mess);
	} else if( mess.substr(0,4) == "wat_"){
		return process_gui_wat_mess(mess);
	} else if( mess.substr(0,4) == "air_"){
		return process_gui_air_mess(mess);
	} else if( mess.substr(0,7) == "extpin_"){
		return process_ext_pin(mess);
	} else if(my_turnikets != NULL){
		if(mess.find(my_turnikets->get_gui_ident()) != std::string::npos){
			process_gui_turnikets(mess);
			return 0;
		}
	}
	string new_gate_value ="";
	ident = mess.substr(0,mess.find("_"));
	query_value = mess.substr(mess.find("_")+1,1);
	if(my_gates != NULL){
		hp_gate *gate = my_gates->is_gate_pin(mess.substr(0,mess.find("_")));
		if(gate != NULL){
			ident = gate->get_move_ident(mess.substr(mess.find("_")+1,1));
			query_value = "1";
			/// |||
			if(gate->get_status_pin_count() == 0){
				push_db_query("UPDATE STATUSES set STATUS = '"+patch::to_string(gate->setup_state(0,""))+"' where ITEM = '"+gate->get_gui_lock_ident()+"'"); 
				this->push_ws_data("doorman",gate->get_gui_lock_ident(),patch::to_string(gate->setup_state(0,"")));
			}
			/*
			if(gate->update_control_pin()){
				if(gate->update_control_pin() == UPDATE_GATE_CONTROL_INVERZ){
					new_gate_value = "0";
				} else {
					new_gate_value = "1";
				}
				//push_db_query("UPDATE STATUSES set STATUS = '"+new_value+"' where ITEM = '"+my_pins[pin_pos]->get_gui_desc()+"'"); 
			} else {
				new_gate_value = "-1";
			}
			ident = gate->get_move_ident(mess.substr(mess.find("_")+1,1));
			query_value = "1";
			*/
		}
	}

	if(my_rekuperacia != NULL){
		hp_rekuperacia_zone *rek_zone =my_rekuperacia->find_zone(ident);
		if(rek_zone != NULL){
			vector<rek_actor_t> actors = rek_zone->get_actors();
			for(unsigned i=0; i<actors.size(); i++){
				int pos = find_item(actors[i].actor,"desc");
				if(pos != -1){
					this->add_message(pos, actors[i].to_value);
					/*
					hp_mess2send_t send_mess = my_pins[pos]->create_send_mess(actors[i].to_value);
					add_mess2sender(send_mess, pos);
					*/
				} else {
					push_db_query("+++++++++Nenasiel som prvok v systeme: " + actors[i].actor, DB_LOG_COM);
				}
			}
			return 0;
		}
	}
	if(my_security != NULL){
		if(ident == my_security->get_simulation_ident() && my_security->is_sim_enabled()){
			my_security->set_sim_value(patch::string2int(query_value));
			push_db_query("UPDATE STATUSES set STATUS = "+query_value+"  where ITEM = '"+my_security->get_simulation_ident()+"'");
			if(query_value == "1"){
				start_simulation(1);
			} else {
				start_simulation(0);
			}
			return 0;
		}
	}
	if(my_jablotron != NULL){
		if(ident == my_jablotron->get_simulation_ident() && my_jablotron->is_sim_enabled()){
			my_jablotron->set_sim_value(patch::string2int(query_value));
			push_db_query("UPDATE STATUSES set STATUS = "+query_value+"  where ITEM = '"+my_jablotron->get_simulation_ident()+"'", DB_QUERY);
			if(query_value == "1"){
				start_simulation(1);
			} else {
				start_simulation(0);
			}
			return 0;
		}

	}

	int item_pos = find_item(ident,"desc",HP_PIN_OUTPUT);
	if(item_pos != -1) {
		if(DB_EARLY_UPDATE){
			push_db_query("UPDATE STATUSES set STATUS = '"+query_value+"' where ITEM = '"+my_pins[item_pos]->get_gui_desc()+"'"); 
		}
		this->add_message(item_pos, query_value, true, 0, 0, false,true, true);
		my_pins[item_pos]->set_priority_state(true);
		
		hp_lightning_rule *out_rule =NULL; 
		if(my_lightning != NULL){
			out_rule = my_lightning->find_rule(my_pins[item_pos]->get_desc(), 1, "normal");
		}
		if(out_rule != NULL){
			vector<hp_rule_light_t> outputs2 = out_rule->get_outputs();
			for(unsigned int j=0; j<outputs2.size(); j++){
				int out_pos2 = this->find_item(outputs2[j].ident, "desc");
				if(out_pos2 != -1){
					if(outputs2[j].to_value == "synchro"){
//						this->add_message(out_pos2, mess.substr(mess.find("_")+1));
						this->add_message(out_pos2,mess.substr(mess.find("_")+1),true,30+j*10);
						if(DB_EARLY_UPDATE){
							push_db_query("UPDATE STATUSES set STATUS = '"+query_value+"' where ITEM = '"+my_pins[out_pos2]->get_gui_desc()+"'"); 
						}
					} else {
						this->add_message(out_pos2, outputs2[j].to_value);
						if(DB_EARLY_UPDATE){
							if(outputs2[j].to_value != "change"){
								push_db_query("UPDATE STATUSES set STATUS = '"+outputs2[j].to_value+"' where ITEM = '"+my_pins[out_pos2]->get_gui_desc()+"'"); 
							}
						}
					}
					my_pins[out_pos2]->set_priority_state(true);
				}
			}
		}
	} else {
		if(my_lightning != NULL){
			if(ident == my_lightning->get_dn_ident()){
				my_lightning->set_actual_mode(query_value);
				push_db_query("UPDATE STATUSES set STATUS = '"+query_value+"' where ITEM = '"+ident+"'"); 
				process_dn_change();
			}
		}
		if(my_dali != NULL){
			if(my_dali->process_gui_command(mess) == -1){
				push_db_query("Nenasiel som ident spravy: " + mess, DB_LOG_COM);
			} else {
				hp_lightning_rule *out_rule = my_lightning->find_rule(mess.substr(0,mess.find("_")), 1, "normal");
				if(out_rule != NULL){
					for(uint16_t i=0; i<out_rule->get_outputs().size(); i++){
						if(out_rule->get_outputs()[i].to_value == "synchro"){
							my_dali->process_gui_command(out_rule->get_outputs()[i].ident + "_" +mess.substr(mess.find("_")+1));
						}
					}
				}
			}
		} else {
			bool nenasiel = true;
			if(my2heating != NULL){
				string new_val = mess.substr(mess.find_last_of("_")+1);
				if(ident == "defTemperature"){
					nenasiel = false;
					my2heating->set_def_temp(patch::string2float(new_val));
					push_db_query("UPDATE STATUSES set STATUS = '"+new_val+"' where ITEM = '"+ident+"'"); 
				}
				if(ident == "defTemperingValue"){
					my2heating->set_def_tempering(patch::string2float(new_val));
					nenasiel = false;
					push_db_query("UPDATE STATUSES set STATUS = '"+new_val+"' where ITEM = '"+ident+"'"); 
				}
			}
			if(nenasiel){
				push_db_query("Nenasiel som ident spravy: " + mess, DB_LOG_COM);
			}
		}
	}

	return 0;
}

void hp_go::check_heating_zones(int mode, int delay, bool set_temp)
{
	if(my2heating != NULL){
		vector<hp_heating_data_t> data = my2heating->set_heating_mode(mode,set_temp);
		for(unsigned int i=0; i<data.size(); i++){
			int pin_pos = find_item(data[i].ident,"desc");
			if(pin_pos != -1){
				this->add_message(pin_pos, data[i].value, false, delay);
			}
		}
	}
}

int hp_go::process_ext_pin(string mess)
{
	vector<string> parsed = parse_response(mess);
	if(parsed.size() > 1){
		for(uint16_t i=0; i<my_pins.size(); i++){
			if(my_pins[i]->get_desc() == parsed[1]){
				string pin_funct = my_pins[i]->get_function();
				if(pin_funct == "onoffButton" || pin_funct == "magKontakt" || pin_funct  == "dayTime"){
					string db_value = my_pins[i]->set_status(parsed[2]);
					push_db_query("UPDATE STATUSES set STATUS = '"+db_value+"' where ITEM = '"+my_pins[i]->get_gui_desc()+"'"); 
					process_rule(i, 1, "normal", true, parsed[2]);
				}
			}
		}
	}
	return 0;
}
int hp_go::process_gui_turnikets(string mess)
{
	vector<string> parsed = this->parse_response(mess);
	
	if(parsed.size() < 2){
		push_db_query("delete from "+my_turnikets->get_table_name()+" where card_id = '"+parsed[2]+"'",DB_QUERY);
		my_turnikets->remove_card(parsed[2]);
	
		push_db_query("INSERT INTO "+my_turnikets->get_table_name()+" (card_id, valid_until,card_type) VALUES ('"+parsed[2]+"', FROM_UNIXTIME(0), "+parsed[1]+")",DB_QUERY);
		cards_data_t tmp;
		tmp.card_id = parsed[2];
		tmp.valid_until= 0;//time(NULL)+add_time*60;
		tmp.last_enter = 0;
		tmp.card_type = parsed[1];
		my_turnikets->add_card(tmp);
		return 0;
	} else {
		return -1;
	}

}

int hp_go::process_sim_data()
{
	int sec_all_status = 0;
	vector<string> valid_rooms;
	if(my_security != NULL || my_jablotron != NULL){
		vector<string> armed_sensor;
		if(my_security != NULL){
			vector<string> tmp = my_security->get_actors4armed_zones();
			armed_sensor.insert(armed_sensor.end(),tmp.begin(), tmp.end());
		}
		if(my_jablotron != NULL){
			valid_rooms = my_jablotron->get_armed_zones();
		}
		if(armed_sensor.size() == 0 && valid_rooms.size() == 0){
			my_simulation_try_counter = MAX_SIMULATION_TRIES+1;
			my_simulation_state = SIM_OFF;
			return -1;
		}
		for(unsigned int i=0; i<armed_sensor.size(); i++){
			int pin_pos = find_item(armed_sensor[i],"desc");
			if(pin_pos != -1){
				string room = my_pins[pin_pos]->get_location();
				bool add_room = true;
				for(unsigned int j=0; j<valid_rooms.size(); j++){
					if(valid_rooms[j] == room){
						add_room = false;
						break;
					}
				}
				if(add_room ){
					valid_rooms.push_back(room);
				}
			}
		}
	}

#ifdef SIMULATION
	if(my_security != NULL){
		sec_all_status = my_security->get_all_zone_status();
	}
	if(my_jablotron != NULL){
		sec_all_status = (int)my_jablotron->all_zones_armed();
	}
	//cout <<"valid rooms: " << valid_rooms << ", all status: " << sec_all_status << endl;
	for(int i=0; i<(int)my_sim_data->sim_data.size(); i++){
	//	cout << "ident: " << my_sim_data->sim_data[i].ident << " to value: " << my_sim_data->sim_data[i].to_value <<  " v case: " << my_sim_data->sim_data[i].hour <<":"<< my_sim_data->sim_data[i].min <<endl;
		int pin_pos = this->find_item(my_sim_data->sim_data[i].ident,"desc");
		if(pin_pos != -1){
			int function = my_pins[pin_pos]->get_int_type();
			bool valid_actor = false;
			for(unsigned int j=0; j<valid_rooms.size(); j++){
				if(my_pins[pin_pos]->get_location() == valid_rooms[j]){
					valid_actor = true;
				}
			}
			if((function == 0 || function == 1) && (valid_actor || sec_all_status)){
				hp_alarm_checker_t tmp;
				tmp.pin_pos = pin_pos;
				tmp.id = my_sim_data->sim_data[i].ident;
				tmp.send_value = my_sim_data->sim_data[i].to_value;
				tmp.hour = my_sim_data->sim_data[i].hour;
				tmp.min= my_sim_data->sim_data[i].min;
				my_sim_control_data.push_back(tmp);
			}
		}
	}
	my_sim_data->sim_data.clear();
	for(unsigned int i=0; i<my_sim_control_data.size(); i++){
		//cout <<my_sim_control_data[i].id << " to: " << my_sim_control_data[i].send_value <<", " << my_sim_control_data[i].hour << ":" << my_sim_control_data[i].min << endl;
	}
	if(my_sim_control_data.size() < 10 && my_simulation_try_counter < MAX_SIMULATION_TRIES){
		my_sim_control_data.clear();
		my_simulation_try_counter++;
		my_simulation_state = SIM_CALCULATION;
		my_sim_data->day_minus = my_simulation_try_counter;
		push_db_query("Spustam vypocet simulacie, query: select ident,status,DATE_FORMAT(cas,\"%H:%i\") from all_statuses where cas like concat('%',(select DATE_SUB(DATE_FORMAT(now(),\"%Y-%m-%d\"),INTERVAL "+patch::to_string(my_sim_data->day_minus)+" DAY)),'%') and ident NOT LIKE '%temp%' AND ident NOT LIKE '%pir%' AND ident != \"\";", DB_LOG_COM);
		my_sim_thread = new std::thread(*my_sec_handler);
		my_simulation_state = SIM_CALCULATION;
	} else {
		if(my_simulation_try_counter < MAX_SIMULATION_TRIES){
			my_simulation_state = SIM_ON;
		} else {
			my_simulation_state = SIM_OFF;
		}
	}
#endif

	return 0;
}

int hp_go::process_gui_comm(string mess, hp_button_scenario *but_scen)
{
	vector<hp_scen_action_t> actions;
	vector<string> services;
	vector<string> shutters;
	if(but_scen != NULL){
		actions = but_scen->get_actions();
		shutters = but_scen->get_shutters();
	} else {
		if(my_cin_scen != NULL){
			if(mess.find(my_cin_scen->get_scen_ident()) != std::string::npos){
				actions = my_cin_scen->get_actions();
				services = my_cin_scen->get_services();
				shutters = my_cin_scen->get_shutters();
	
			}
		}
		if(my_cout_scen != NULL){
			if(mess.find(my_cout_scen->get_scen_ident()) != std::string::npos){
				actions = my_cout_scen->get_actions();
				services = my_cout_scen->get_services();
				shutters = my_cout_scen->get_shutters();
			}
		}
	}
	int delay_counter = 0;
	if(actions.size() != 0){
		for(unsigned int i=0; i<actions.size(); i++){
			int pin_pos = find_item(actions[i].ident,"desc");
			if(pin_pos != -1){
				this->add_message(pin_pos, actions[i].to_value,true, (delay_counter++)*20,atoi(actions[i].timer_time.c_str()));
			} else {
				if(actions[i].ident.find("comm_c") == std::string::npos){
					process_gui_mess("GUI_internalAdvanced_"+actions[i].ident+"_"+actions[i].to_value);
				}
			}
		}
	}
	if(services.size() > 0){
		for(unsigned int i=0; i<services.size(); i++){
			if(services[i].find("temp") != std::string::npos){
				process_gui_temp_mess(services[i]);
			} else if(services[i].find("secur") != std::string::npos){
				process_gui_secur_mess(services[i]);
			}
		}
	}
	if(shutters.size() > 0){
		erase_delayed_event();
		for(unsigned int i=0; i<shutters.size(); i++){
			push_delayed_event(prepare_event(EVENT_SHUTTERS, i, shutters[i]));
		}
	}
	return 0;
}

int hp_go::process_gui_wat_mess(string mess)
{
	if(my_watering != NULL){
		vector<string> parsed = this->parse_response(mess);
		if(parsed.size() > 1){
			my_watering->update_zone(parsed);
			if(parsed[1] == "time"){
				if(parsed.size() > 4){
					if(parsed[3] == "1"){
						push_db_query("UPDATE watering set time_1= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "2"){
						push_db_query("UPDATE watering set time_2= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "3"){
						push_db_query("UPDATE watering set time_3= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "4"){
						push_db_query("UPDATE watering set time_4= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					}
				}
			} else if(parsed[1] == "auto"){
				if(parsed.size() > 4){
					if(parsed[3] == "time1"){
						push_db_query("UPDATE watering set time1ON= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "time2"){
						push_db_query("UPDATE watering set time2ON= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "time3"){
						push_db_query("UPDATE watering set time3ON= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "time4"){
						push_db_query("UPDATE watering set time4ON= '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					}
				}
			} else if(parsed[1] == "period"){
				if(parsed.size() > 4){
					if(parsed[3] == "monday"){
						push_db_query("UPDATE watering set monday = '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "tuesday"){
						push_db_query("UPDATE watering set tuesday = '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "wednesday"){
						push_db_query("UPDATE watering set wednesday = '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "thursday"){
						push_db_query("UPDATE watering set thursday ='"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "friday"){
						push_db_query("UPDATE watering set friday = '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "sathurday"){
						push_db_query("UPDATE watering set sathurday = '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					} else if(parsed[3] == "sunday"){
						push_db_query("UPDATE watering set sunday = '"+parsed[4]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
					}
				}
			} else if(parsed[1] == "hold"){
				if(parsed.size() > 3){
					push_db_query("UPDATE watering set timer = '"+parsed[3]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
				}
			} else if(parsed[1] == "allauto"){
				if(parsed.size() > 3){
					push_db_query("UPDATE watering set auto = '"+parsed[3]+"' where ident = '"+parsed[2]+"'",DB_QUERY);
				}
			} else {
				if(parsed.size() > 2){
					hp_watering_zone *zone = my_watering->find_wat_zone(parsed[1]);
					if(zone != NULL){
						if(zone->get_zone_type() == ZONE_SINGLE){
							vector<string> out = zone->get_actors();
							int zone_timer = zone->get_time_period();
							if(zone_timer != 0){
								for(unsigned int i=0; i<out.size(); i++){
									int pin_pos = find_item(out[i], "desc");
									if(pin_pos != -1){
										this->add_message(pin_pos, parsed[2], false, 0, parsed[2]=="1"?zone_timer*60:0, parsed[2]=="0"?true:false, false, true);
									}
								}
							}
						} else {
							vector<string> zones = my_watering->get_zones_ident();
							if(parsed[2] == "1"){
								string ev_var = "";
								for(uint16_t i=0; i<zones.size(); i++){
									ev_var += zones[i] + ";";
								}
								if(ev_var != ""){
									push_delayed_event(prepare_event(EVENT_WAT_ALL_ZONE,0,ev_var));
									push_db_query("UPDATE watering set running = 1 where ident = '"+my_watering->get_all_ident()+"';",DB_QUERY);
									this->push_ws_data("watering", my_watering->get_all_ident(), "1");
								}
							} else {
								for(uint16_t i=0; i<zones.size(); i++){
									turn_off_wat_zone(zones[i]);
								}
								erase_delayed_event(EVENT_WAT_ALL_ZONE);
								push_db_query("UPDATE watering set running = 0 where ident = '"+my_watering->get_all_ident()+"';",DB_QUERY);
								this->push_ws_data("watering", my_watering->get_all_ident(), "0");
							}
						}
					}
				}
			}
		}
		fill_shared_memory();
	}
	return 0;
}
void hp_go::turn_off_wat_zone(string zone_ident)
{
	hp_watering_zone *zone_off = my_watering->find_wat_zone(zone_ident);
	if(zone_off != NULL){
		if(zone_off->is_running()){
			vector<string> out = zone_off->get_actors();
			for(unsigned int i=0; i<out.size(); i++){
				int pin_pos = find_item(out[i], "desc");
				if(pin_pos != -1){
					this->add_message(pin_pos,"0", false, 0, 0, true);
				}
			}
		}
	}
}

int hp_go::process_gui_air_mess(string mess)
{
	return 0;
}

int hp_go::process_gui_temp_mess(string mess)
{
	if(my2heating != NULL){
		vector<string> parsed = this->parse_response(mess);
		if(parsed.size() >= 3){
			if(parsed[2] == "mode"){
				if(parsed[1] == my2heating->get_all_ident()) {
					string others = "1";
					if(parsed[3] == "0"){
						others = "-1";
					}
					if(parsed[3] == "1"){
						others = "1";
					}
					if(parsed[3] == "2"){
						others = "-2";
					}
					if(parsed[3] == "3"){
						others = "-3";
					}
					if(parsed[3] == "4"){
						others = "1";
					}
					if(others != "1"){
						check_heating_zones(patch::string2int(parsed[3]));
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), MODE = '"+parsed[3]+"' where ID= '"+parsed[1]+"'",DB_QUERY); 
						this->push_ws_data("heatingMode",parsed[1],parsed[3]);
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = "+others,DB_QUERY); 
						this->push_ws_data("heatingQueryTemp","allZones",others); // update all zones
					} else {
						check_heating_zones(patch::string2int(parsed[3]),0,true);
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = "+patch::to_string(my2heating->get_all_query_temp())+", MODE = '"+parsed[3]+"' where ID= '"+parsed[1]+"'",DB_QUERY); 
						this->push_ws_data("heatingQueryTemp",parsed[1],patch::to_string(my2heating->get_all_query_temp()));
						this->push_ws_data("heatingMode",parsed[1],parsed[3]);
					}
				}
			} else if (parsed[1] == "thermostat") {
				my2heating->process_thermostat_mess(parsed);	
			}else if(parsed[1] == my2heating->get_eco_ident()) {
				my2heating->set_eco_temp(patch::string2float(parsed[2]));
				push_db_query("UPDATE STATUSES set STATUS = '"+parsed[2]+"' where ITEM = '"+parsed[1]+"'"); 
				check_heating_zones();
			} else  if(parsed[2] == "cooler") {
				if(parsed[1] == my2heating->get_all_ident()) {
					if(parsed[3] == "0"){
						check_heating_zones(HEATING_OFF);
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), MODE = 0  where ID= '"+parsed[1]+"'",DB_QUERY); 
						this->push_ws_data("heatingMode",parsed[1],"0");
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = -1",DB_QUERY); 
						this->push_ws_data("heatingQueryTemp","allZones","-1");
					} else {
						check_heating_zones(HEATING_COOLING,0, true);
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = "+patch::to_string(my2heating->get_all_query_temp())+", MODE = 4 where ID= '"+parsed[1]+"'",DB_QUERY); 
						this->push_ws_data("heatingMode",parsed[1],"4");
						this->push_ws_data("heatingQueryTemp",parsed[1],patch::to_string(my2heating->get_all_query_temp()));
					}
				}
			} else {
				if(parsed[1] == my2heating->get_all_ident()){
					push_delayed_event(prepare_event(EVENT_HEATING_CHECK, 1, parsed[2]));
					push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = '"+parsed[2]+"'",DB_QUERY); 
					this->push_ws_data("heatingQueryTemp","allZones",parsed[2]);
				} else {
					hp_heating_section *section= my2heating->find_section(parsed[1], "ident");
					if(section != NULL){
						string new_temp = string(parsed[2]);
						if(parsed[2][0] == 'M'){
							if(parsed[2][1] == '0'){
								section->set_query_temp(my2heating->get_def_tempering());
								new_temp = patch::to_string(my2heating->get_def_tempering());
							} else {
								section->set_query_temp(my2heating->get_def_temp());
								new_temp = patch::to_string(my2heating->get_def_temp());
							}
						} else {
							section->set_query_temp(patch::string2float(parsed[2]));
						}
						push_delayed_event(prepare_event(EVENT_MESS_HEATING, 1,section->get_id()));
						push_db_query("UPDATE HEATING_ZONES set MODIFICATION = NOW(), SET_TEMPERATURE = '"+new_temp+"' where ID= '"+parsed[1]+"'",DB_QUERY); 
						this->push_ws_data("heatingQueryTemp",parsed[1],new_temp);
					} 
				}
			}
		}
	}
	return 0;
}

int hp_go::start_simulation(int state)
{
	if(my_security != NULL || my_jablotron != NULL){
#ifdef SIMULATION
		int control = 0;
		int check_zones = true;
		if(my_security != NULL){
			if(my_security->is_sim_enabled() && my_security->get_simulation_value()){
				control = 1;
			}
			check_zones = my_security->has_armed_zone();
		}
		if(my_jablotron != NULL){
			if(my_jablotron->is_sim_enabled() && my_jablotron->get_simulation_value()){
				control = 1;
			}
			check_zones = my_jablotron->has_armed_zone();
		}
		if(state == 1 && control && check_zones){
			if(my_sim_thread == NULL){
				my_sim_data->sim_data.clear();
				my_sim_control_data.clear();
				my_simulation_state = SIM_CALCULATION;
				my_simulation_try_counter =0;
				my_sim_data->day_minus = 7;
				my_sim_data->finished = false;
				push_db_query("Spustam vypocet simulacie, query: select ident,status,DATE_FORMAT(cas,\"%H:%i\") from all_statuses where cas like concat('%',(select DATE_SUB(DATE_FORMAT(now(),\"%Y-%m-%d\"),INTERVAL "+patch::to_string(my_sim_data->day_minus)+" DAY)),'%') and ident NOT LIKE '%temp%' AND ident NOT LIKE '%pir%' AND ident != \"\";", DB_LOG_COM);
				my_sim_thread = new std::thread(*my_sec_handler);
			} else {
				push_db_query("Nespustam vypocet simulacie, lebo vypocet uz bezi... ", DB_LOG_COM);
				return -1;
			}
		} else {
			push_db_query("Vypinam simulaciu pritomnosti",DB_LOG_COM);
			if(my_sim_thread != NULL){
				my_sim_data->finished = false;
				my_sim_thread->join();						
				delete my_sim_thread;
				my_sim_thread = NULL;
			}
			my_sim_control_data.clear();
			my_simulation_state = SIM_OFF;
		}
#endif
	}
	return 0;
}

int hp_go::process_gui_secur_mess(string mess)
{
	string query;
//	secur_garaz_1
	if(my_jablotron != NULL){
		vector<string> parsed = this->parse_response(mess);
		if(parsed.size() >= 3){
			hp_jablotron_zone *zone = my_jablotron->find_zone(parsed[1]);
			if(zone != NULL){
				if(parsed[2] == "1"){
					if(zone->get_zone_status() == 0){
						if(zone->has_impulz_control()){
							int pos = find_item(zone->get_on_ident(), "desc");
							if(pos != -1){
								this->add_message(pos, "1",true,0,0,false,true,true);
							}
						} else {
							int pos = find_item(zone->get_actor_ident(), "desc");
							if(pos != -1){
								this->add_message(pos, "0",true,0,0,false,true,true);
							}
						}
					}
				} else if(parsed[2] == "0"){
					if(zone->get_zone_status() != 0){
						if(zone->has_impulz_control()){
							int pos = find_item(zone->get_off_ident(), "desc");
							if(pos != -1){
								this->add_message(pos, "1",true,0,0,false,true,true);
							}
						} else {
							int pos = find_item(zone->get_actor_ident(), "desc");
							if(pos != -1){
								this->add_message(pos, "1",true,0,0,false,true,true);
							}
						}
					}
				}
			}
		}
	}
	if(my_security != NULL){
		vector<string> parsed = this->parse_response(mess);
		if(parsed.size() >= 3){
			vector<string> zone_idents;
			if(parsed[1] == my_security->get_all_ident()){
				zone_idents = my_security->get_zones_ident();
			} else {
				zone_idents.push_back(parsed[1]);
			}
			bool check_all_ident = false;
			for(unsigned int i=0; i< zone_idents.size(); i++){
				hp_security_zone *zone  = NULL; 
				zone = my_security->find_zone(zone_idents[i]);
				if(zone != NULL){
					if(parsed[2] == "1"){
						if(zone->get_sec_status() == 0){
							my_running_zone.push_back(zone);
							query = "UPDATE SECURITY_ZONES set ALARM_COUNTDOWN = "+ patch::to_string(zone->init_countdown()) + " where ID = '"+zone->get_id()+"'";
							this->push_ws_data("securityCountDown",zone->get_id(), patch::to_string(zone->init_countdown()));
							push_db_query(query);
						}
					} else if (parsed[2] == "0"){
						if(zone->get_sec_status() == 2){
							vector<string> outputs = zone->get_actors();
							for(vector<string>::iterator it =outputs.begin(); it != outputs.end(); ++it){
								int item_pos = find_item(*it, "desc");
								if(item_pos != -1){
									this->add_message(item_pos, "0");
								} else {
									cout << "Nenasiel som aktora pre zonu: " << zone->get_id() << endl;
								}
							}
						}
						zone->set_sec_status(0);
						query = "UPDATE SECURITY_ZONES set ALARM_COUNTDOWN = -1, STATUS = 0 where ID = '"+zone->get_id()+"'";
						this->push_ws_data("security",zone->get_id(),"0");
						this->push_ws_data("securityCountDown",zone->get_id(),"-1");
						erase_running_secur(zone->get_id());
						push_db_query(query, DB_QUERY);
						check_all_ident = true;
						start_simulation(0);
						if(my_notification != NULL){
							try {
								if(my_rooms.find(my_running_zone[i]->get_id()) != my_rooms.end()){
									my_notification->push_security_notification(my_rooms.at(zone->get_id()),0);
								}
							} catch (const std::exception& e) {
								push_db_query("A standard exception was caught, with message '" + string(e.what()) + "', file: " + __FILE__ +", line: " + patch::to_string(__LINE__), DB_LOG_COM);
							}
						}
					} else {
						cout <<"Nespravna hodnota pre security " << endl;
					}
					start_simulation(1);
				} else {
					cout << "Nenasiel som secur zone: " << parsed[1] << endl;
				}
			}
			if(check_all_ident > 0){
				int all_zone_status = my_security->get_all_zone_status();
				push_db_query("UPDATE SECURITY_ZONES SET STATUS = "+patch::to_string(all_zone_status)+" where ID = '"+my_security->get_all_ident()+"'", DB_QUERY);
				this->push_ws_data("security",my_security->get_all_ident(),patch::to_string(all_zone_status));
			}
			check_sec_singnalization();
		}
	}
		
	return 0;
}

void hp_go::erase_running_secur(string id)
{
	for(unsigned int i=0; i<my_running_zone.size(); i++){
		if(my_running_zone[i]->get_id() == id){
			my_running_zone.erase(my_running_zone.begin() + i);
			break;
		}
	}
}

int hp_go::process_gui_scen_mess(string mess)
{
	unsigned int i;
	// scen_new_hílš_svetloTerasa_0
	// scen_new_sadad_svetloTerasa_0-svetloMisko_0-brana_0
	vector<string> parsed = this->parse_response(mess);
	if(parsed.size() > 2){
		string query = "";
		if(parsed[1] == "new"){
			query = "INSERT INTO SCENARIA (label) VALUE ('"+parsed[2]+"');";
			push_db_query(query,DB_QUERY);
			query = "INSERT INTO SCENARIA_CONFIG (id_scenaria, ident, to_value) VALUES ";
			for(i=3;i<parsed.size(); i++){
				if(i==3){
					query += "((SELECT id_scenaria FROM SCENARIA ORDER BY id_scenaria DESC LIMIT 1), '"+parsed[i]+"', ";
				} else if(i == parsed.size()-1) {
					query += parsed[i]+") ";
				} else {
					query += parsed[i].substr(0,1) + "), ((SELECT id_scenaria FROM SCENARIA ORDER BY id_scenaria DESC LIMIT 1), '"+parsed[i].substr(2) +"', ";
				}
			}
			push_db_query(query,DB_QUERY);
			query = "INSERT INTO AUTOMATABLE_DEVICES (NAME, LABEL, TYPE) VALUES ('"+parsed[2]+"','"+parsed[2]+"',99)";
			push_db_query(query,DB_QUERY);
		} else if(parsed[1] == "delete"){
			if(parsed.size() > 2){
				cout <<"Scenario name to delete: " << parsed[2] << endl;
				query = "DELETE from AUTOMATED_DEVICES where NAME= (SELECT label from SCENARIA where id_scenaria = "+parsed[2]+") AND COMMAND = 99";
				push_db_query(query,DB_QUERY);
				query = "DELETE from AUTOMATABLE_DEVICES where NAME= (SELECT label from SCENARIA where id_scenaria = "+parsed[2]+")";
				push_db_query(query,DB_QUERY);
			}
			query = "DELETE from SCENARIA where id_scenaria = "+parsed[2];
			push_db_query(query,DB_QUERY);
			push_db_query("DELETE FROM SCENARIA_CONFIG where id_scenaria = "+parsed[2],DB_QUERY);
		} else if (parsed[1] == "update"){
			push_db_query("DELETE FROM SCENARIA_CONFIG where id_scenaria = "+parsed[2],DB_QUERY);
			query = "INSERT INTO SCENARIA_CONFIG (id_scenaria, ident, to_value) VALUES ";
			for(i=3;i<parsed.size(); i++){
				if(i==3){
					query += "("+parsed[2]+", '"+parsed[i]+"', ";
				} else if(i == parsed.size()-1) {
					query += parsed[i]+") ";
				} else {
					query += parsed[i].substr(0,1) + "), ("+parsed[2]+", '"+parsed[i].substr(2) +"', ";
				}
			}
			push_db_query(query,DB_QUERY);
		} else {
			map<string,string> scen_items = get_scen_data(parsed[1]);
			for(map<string,string>::iterator it = scen_items.begin(); it != scen_items.end(); ++it){
				int pin_pos = this->find_item(it->first, "desc");
				if(pin_pos != -1){
					if(my_pins[pin_pos]->get_function() == "shutter"){
						//GUI_60_shutt_shuttHost_allup_1
						string gui_mess = "shutt_"+it->first+"_";
						if(it->second == "2"){
							gui_mess += "up_7";
						} else if(it->second == "3"){
							gui_mess += "down_7";
						} else if(it->second == "4"){
							gui_mess += "allup_1";
						} else if(it->second == "5"){
							gui_mess += "alldown_1";
						}
						//process_gui_shutt_mess(gui_mess);
						push_delayed_event(prepare_event(EVENT_SHUTTERS, std::distance(scen_items.begin(), it), gui_mess));
					} else{
						/*
						hp_mess2send_t mess = my_pins[pin_pos]->create_send_mess(it->second);
						mess.debug = "Procesing scenare mess: 1031";
						add_mess2sender(mess, pin_pos);
						*/
						this->add_message(pin_pos, it->second);
					}
				}
			}
		}
	}
	return 0;
}
map<string,string> hp_go::get_scen_data(string scen_id)
{
	string query="";
	map<string,string> res;
	query = "Select ident, to_value from SCENARIA_CONFIG where id_scenaria = "+scen_id;

	if(query != ""){
		MYSQL *conn;
		MYSQL_RES *result;
		MYSQL_ROW row;
		int query_state;

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
			return res;
		} else {
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL){
							res.insert(std::pair<string,string>(row[0], row[1]));
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
#ifndef AXON_SERVER
				mysql_library_end();
#endif
				conn = NULL;
			}
		}
	}
	return res;
}

void hp_go::erase_delayed_event(int event_type)
{
	/*
	CHECK();
	//auto itr = std::remove(my_delayed_events.begin(),my_delayed_events.end(),event_type);
	cout <<"Del size : " << my_delayed_events.size() << endl;
	auto itr = std::remove_if(my_delayed_events.begin(),my_delayed_events.end(),[&event_type](delayed_event_t& p){return p.event_type == event_type;});
	my_delayed_events.erase(itr, my_delayed_events.end());
	cout <<"Del size : " << my_delayed_events.size() << endl;

	for(auto itr = my_delayed_events.begin(); itr != my_delayed_events.end();){
		if(itr->event_type == event_type){
			itr = my_delayed_events.erase(itr);
		} else {
			itr++;
		}
	}
	*/
	for(uint16_t i=0; i<my_delayed_events.size(); i++){
		if(my_delayed_events[i].event_type == event_type){
			if(my_delayed_events.begin()+i != my_delayed_events.end()-1){
				my_delayed_events.erase(my_delayed_events.begin()+i);
				i--;
			} else {
				my_delayed_events.erase(my_delayed_events.begin()+i);
			}
		}
	}
}

int hp_go::process_gui_shutt_mess(string mess)
{
	unsigned int i;
	vector<string> parsed = this->parse_response(mess);
	//GUI_60_shutt_shuttHost_allup_1
	//GUI_45_shutt_shuttHost_alldown_1
	//GUI_35_shutt_shuttHost_down_1
	//shutt_shuttHost_down_0
	//shutt_blindLivingRoom1-blindLivingRoom2-blindLivingRoom3_down_0
	//GUI_33_shutt_shuttHost_up_1
	//GUI_93_shutt_shuttHost_up_0
	if(my_shutters == NULL){ 
		return -1;
	}

	if(parsed.size() == 4){
		if(parsed[1] == my_shutters->get_all_ident()){
			vector<hp_shutter *> multi_shutter = my_shutters->move_all_shutters();
			if(multi_shutter.size() != 0 ){
				erase_delayed_event();
				for(unsigned int i=0; i<multi_shutter.size(); i++){
					string gui_mess = "shutt_"+multi_shutter[i]->get_gui_ident()+"_"+parsed[2]+"_1";
					push_delayed_event(prepare_event(EVENT_SHUTTERS, i, gui_mess));
				}
			}
			return 0;
		}
		vector<string> idents = this->parse_response(parsed[1], "-");
		for(unsigned int j= 0; j<idents.size(); j++){
			hp_shutter *rule_shutter= NULL;
			rule_shutter = my_shutters->find_shutter(idents[j]);
			if(rule_shutter != NULL){
				int shut_pos = find_item(idents[j],"desc");
				if(parsed[2] == my_shutters->get_all_up_ident()){
					rule_shutter->set_move_direction(SHUT_UP);
					this->add_message(rule_shutter->get_pin_on_off(),"1",false,0,rule_shutter->get_my_timer());
					this->add_message(rule_shutter->get_pin_dir(),"up");
				}
				if(parsed[2] == my_shutters->get_all_down_ident()){
					rule_shutter->set_move_direction(SHUT_DOWN);
					this->add_message(rule_shutter->get_pin_on_off(),"1",false,0,rule_shutter->get_my_timer());
					this->add_message(rule_shutter->get_pin_dir(),"down");
				}
				if(parsed[2] == my_shutters->get_up_ident()){
					if(parsed[3] == "1"){
						if(SHUTTER_CONTROL == 0){
							int move_steps = rule_shutter->get_mevement_steps("up");
							if( move_steps != 0){
								rule_shutter->set_move_direction(SHUT_UP);
								hp_mess2send_t mess = my_pins[rule_shutter->get_pin_on_off()]->create_send_mess(patch::to_string(SHUTTER_STEP_TIME));
								my_sender->delete_timer_mess(mess.mess, mess.xbee_id);
								my_sender->add_message(mess);
								my_repeat_mess.push_back(hp_repeat_mess(mess, SHUTTER_STEP_TIME*SHUTTER_STEP_INDEX, "up",shut_pos, move_steps-1));
					
								mess = my_pins[rule_shutter->get_pin_dir()]->create_send_mess("up");
								my_sender->add_message(mess);
								my_repeat_mess.push_back(hp_repeat_mess(mess, SHUTTER_STEP_TIME*SHUTTER_STEP_INDEX, "up", -1, move_steps-1));
							}
						} else {
							this->add_message(rule_shutter->get_pin_on_off(),"1",false,0,0,true);
							this->add_message(rule_shutter->get_pin_dir(),"up");
						}

					} else if(parsed[3] == "0") {
						if(SHUTTER_CONTROL == 0){
							hp_mess2send_t mess = my_pins[rule_shutter->get_pin_on_off()]->create_send_mess(patch::to_string(SHUTTER_STEP_TIME));
							hp_mess2send_t mess1 = my_pins[rule_shutter->get_pin_dir()]->create_send_mess("up");
							for(i=0; i<my_repeat_mess.size(); i++){
								if(mess.xbee_id == my_repeat_mess[i].get_xbee_id() && mess.mess == my_repeat_mess[i].get_mess() ){
									if(my_repeat_mess[i].get_pin_pos() != -1 && my_repeat_mess[i].get_token() != 10){
										my_shutters->setup_shut_tilt(my_pins[my_repeat_mess[i].get_pin_pos()]->get_gui_desc(), my_repeat_mess[i].get_token());
									}
									my_repeat_mess.erase(my_repeat_mess.begin()+i);
									i--;
									continue;
								}
								if(mess1.xbee_id == my_repeat_mess[i].get_xbee_id() && mess1.mess == my_repeat_mess[i].get_mess() ){
									my_repeat_mess.erase(my_repeat_mess.begin()+i);
									i--;
									continue;
								}
							}
						} else if(SHUTTER_CONTROL == 1){
							this->add_message(rule_shutter->get_pin_on_off(),"0",false,0,0,true);
						}
					} else {
						int multiply = patch::string2int(parsed[3]);
						if(multiply != -1){
							this->add_message(rule_shutter->get_pin_on_off(),patch::to_string(multiply*SHUTTER_STEP_TIME),false,0,0,true);
							this->add_message(rule_shutter->get_pin_dir(),"up");
						}
					}
				}
				if(parsed[2] == my_shutters->get_down_ident()){
					int added_time = 10;
					if(parsed[3] == "1"){
						if(SHUTTER_CONTROL == 0){
							int move_steps = rule_shutter->get_mevement_steps("down");
							if( move_steps != 0){
								rule_shutter->set_move_direction(SHUT_DOWN);
								hp_mess2send_t mess = my_pins[rule_shutter->get_pin_on_off()]->create_send_mess(patch::to_string(SHUTTER_STEP_TIME+added_time));
								my_sender->delete_timer_mess(mess.mess, mess.xbee_id);
								my_sender->add_message(mess);
								my_repeat_mess.push_back(hp_repeat_mess(mess, SHUTTER_STEP_TIME*SHUTTER_STEP_INDEX, "down",shut_pos, move_steps-1));
					
								mess = my_pins[rule_shutter->get_pin_dir()]->create_send_mess("down");
								my_sender->add_message(mess);
								my_repeat_mess.push_back(hp_repeat_mess(mess, SHUTTER_STEP_TIME*SHUTTER_STEP_INDEX, "down",-1, move_steps-1));
							}
						} else if(SHUTTER_CONTROL == 1){
							this->add_message(rule_shutter->get_pin_on_off(),"1",false,0,0,true);
							this->add_message(rule_shutter->get_pin_dir(),"down");
						}
					} else if(parsed[3] == "0") {
						if(SHUTTER_CONTROL == 0){
							hp_mess2send_t mess = my_pins[rule_shutter->get_pin_on_off()]->create_send_mess(patch::to_string(SHUTTER_STEP_TIME+added_time));
							hp_mess2send_t mess1 = my_pins[rule_shutter->get_pin_dir()]->create_send_mess("down");
							for(i=0; i<my_repeat_mess.size(); i++){
								if(mess.xbee_id == my_repeat_mess[i].get_xbee_id() && mess.mess == my_repeat_mess[i].get_mess() ){
									if(my_repeat_mess[i].get_pin_pos() != -1 && my_repeat_mess[i].get_token() != 10){
										my_shutters->setup_shut_tilt(my_pins[my_repeat_mess[i].get_pin_pos()]->get_gui_desc(), my_repeat_mess[i].get_token());
									}
									my_repeat_mess.erase(my_repeat_mess.begin()+i);
									i--;
									continue;
								}
								if(mess1.xbee_id == my_repeat_mess[i].get_xbee_id() && mess1.mess == my_repeat_mess[i].get_mess() ){
									my_repeat_mess.erase(my_repeat_mess.begin()+i);
									i--;
									continue;
								}
							}
						} else if(SHUTTER_CONTROL == 1){
							this->add_message(rule_shutter->get_pin_on_off(),"0",false,0,0,true);
						}
					} else {
						int multiply = patch::string2int(parsed[3]);
						if(multiply != -1){
							this->add_message(rule_shutter->get_pin_on_off(),patch::to_string(multiply*SHUTTER_STEP_TIME),false,0,0,true);
							this->add_message(rule_shutter->get_pin_dir(),"down");
						}
					}
				}
			} else {
				push_db_query("Nenasiel som pravidlo pre zaluziu: " + idents[j], DB_LOG_COM);
			}
		}
	} else {
		push_db_query("Nespravna shutt sprava: " + mess, DB_LOG_COM);
	}
	
	return 0;
}

int hp_go::process_gui_alarm_mess(string mess)
{
	// GUI_47_alarm_lightObyvacka2_20:49_0
	
	unsigned int i;
	vector<string> parsed = this->parse_response(mess);
	if(mess.find("alarm_update_") != std::string::npos){
		if(parsed.size() > 5){
			push_db_query("UPDATE AUTOMATED_DEVICES set TIME = '"+parsed[4]+"', COMMAND = "+parsed[5]+" where ID = " +parsed[2],DB_QUERY);
			for(i=0; i<my_alarm_checker.size(); i++){
				if(my_alarm_checker[i].id == parsed[2]){
					my_alarm_checker[i].send_value = parsed[5];
					string time_str = parsed[4];
					my_alarm_checker[i].hour= atoi(time_str.substr(0, time_str.find(":")).c_str());
					time_str = time_str.substr(time_str.find(":")+1);
					my_alarm_checker[i].min= atoi(time_str.substr(0, time_str.find(":")).c_str());
					break;
				}
			}
		}
	} else if(mess.find("alarm_delete_") != std::string::npos) {
		if(parsed.size() == 3){
			push_db_query("DELETE from AUTOMATED_DEVICES where ID = " +parsed[2],DB_QUERY);
			for(i=0; i<my_alarm_checker.size(); i++){
				if(my_alarm_checker[i].id == parsed[2]){
					my_alarm_checker.erase(my_alarm_checker.begin()+i);
					break;
				}
			}
		}
	} else {
		if(parsed.size() == 4){
			if(parsed[1].length() > 0){
				//@@TODO@@
			//	if(parsed[1][0] == '9' && parsed[1][1] == '9') {
					if(parsed[1] == "s"){
						cout <<  parsed[1] << ", " << parsed[2] << endl;
						delayed_event_t event;
						event.event_type = EVENT_UPDATE_ALARMS;
						event.run_time = time(NULL) + 2;
						push_delayed_event(event);
						string query = "INSERT INTO AUTOMATED_DEVICES (NAME, COMMAND, TIME, LABEL) VALUES ('"+parsed[1]+"',99,'"+parsed[2]+"', '"+parsed[1]+"');";
						push_db_query(query,DB_QUERY);

					}
			//	}
			}
			int item_pos = find_item(parsed[1], "desc");
			if(item_pos != -1 ){
				hp_alarm_checker_t tmp;
				tmp.pin_pos = find_item(parsed[1],"desc");
				if(tmp.pin_pos != -1){
					delayed_event_t event;
					event.event_type = EVENT_UPDATE_ALARMS;
					event.run_time = time(NULL) + 2;
					push_delayed_event(event);
					string query = "INSERT INTO AUTOMATED_DEVICES (NAME, COMMAND, TIME, LABEL) VALUES ('"+parsed[1]+"', "+parsed[3]+",'"+parsed[2]+"', '"+my_pins[item_pos]->get_desc_2()+"');";
					push_db_query(query,DB_QUERY);
				}
			} else {
				// !!! poslat mess na websocket ze chyba 
			}
		}
	}
	fill_shared_memory();

	return 0;
}

void hp_go::restart_uc(int hbx_pos)
{
	if(my_hbxs[hbx_pos-1]->restart_enabled()){
		hp_mess2send_t data;
		data.xbee_id = patch::to_string(hbx_pos);
		data.uc_mess_type = "0";
		data.mess = "D0_4";
		data.service_type = "HBX";
		data.debug = "restart hbx, 1283 - D0_4";
		this->add_message(data);
		data.mess = "D0_5";
		data.debug = "restart hbx, 1283 - D0_5";
		this->add_message(data,0,0,100);
		my_hbxs[hbx_pos-1]->set_restart_type(RESTART_NACK);
		setup_hbx_outputs(hbx_pos);
	}
}

void hp_go::setup_hbx_outputs(int hbx_pos)
{
	bool push_query = false;
	string query = "UPDATE STATUSES set STATUS = 0 where ";
	for(unsigned int i=0; i<my_pins.size(); i++){
		if(my_pins[i]->get_hbx_pos() == hbx_pos){
			string funct = my_pins[i]->get_function();
			if(funct == "status_light" || funct == "pwm" || funct == "socket" || funct == "heater" || funct == "cooler"|| funct == "sprinkler"|| funct == "shutter"){
				query += " ITEM = '"+my_pins[i]->get_desc()+"' OR ";
				my_pins[i]->set_status("0");
			}
		}
	}
	if(push_query){
		query = query.substr(0, query.length() - 3);
		push_db_query(query);
	}
}

void hp_go::get_ouput_states()
{
	int counter = 0;
	if(!my_simulator){
		for(unsigned int i=0; i<my_pins.size(); i++){
			if(my_pins[i]->get_int_type() < 10 && my_pins[i]->get_int_type() != 5){
				if(my_pins[i]->get_hbx_pos() == 1){
					//cout <<"Zistujem stav pre prvok: " << my_pins[i]->get_desc() << endl;
					my_sender->add_message(prepare_mess(patch::to_string(my_pins[i]->get_hbx_pos()),"6",my_pins[i]->get_id(), "get_output_states"),0,0,100*counter);
					counter++;
				}
			}
		}
	}
}

void hp_go::send_diagnostic()
{
	if(!my_simulator){
		time_t rawtime;
		struct tm * timeinfo;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		push_db_query("Posielam diagnostiku v case: " + patch::to_string(timeinfo->tm_hour) + ":" + patch::to_string(timeinfo->tm_min) + ":" + patch::to_string(timeinfo->tm_sec),DB_LOG_COM);
		for(unsigned int i=0; i<my_hbxs.size(); i++){
			if(my_hbxs[i]->is_active()){
				this->add_message(patch::to_string(my_hbxs[i]->get_pos()),"8","X",0,0,i*40);
			}
		}
	}
}

void hp_go::send_U_message(int hbx_pos, int delay)
{
	if(!my_hbxs[hbx_pos-1]->was_u_sent()){
		this->add_message(patch::to_string(hbx_pos),"9","",0,5,delay);
		push_db_query("\t\tPosielam U-mess, hbx mac: " + my_hbxs[hbx_pos-1]->get_mac() + " hbx_pos: " + patch::to_string(hbx_pos) + " posledny restart typ: " +patch::to_string(my_hbxs[hbx_pos-1]->get_restart_type()), DB_LOG_COM);
		my_hbxs[hbx_pos-1]->u_sent(true);
		if(my_notification != NULL){
			if(my_system_start + 120 < time(NULL)){
				my_notification->push_notification(NN_ERROR,"Štart hbx: " + my_hbxs[hbx_pos-1]->get_name());
			}
		}
	}

}

void hp_go::send_U_messages()
{
	for(unsigned int i=0; i<my_hbxs.size(); i++){
		send_U_message(my_hbxs[i]->get_pos(), i*40);
	}
}

void hp_go::setup_blinds_time()
{
	vector<hp_shutter*> shutt = my_shutters->move_all_shutters();
	for(uint16_t i=0; i<shutt.size();i++){
		int pin_pos = shutt[i]->get_pin_dir();
		if(pin_pos != -1){
		push_db_query("Nastavujem cas pre zaluziu: " + my_pins[pin_pos]->get_desc_2() + " , HBX: " + patch::to_string(my_pins[pin_pos]->get_hbx_pos()) + ", pozicia zaluzie: " + patch::to_string(shutt[i]->get_pos()) + ", na cas: " + patch::to_string(shutt[i]->get_my_timer()), DB_LOG_COM);
			my_sender->add_message(prepare_mess(patch::to_string(my_pins[pin_pos]->get_hbx_pos()),"2",patch::to_string(shutt[i]->get_pos()-1)+patch::to_string(shutt[i]->get_my_timer()), "setup_blind_time"),0,0,20*i);
		}
	}
}

int hp_go::sync_hbx(int hbx_pos)
{
	my_hbxs[hbx_pos]->set_restart_type(-1);
	MYSQL *conn;
	conn = mysql_init(NULL);
	string query;

	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	map<string,string> items2sync;
	
	bool valid_query = false;
	query = "SELECT STATUS, ITEM from STATUSES where";
	for(unsigned int i=0; i<my_pins.size(); i++){
		if(my_pins[i]->get_hbx_pos()-1 == hbx_pos && my_pins[i]->get_io_type() == HP_PIN_OUTPUT){
			valid_query = true;
			query += " ITEM = '"+my_pins[i]->get_gui_desc()+"' OR";
		}
	}
	if(valid_query){
		push_db_query("Sync query: " + query , DB_LOG_COM);
		if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			mysql_close(conn);
#ifndef AXON_SERVER
			mysql_library_end();
#endif
			return -1;
		} else {
			query = query.substr(0, query.length() - 3);
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL && row[1] != NULL){
							items2sync.insert(std::make_pair<string,string>(row[1],row[0]));
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
#ifndef AXON_SERVER
				mysql_library_end();
#endif
				conn = NULL;
			}
		}
	}
	for(map<string,string>::iterator it= items2sync.begin(); it != items2sync.end(); ++it){
		int pin_pos = find_item(it->first,"desc");
		if(pin_pos != -1){
			int pin_type = my_pins[pin_pos]->get_int_type();
			if(pin_type == 0 ||pin_type == 1 ||pin_type == 4 ||pin_type == 7||pin_type == 8){
				this->add_message(pin_pos, it->second, true,20*std::distance(items2sync.begin(), it),0,false,true,true);//std::distance(items2sync.begin(),it)/2);
			}
		} else {
			cout <<"Nenasiel som : " << it->first << endl;
		}
	}
	if(valid_query){
		push_db_query("Sync finished.", DB_LOG_COM);
	}
	return 0;
}

int hp_go::find_item(string find, string property,int io_type,int hbx_pos, int start)
{
	//static long sum = 0, cc = 0;
//	cc++;
	int res = -1;
	if(property == "desc"){
		for(unsigned int i=start; i<my_pins.size(); i++){
			if(my_pins[i]->get_desc() == find){
				res = i;
				break;
			}
		}
		/*
		for(auto it=my_pins.begin(); it < my_pins.end(); ++it){
			if((*it)->get_desc() == find){
				res = std::distance(my_pins.begin(), it);
				break;
			}
		}
		for(auto i: my_pins){
			if( i->get_desc() == find){
				res = 1;
				break;
			}
		}
		*/

	} else if (property == "id"){
	 	for(unsigned int i=start; i<my_pins.size(); i++){
			if(my_pins[i]->get_id() == find && my_pins[i]->get_hbx_pos() == hbx_pos){
				res = i;
				break;
			}
		}
	}
	if(io_type != -1 && res != -1){
		if(io_type != my_pins[res]->get_io_type()){
			push_db_query("\t\t++++Chyba pri hladani pinu, pozadovany stav bol: "+ string(io_type == HP_PIN_INPUT?"vstup":"vystup") + " a pin: " + my_pins[res]->get_gui_desc() + " je " + string( my_pins[res]->get_io_type()==HP_PIN_INPUT?"vstup":"vystup") , DB_LOG_COM);
			res = -1;
		}
	}

	return res;
}

vector<string> hp_go::parse_response(string resp, string separator)
{
	vector<string> res;

	while(resp.find(separator) != std::string::npos){
		string tmp = resp.substr(0,resp.find(separator));
		res.push_back(tmp);
		resp = resp.substr(resp.find(separator)+1);
	}
	string tmp = boost::trim_right_copy(resp);
	if(tmp.length() > 0){
		res.push_back(tmp);
	}
	return res;
}

void hp_go::read_dali_sht() {
	if(m_dali_sht != NULL){
		m_dali_sht->mtx.lock();
		for(uint16_t i=0; i<m_dali_sht->mess.size(); i++){
			push_ws_data(m_dali_sht->mess[i].type,m_dali_sht->mess[i].ident,m_dali_sht->mess[i].status);
		}
		m_dali_sht->mess.clear();
		m_dali_sht->mtx.unlock();
	}
}

void hp_go::push_ws_data(string type, string ident, string value)
{
	if(my_use_ws){
		my_ws_data[my_ws_data.size()]["type"] = type;
		my_ws_data[my_ws_data.size()-1]["ident"] = ident;
		my_ws_data[my_ws_data.size()-1]["status"] = value;
	}
}

void hp_go::push_db_query(string query, int type, int log_level)
{
	//cout <<"type: " << type << "-" << query << endl;
	//if(DISPLAY_MESSAGES && type != DB_ALL_STATUSES && type != DB_TRANSACTION){
	if(type == DB_LOG_COM){
	//	cout << query << endl;
	}
	hp_db_queries_t tmp;
	tmp.query = query;
	tmp.type = type;
	tmp.log_level = log_level;

	if(!(type == DB_STATUSES_QUERY && my_has_hpp_web)){
		my_db_data.mtx.lock();
		my_db_data.queries.push_back(tmp);
		my_db_data.mtx.unlock();
	}
	if(my_has_hpp_web && type != DB_LOG_COM && type != DB_ALL_STATUSES && DB_TRANSACTION != type){
	//	cout <<"!!1 shm:" << query << endl;
		fill_shared_memory();
	}
}

void hp_go::push_all_statuses(string ident, string value)
{
	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );	
	
	std::stringstream s;
	s<<timeinfo->tm_year+1900<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_mday<<" "<<timeinfo->tm_hour<<":"<<timeinfo->tm_min<<":"<<timeinfo->tm_sec;
	string query = "('"+ident+"', '"+value+"', '"+s.str()+"'),";
	push_db_query(query,DB_ALL_STATUSES);
}


int hp_go::init_db()
{
	MYSQL *conn;
	conn = mysql_init(NULL);
	MYSQL_RES *result;
	MYSQL_ROW row;
	int query_state;

	if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),0,0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
#ifndef AXON_SERVER
		mysql_library_end();
#endif
		return -1;
	} else {
	execute_query("CREATE DATABASE IF NOT EXISTS "+my_mysql_db, conn);
	if(execute_query("USE "+my_mysql_db, conn) != 0){
		return -1;
	}
	if(execute_query("CREATE TABLE IF NOT EXISTS trusted_devices(id int NOT NULL AUTO_INCREMENT PRIMARY KEY,identification VARCHAR(200), notes VARCHAR(200)) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if(execute_query("CREATE TABLE IF NOT EXISTS STATUSES(ID int NOT NULL AUTO_INCREMENT PRIMARY KEY,ITEM VARCHAR(50), LABEL VARCHAR(100), STATUS double, MODIFICATION TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	bool alter_table = true;
	query_state = mysql_query(conn, ("SELECT NULL FROM INFORMATION_SCHEMA.COLUMNS WHERE table_schema = '"+my_mysql_db+"'   AND table_name = 'STATUSES' AND column_name = 'LABEL';").c_str());
	if(query_state == 0){
		result = mysql_store_result(conn);
		if(result->row_count > 0){
			alter_table = false;
		}
		if(result != NULL){
			mysql_free_result(result);
		}
	} else {
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
	}
	if(alter_table){
		execute_query("ALTER TABLE STATUSES ADD LABEL varchar(100) default ''");
		execute_query("DELETE from STATUSES");
	}
	if(execute_query("ALTER TABLE STATUSES MODIFY MODIFICATION TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP", conn) != 0){
		return -1;
	}

	if(execute_query("CREATE TABLE IF NOT EXISTS all_statuses(id_all_statuses int NOT NULL AUTO_INCREMENT PRIMARY KEY,ident VARCHAR(200), status double, cas datetime) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if(execute_query("ALTER TABLE all_statuses MODIFY ident VARCHAR(200)", conn) != 0){
		return -1;
	}
	if (execute_query("CREATE TABLE IF NOT EXISTS SCENARIABLE_DEVICES (id int NOT NULL AUTO_INCREMENT PRIMARY KEY, ident varchar(100) NOT NULL, label varchar(100) NOT NULL, type int NOT NULL) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if (execute_query("CREATE TABLE IF NOT EXISTS SCENARIA(id_scenaria int NOT NULL AUTO_INCREMENT PRIMARY KEY, label varchar(100) NOT NULL, cas TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if (execute_query("CREATE TABLE IF NOT EXISTS SCENARIA_CONFIG(ID int NOT NULL AUTO_INCREMENT PRIMARY KEY,id_scenaria int, ident varchar(100) NOT NULL, to_value int NOT NULL) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if (execute_query("CREATE TABLE IF NOT EXISTS AUTOMATABLE_DEVICES(ID int NOT NULL AUTO_INCREMENT PRIMARY KEY, NAME varchar(100) NOT NULL, LABEL varchar(100) NOT NULL, TYPE int NOT NULL) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if (execute_query("CREATE TABLE IF NOT EXISTS AUTOMATED_DEVICES(ID int NOT NULL AUTO_INCREMENT PRIMARY KEY, NAME varchar(100) NOT NULL, COMMAND INT NOT NULL, TIME time NOT NULL, LABEL varchar(100) NOT NULL) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if(execute_query("CREATE TABLE IF NOT EXISTS hbxs(id int NOT NULL AUTO_INCREMENT PRIMARY KEY, xbee_driver_id int, mac varchar(50), status int, modification datetime) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if(execute_query("CREATE TABLE IF NOT EXISTS hp_go(id int NOT NULL AUTO_INCREMENT PRIMARY KEY,cas TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, send_messages int, list_send varchar(8024), non_free_id int, list_id varchar(1024)) DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
		return -1;
	}
	if(my_security != NULL || my_jablotron != NULL){
		if(execute_query("CREATE TABLE IF NOT EXISTS SECURITY_ZONES (ID varchar(50) NOT NULL PRIMARY KEY,MODIFICATION datetime DEFAULT NULL, STATUS double NOT NULL, SENSOR int(11) NOT NULL,ACTOR tinyint(1) NOT NULL,ALARM_COUNTDOWN int(11) NOT NULL) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci",conn)){
			return -1;
		}
	}
	if(my2heating != NULL){
		if(execute_query("CREATE TABLE IF NOT EXISTS HEATING_ZONES (ID varchar(50) NOT NULL,ACTUAL double NOT NULL,SET_TEMPERATURE double NOT NULL,MODIFICATION datetime DEFAULT NULL,MODE int(11) NOT NULL,ACTOR tinyint(1) NOT NULL, COOLER tinyint(1) NOT NULL, PRIMARY KEY (ID)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci",conn)){
			return -1;
		}
		bool delete_termostat = false;
		query_state = mysql_query(conn, ("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE table_schema = '"+my_mysql_db+"'   AND table_name = 'termostat';").c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(string(row[0]) == "7"){
						delete_termostat = true;
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
		if(delete_termostat){
			execute_query("drop table termostat");
		}

		if(execute_query("CREATE TABLE IF NOT EXISTS termostat (id_termostat int(11) NOT NULL AUTO_INCREMENT, day int(11) NOT NULL,from_time int(11) NOT NULL,to_time int(11) NOT NULL,temp float NOT NULL, PRIMARY KEY (id_termostat)) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
			return -1;
		}
	}
	
	if(my_has_electro_cons) {
		if(execute_query("CREATE TABLE IF NOT EXISTS CONSUMPTION_DATA (TYPE int(11) DEFAULT NULL,VALUE int(11) DEFAULT NULL,FLAG int(11) DEFAULT NULL,DATE datetime DEFAULT NULL) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
			return -1;
		}
	}
	if(my_watering != NULL){
		if(execute_query("CREATE TABLE IF NOT EXISTS watering (id_watering int(11) NOT NULL AUTO_INCREMENT,ident varchar(100) NOT NULL,running int(11) NOT NULL,auto int(11) NOT NULL,timer int(11) NOT NULL,time_1 time NOT NULL,time_2 time NOT NULL,time_3 time NOT NULL,time_4 time NOT NULL,time1ON int(11) NOT NULL,time2ON int(11) NOT NULL,time3ON int(11) NOT NULL,time4ON int(11) NOT NULL,monday int(11) NOT NULL,tuesday int(11) NOT NULL,wednesday int(11) NOT NULL,thursday int(11) NOT NULL,friday int(11) NOT NULL,sathurday int(11) NOT NULL,sunday int(11) NOT NULL,PRIMARY KEY (id_watering)) AUTO_INCREMENT=1 ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci", conn) != 0){
			return -1;
		}
	}
	if(my_turnikets != NULL){
		if(execute_query("CREATE TABLE IF NOT EXISTS "+my_turnikets->get_table_name()+"(id int NOT NULL AUTO_INCREMENT PRIMARY KEY, card_id varchar(20),valid_until datetime, card_type int) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci",conn)){
			return -1;
		}
	}
	if(my_notification!= NULL){
		if(execute_query("CREATE TABLE IF NOT EXISTS NOTIFICATION (id int NOT NULL AUTO_INCREMENT PRIMARY KEY, status int(8), text varchar(1000), value double, units varchar(100), cas TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP)  ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci",conn)){
			return -1;
		}
	}
	if(my_impulz_counter!= NULL){
		if(execute_query("CREATE TABLE IF NOT EXISTS impulz (id int NOT NULL AUTO_INCREMENT PRIMARY KEY,ident varchar(100), record_type int(8), impulz_count int(11), impulz_len int(11), cas TIMESTAMP)  ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_slovak_ci",conn)){
				return -1;
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
	return 0;
}

int hp_go::sync_with_db()
{
	MYSQL *conn;
	conn = mysql_init(NULL);
	string query= "";

	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;

	if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
#ifndef AXON_SERVER
		mysql_library_end();
#endif
		return -1;
	} else {
		if(my_security != NULL || my_jablotron != NULL){
			bool hippo_sec = true;
			if(my_jablotron != NULL){
				hippo_sec = false;
			}
			int control = 0;
			control = hippo_sec?my_security->is_sim_enabled():my_jablotron->is_sim_enabled();
			if(control){
				query = "SELECT STATUS from STATUSES where ITEM = '"+(hippo_sec?my_security->get_simulation_ident():my_jablotron->get_simulation_ident())+"'";
				query_state = mysql_query(conn, "SET NAMES 'utf8'");
				query_state = mysql_query(conn, query.c_str());
				if(query_state == 0){
					result = mysql_store_result(conn);
					if(result->row_count > 0){
						while ((row = mysql_fetch_row(result)) != NULL ) {
							if(row[0] != NULL){
								string tmp(row[0]);
								if(hippo_sec){
									my_security->set_sim_value(tmp=="1"?true:false);
								} else {
									my_jablotron->set_sim_value(tmp=="1"?true:false);
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
					cout << "query chyba " << endl;
				}
			}
			query = "SELECT ID,STATUS from SECURITY_ZONES";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					bool start_sim = false;
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL){
							if(hippo_sec){
								hp_security_zone *zone  = NULL; 
								zone = my_security->find_zone(row[0]);
								if(zone != NULL){
									int status = patch::string2int(string(row[1]));
									if(status != -1 ){
										zone->set_sec_status(status);
										if(status == 1 ){
											start_sim = true;
										}
									}
								}
							} else {
								hp_jablotron_zone *zone  = NULL; 
								zone = my_jablotron->find_zone(row[0]);
								if(zone != NULL){
									int status = patch::string2int(string(row[1]));
									if(status != -1 ){
										zone->set_zone_status(status);
										if(status == 1 ){
											start_sim = true;
										}
									}
								}
							}
						}
					}
					if(start_sim && (hippo_sec?my_security->get_simulation_value():my_jablotron->get_simulation_value())){
						start_simulation(1);
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
		}
		if(my2heating != NULL){
			query = "SELECT STATUS from STATUSES where ITEM = '"+my2heating->get_eco_ident()+"'";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL){
							my2heating->set_eco_temp(patch::string2float(row[0]));
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
			query = "SELECT ID, ACTUAL, SET_TEMPERATURE, MODE, ACTOR, COOLER from HEATING_ZONES";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					int heating_mode = 0;
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(my2heating != NULL){
							if(row[0] != NULL){
								string id(row[0]);
								if(id == "all"){
									heating_mode = patch::string2int(string(row[3]));
									my2heating->set_query_temp(patch::string2float(string(row[2])));
								} else {
									hp_heating_section *section = my2heating->find_section(row[0], "ident");
									if(section != NULL){
										section->set_query_temp(patch::string2float(row[2]));
										section->set_actual_temp(patch::string2float(row[1]));
										section->set_heater_state(patch::string2int(string(row[3])));
										section->set_cooler_state(patch::string2int(string(row[4])));
									} else {
										//cout << "Nenasiel som zonu: " << row[0] << endl;
									}
								}
							}
						}
					}
					check_heating_zones(heating_mode, 100);
				}
				if(result != NULL){
					mysql_free_result(result);
				}
			} else {
				char bla[200];
				strcpy(bla, mysql_error(conn));
				cout << bla << endl;
			}
		}
		if(my_lightning != NULL){
			if(my_lightning->get_dn_type() == "time"){
				query = "UPDATE STATUSES set STATUS = '"+patch::to_string(my_lightning->get_actual_int_mode())+"' where ITEM = '"+my_lightning->get_dn_ident()+"'";
				query_state = mysql_query(conn, "SET NAMES 'utf8'");
				query_state = mysql_query(conn, query.c_str());
				if(query_state == 0){
				} else {
					char bla[200];
					strcpy(bla, mysql_error(conn));
					cout << bla << endl;
				}
			} else if(my_lightning->get_dn_type() == "pin"){
				query = "SELECT STATUS from STATUSES where ITEM = '"+my_lightning->get_dn_ident()+"'";
				query_state = mysql_query(conn, "SET NAMES 'utf8'");
				query_state = mysql_query(conn, query.c_str());
				if(query_state == 0){
					result = mysql_store_result(conn);
					if(result->row_count > 0){
						while ((row = mysql_fetch_row(result)) != NULL ) {
							if(row[0] != NULL){
								my_lightning->set_actual_mode(row[0]);
								break;
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
			}
		}
		if(my_watering != NULL){
			query = "SELECT * from watering";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						int running = patch::string2int(string(row[2]));
						int auto_enabled = patch::string2int(string(row[3]));
						int timer= patch::string2int(string(row[4]));
						vector<int> start_time, time_enabled, week_day_enabled;
						for(int i=0; i<4; i++){
							string date(row[i+5]);
							start_time.push_back(patch::string2int(date.substr(0,2))*60+patch::string2int(date.substr(3,2)));
						}
						for(int i=0; i<4; i++){
							time_enabled.push_back(patch::string2int(string(row[i+9])));
						}
						for(int i=0; i<7; i++){
							week_day_enabled.push_back(patch::string2int(string(row[i+13])));
						}
						my_watering->setup_zone(string(row[1]),running, auto_enabled, timer, start_time, time_enabled, week_day_enabled);
					}
				}
				if(result != NULL){
					mysql_free_result(result);
				}
				query = "SELECT STATUS FROM STATUSES where ITEM = '"+my_watering->get_sensor_ident()+"'";
				query_state = mysql_query(conn, "SET NAMES 'utf8'");
				query_state = mysql_query(conn, query.c_str());
				if(query_state == 0){
					result = mysql_store_result(conn);
					if(result->row_count > 0){
						while ((row = mysql_fetch_row(result)) != NULL ) {
							my_watering->setup_wat_enabled(my_watering->get_sensor_ident(), patch::string2int(string(row[0])));
						}
					}
					if(result != NULL){
						mysql_free_result(result);
					}
				}
			} else {
				char bla[200];
				strcpy(bla, mysql_error(conn));
			}
		}


		query = "SELECT ITEM, STATUS from STATUSES";
		query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL){
						int pin_pos = find_item(row[0], "desc");
						if(pin_pos != -1){
							string value(row[1]);
							if(my_pins[pin_pos]->get_function() != "pwm"){
								if(value == my_pins[pin_pos]->get_active_value()){
									value = "009";
								}
							}
							string pin_function = my_pins[pin_pos]->get_function();
							if(pin_function != "onoffButton" && pin_function != "magKontakt" && pin_function != "dayTime"){
								my_pins[pin_pos]->set_status(value);
							}
						} else {
							if(my_gates != NULL){
								my_gates->init_gate_status(row[0],patch::string2int(string(row[1])));
							}
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

		if(my_rekuperacia != NULL){
			vector<string> rek_q = my_rekuperacia->sync_rek(my_pins);
			for(unsigned int k=0; k<rek_q.size(); k++){
				push_db_query(rek_q[k],DB_QUERY);
			}
		}
		if(my_turnikets != NULL){
			query = "delete from "+my_turnikets->get_table_name()+" where valid_until < NOW() && valid_until != FROM_UNIXTIME(0)";
			query_state = mysql_query(conn, query.c_str());

			query = "SELECT id, card_id, card_type, UNIX_TIMESTAMP(valid_until) as until from "+my_turnikets->get_table_name();
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL){
							cards_data_t tmp;
							tmp.id= row[0];
							tmp.card_id = row[1];
							tmp.card_type= row[2];
							tmp.valid_until= patch::string2int(string(row[3]));
							tmp.last_enter = 0;
							my_turnikets->add_card(tmp);
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
			my_turnikets->print_cards();
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

int hp_go::check_gui_mess(string ident)
{
	int res = -1;
	//cout << "looking for: " << ident << endl;
	map<string,string>::iterator i = my_trusted_devices.find(ident);

	if(i != my_trusted_devices.end()){
		res = std::distance(my_trusted_devices.begin(),i);
	}

	return res;
}

void hp_go::get_trusted_devices()
{
	MYSQL *conn;
	MYSQL_RES *result;
	MYSQL_ROW row;
	conn = mysql_init(NULL);
	string query= "";
	my_trusted_devices.clear();

	if((mysql_real_connect(conn,"localhost",my_mysql_user.c_str(), my_mysql_passwd.c_str(),my_mysql_db.c_str(),0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		cout << bla << endl;
		mysql_close(conn);
#ifndef AXON_SERVER
		mysql_library_end();
#endif
		conn = NULL;
		return ;
	} else {
		query = "select identification, notes  from trusted_devices";
		int query_state = mysql_query(conn, "SET NAMES 'utf8'");
		query_state = mysql_query(conn, query.c_str());
		if(query_state == 0){
			result = mysql_store_result(conn);
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					if(row[0] != NULL && row[1] != NULL){
						my_trusted_devices.insert(std::make_pair<string,string>(row[0],row[1]));
					}
				}
			}
			if(result != NULL){
				mysql_free_result(result);
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
}

int hp_go::fill_db()
{
	MYSQL *conn;
	conn = mysql_init(NULL);
	string query= "";

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
		process_hbxs(conn);
		process_statuses(conn);
		process_scenaria(conn);
		process_alarm(conn);
		if(my_security != NULL){
			process_security(conn);
		}
		if(my_jablotron != NULL){
			my_jablotron->process_jablotron(conn);
		}
		if(my2heating != NULL){
			my2heating->process_db(conn);
		}
		if(my_impulz_counter != NULL){
			my_impulz_counter->process_db(conn);
		}
		if(my_dali != NULL){
			my_dali->fill_db(conn);
		}
		if(my2modbus != NULL){
			my2modbus->process_db(conn);
		}

		if(my_watering!= NULL){
			my_watering->process_db(conn);
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

int hp_go::process_hbxs(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT mac from hbxs";
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
		query = "INSERT INTO hbxs(xbee_driver_id,mac, status, modification ) VALUES ";
		for(unsigned int i=0; i<my_hbxs.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_hbxs[i]->get_mac()){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+patch::to_string(my_hbxs[i]->get_pos())+"', '"+my_hbxs[i]->get_mac()+"', '-1', NOW()),";
			}
		}
		if(query[query.length()-1] != ' '){
			execute_query(query.substr(0,query.length()-1), conn);
		}
	}

	return 0;
}

string hp_go::get_last_id(string table_name)
{
	string query="";
	string res = "";
	if(table_name == "AUTOMATED_DEVICES"){
		query =  "SELECT ID FROM AUTOMATED_DEVICES ORDER BY ID DESC LIMIT 1";
	}
	if(table_name == "SCENARIA"){
		query =  "SELECT id_scenaria FROM SCENARIA ORDER BY id_scenaria DESC LIMIT 1";
	}

	if(query != ""){
		MYSQL *conn;
		MYSQL_RES *result;
		MYSQL_ROW row;
		int query_state;

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
			return "";
		} else {
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
			if(query_state == 0){
				result = mysql_store_result(conn);
				if(result->row_count > 0){
					while ((row = mysql_fetch_row(result)) != NULL ) {
						if(row[0] != NULL){
							res = row[0];
						}
					}
				}
				if(result != NULL){
					mysql_free_result(result);
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
	}
	return res;
}


int hp_go::process_security(MYSQL *conn)
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
		/*
		for(unsigned int j=0; j<items_idents.size(); j++){
			if(!my_security->is_active_zone(items_idents[j])){
				query += "('"++"'
			}
		}
		*/

		bool add_item = true;
		for(unsigned int i=0; i<my_security->get_zones_count(); i++){
			add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_security->get_zone_ident(i) && my_security->get_zone_ident(i) != ""){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_security->get_zone_ident(i)+"', NOW(),0,0,0,-1),";
			}
		}
		string all_zone_ident = my_security->get_all_ident();
		add_item = true;
		for(unsigned int j=0; j<items_idents.size(); j++){
			if(items_idents[j] == all_zone_ident){
				add_item = false;
				break;
			}
		}
		if(add_item){
			query += "('"+all_zone_ident+"', NOW(),0,0,0,-1),";
		}
		if(query[query.length()-1] != ' '){
			execute_query(query.substr(0,query.length()-1), conn);
		}
	}

	return 0;
}

int hp_go::process_scenaria(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;
	query = "SELECT ident from SCENARIABLE_DEVICES";
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
		query = "INSERT INTO SCENARIABLE_DEVICES (ident, label,type) VALUES ";
		for(unsigned int i=0; i<my_pins.size(); i++){
			bool add_item = true;
			if(!my_pins[i]->is_scenario()){
				continue;
			}
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_pins[i]->get_desc()){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_pins[i]->get_desc()+"', '"+my_pins[i]->get_desc_2()+"', '"+patch::to_string(my_pins[i]->get_int_type())+"'),";
			}
		}
		if(query[query.length()-1] != ' '){
			execute_query(query.substr(0,query.length()-1), conn);
		}
	}

	return 0;
}

int hp_go::process_alarm(MYSQL *conn, bool only_check)
{
	string query;
	bool quit_sql = false;
	int query_state;
	MYSQL_RES *result;
	MYSQL_RES *result_scen = NULL;
	MYSQL_ROW row;
	MYSQL_ROW row_scen;
	vector<string> items_idents;
	
	if(conn == NULL){
		quit_sql = true;
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
		} 
	}

	if(only_check){
		query = "SELECT NAME, COMMAND, TIME, ID FROM AUTOMATED_DEVICES";
		my_alarm_checker.clear();
	} else {
		query = "SELECT NAME from AUTOMATABLE_DEVICES";
	}
	query_state = mysql_query(conn, "SET NAMES 'utf8'");
	query_state = mysql_query(conn, query.c_str());
	if(query_state == 0){
		if(!only_check){
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
			query = "INSERT INTO AUTOMATABLE_DEVICES(NAME, LABEL,TYPE) VALUES ";
			for(unsigned int i=0; i<my_pins.size(); i++){
				bool add_item = true;
				if(!my_pins[i]->is_scenario()){
					continue;
				}
				for(unsigned int j=0; j<items_idents.size(); j++){
					if(items_idents[j] == my_pins[i]->get_desc()){
						add_item = false;
						break;
					}
				}
				if(add_item){
					query += "('"+my_pins[i]->get_desc()+"', '"+my_pins[i]->get_desc_2()+"', '"+patch::to_string(my_pins[i]->get_int_type())+"'),";
				}
			}
			if(query[query.length()-1] != ' '){
				execute_query(query.substr(0,query.length()-1), conn);
			}

			query = "SELECT NAME, COMMAND, TIME, ID FROM AUTOMATED_DEVICES";
			query_state = mysql_query(conn, "SET NAMES 'utf8'");
			query_state = mysql_query(conn, query.c_str());
		}
		if(query_state == 0){
			result = mysql_store_result(conn);
			mysql_query(conn,"select id_scenaria, label from SCENARIA");
			result_scen = mysql_store_result(conn);
			std::map<string,string> scen_data;
			if(result_scen->row_count > 0){
				while ((row_scen = mysql_fetch_row(result_scen)) != NULL ) {
					scen_data.insert(std::make_pair(string(row_scen[0]), string(row_scen[1])));
				}
			}
			if(result->row_count > 0){
				while ((row = mysql_fetch_row(result)) != NULL ) {
					hp_alarm_checker_t tmp;
					string time_str = row[2];
					tmp.hour= atoi(time_str.substr(0, time_str.find(":")).c_str());
					time_str = time_str.substr(time_str.find(":")+1);
					tmp.min= atoi(time_str.substr(0, time_str.find(":")).c_str());
					tmp.send_value = row[1];
					tmp.id = row[3];

					tmp.pin_pos = find_item(row[0],"desc");
					if (string(row[1]) == "99") {
						for(map<string,string>::iterator i=scen_data.begin(); i != scen_data.end(); i++){							
							if((*i).second == string(row[0])){
								tmp.scen_command= "scen_"+(*i).first+"_1";
							}
						}
					}
					my_alarm_checker.push_back(tmp);
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}
			if(result_scen != NULL){
				mysql_free_result(result_scen);
			}
		}
	}
	if(quit_sql){
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

int hp_go::process_statuses(MYSQL *conn)
{
	string query;
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;

	query = "SELECT ITEM from STATUSES";
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
		query = "INSERT INTO STATUSES (ITEM,LABEL, STATUS, MODIFICATION) VALUES ";
		if(my_gates != NULL){
			for(int i=0; i<my_gates->get_gates_size(); i++){
				bool add_item = true;
				for(unsigned int j=0; j<items_idents.size(); j++){
					if(items_idents[j] == my_gates->get_lock_ident(i)){
						add_item = false;
						break;
					}
				}
				if(add_item){
					query += "('"+my_gates->get_lock_ident(i)+"','', '0', NOW()),";
					items_idents.push_back(my_gates->get_lock_ident(i));
				}
				add_item = true;
				for(unsigned int j=0; j<items_idents.size(); j++){
					if(items_idents[j] == my_gates->get_status_ident(i)){
						add_item = false;
						break;
					}
				}
				if(add_item){
					query += "('"+my_gates->get_status_ident(i)+"','', '0', NOW()),";
				}
			}
		}
		if(my_lightning != NULL){
			if(my_lightning->get_dn_ident() != ""){
				bool add_item = true;
				for(unsigned int j=0; j<items_idents.size(); j++){
					if(items_idents[j] == my_lightning->get_dn_ident()){
						add_item = false;
						break;
					}
				}
				if(add_item){
					query += "('"+my_lightning->get_dn_ident()+"','', '0', NOW()),";
				}
			}
		}
		for(unsigned int i=0; i<my_meteo_idents.size(); i++){
			bool add_item = true;
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == my_meteo_idents[i].ident){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+my_meteo_idents[i].ident+"','', '0', NOW()),";
			}
		}
		if(my_security != NULL){
			if(my_security->is_sim_enabled()){
				bool add_item = true;
				for(unsigned int j=0; j<items_idents.size(); j++){
					if(items_idents[j] == my_security->get_simulation_ident()){
						add_item =false;
						break;
					}
				}
				if(add_item){
					query += "('"+my_security->get_simulation_ident()+"','', '0', NOW()),";
				}
			}
		}
		if(my_rekuperacia != NULL){
			vector<string> rek_idents= my_rekuperacia->get_rek_zones_idents();
			for(unsigned int i=0; i<rek_idents.size(); i++){
				bool add_item = true;
				for(unsigned int j=0; j<items_idents.size(); j++){
					if(items_idents[j] == rek_idents[i]){
						add_item = false;
						break;
					}
				}
				if(add_item){
					query += "('"+rek_idents[i]+"','', '0', NOW()),";
				}
			}
		}
			
		for(unsigned int i=0; i<my_pins.size(); i++){
			bool add_item = true;
			if(!my_pins[i]->update_gui()){
				continue;
			}
			for(unsigned int j=0; j<items_idents.size(); j++){
				if(items_idents[j] == (my_pins[i]->get_gui_desc())){
					add_item = false;
					break;
				}
			}
			if(add_item){
				query += "('"+(my_pins[i]->get_gui_desc())+"','"+(my_pins[i]->get_desc_2())+"', '0', NOW()),";
			}
		}
		if(query[query.length()-1] != ' '){
			execute_query(query.substr(0,query.length()-1), conn);
		}
	}
	return 0;
}

int hp_go::execute_query(string query, MYSQL *conn)
{
	int query_state;
	if(conn != NULL){
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


hp_go::~hp_go()
{

}
