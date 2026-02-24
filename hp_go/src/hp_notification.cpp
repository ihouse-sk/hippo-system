#include "../include/hp_notification.h"

hp_notification::hp_notification(XMLNode node,string system_ident, hp_db_data_t *data, bool simulator)
{
	my_hbx_check_time = patch::string2int(xml_parser.get_node_value(node, "hbx_notification"));
	for(uint16_t i=0; i<node.getChildNode("pins").nChildNode("pin"); i++){
		my_pins_not.push_back(hp_pin_notifier(node.getChildNode("pins").getChildNode("pin",i)));
	}
	for(uint16_t i=0; i<node.getChildNode("guards").nChildNode("guard"); i++){
		my_guards.push_back(hp_guard(node.getChildNode("guards").getChildNode("guard",i)));
	}
	my_db_data = data;
	my_system_identifier = system_ident;
	my_security_notification= xml_parser.get_node_value(node, "security_notification")=="yes"?true:false;

	my_telegram_token = "";
	my_main_url = "";
	my_main_chat_id = xml_parser.get_node_value(node, "main_chat_id");
	my_simulator = simulator;
}
void hp_notification::setup_guards_position(const vector<hp_virtual_pin *> pins)
{
	for(uint16_t j=0; j<my_guards.size(); j++){
		for(auto i = pins.begin(); i != pins.end(); ++i){
			if( (*i)->get_desc() == my_guards[j].get_ident()){
				my_guards[j].setup_pin_pos(std::distance(pins.begin(), i));
			}
		}
	}
}
void hp_notification::check_pin_notification(string pin_id, string value, string pin_label)
{
	for(uint16_t i=0; i<my_pins_not.size(); i++) {
		if(pin_id == my_pins_not[i].get_ident()){
			string not_type = my_pins_not[i].get_type(), mess = "";
			if(not_type == "php"){
				mess = my_pins_not[i].get_url(value);
				push_db_query("Php query: "+mess, DB_LOG_COM);
			//	thread th2(hp_threaded_socket(0,"",mess, THREAD_CURL));
			//	th2.detach();
			} else if (not_type == "socket"){
				mess = "GUI_11_extpin_" +pin_id + "_"+value;
				push_db_query("Posielam notification: "+mess+" ip:port "+my_pins_not[i].get_ip() +":"+my_pins_not[i].get_port() , DB_LOG_COM);
				thread th2(hp_threaded_socket(patch::string2int(my_pins_not[i].get_port()), my_pins_not[i].get_ip(), mess));
				th2.detach();
			}
		}
	}
	for(uint16_t i=0; i<my_guards.size(); i++) {
		//cout << pin_id << " == " <<  my_guards[i].get_ident() << " && " <<  my_guards[i].get_type()<<  " == value " << endl;
		if(pin_id == my_guards[i].get_ident() && my_guards[i].get_type() == "value"){
			bool push_notification = false;
			string condition = "";
			if(my_guards[i].get_status()){
				// ak je guard aktivny a cakame kym sa vrati do normalu
				if(my_guards[i].get_condition() == "lt"){
					if(my_guards[i].get_critical_value() < patch::string2float(value)){
						push_notification = true;
						condition = "lt";
					}
				} else if ( my_guards[i].get_condition() == "gt"){
					if(my_guards[i].get_critical_value() > patch::string2float(value)){
						push_notification = true;
						condition = "gt";
					}
				} else if (my_guards[i].get_condition() == "eq"){
					//cout << "val: " <<  patch::string2float(value) << ", critical: " << my_guards[i].get_critical_value() << endl;
					if(!((my_guards[i].get_critical_value() + 0.1 < patch::string2float(value)) && (my_guards[i].get_critical_value() - 0.1 > patch::string2float(value)))){
						push_notification = true;
						condition = "eq";
					}
				}
				if(push_notification){
					my_guards[i].set_status(false);
					for(uint16_t j=0; j<my_guards[i].n_notifiers(); j++){
						push_guard_notification(my_guards[i].get_notifier(j), condition, my_guards[i].get_text_ok(), pin_label, value,0, my_guards[i].get_unit());
					}
				}
			} else {
				if(my_guards[i].get_condition() == "lt"){
					if(my_guards[i].get_critical_value() > patch::string2float(value)){
						push_notification = true;
						condition = "lt";
					}
				} else if ( my_guards[i].get_condition() == "gt"){
					if(my_guards[i].get_critical_value() < patch::string2float(value)){
						push_notification = true;
						condition = "gt";
					}
				} else if (my_guards[i].get_condition() == "eq"){
					if((my_guards[i].get_critical_value() + 0.1 > patch::string2float(value)) && (my_guards[i].get_critical_value() - 0.1 < patch::string2float(value))){
						push_notification = true;
						condition = "eq";
					}
				}
				if(push_notification){
					my_guards[i].set_status(true);
					for(uint16_t j=0; j<my_guards[i].n_notifiers(); j++){
						push_guard_notification(my_guards[i].get_notifier(j), condition, my_guards[i].get_text(), pin_label, value,1, my_guards[i].get_unit());
					}
				}
			}
		}
		if(pin_id == my_guards[i].get_ident() && my_guards[i].get_type() == "time"){
			if(my_guards[i].get_status()){
				for(uint16_t j=0; j<my_guards[i].n_notifiers(); j++){
					push_guard_notification(my_guards[i].get_notifier(j), "ok", my_guards[i].get_text_ok(), pin_label, value,0, my_guards[i].get_unit());
				}
			}
			my_guards[i].set_status(false);
			my_guards[i].set_last_change_time();

		}
	}
}

void hp_notification::check_periodic_guard(const vector<hp_virtual_pin *> pins)
{
	for(uint16_t i=0; i<my_guards.size(); i++) {
		if(my_guards[i].get_type() == "time"){
			int pos = my_guards[i].get_pin_pos();
			if(pos >= 0 && pos < (int)pins.size()){
				if(pins[pos]->get_desc() == my_guards[i].get_ident()){ 
					if(!my_guards[i].get_status()){
						if(time(NULL) > my_guards[i].get_last_change_time() + my_guards[i].get_check_time()){
							my_guards[i].set_status(true);
							for(uint16_t j=0; j<my_guards[i].n_notifiers(); j++){
								push_guard_notification(my_guards[i].get_notifier(j),"aa", my_guards[i].get_text(), pins[pos]->get_desc_2(),patch::to_string(my_guards[i].get_check_time()),1, "minút");
							}
						}
					}
				}
			}
		}
	}
}

void hp_notification::push_security_notification(string mess, int sec_value)
{
	if(my_security_notification){
		string api_mess = "";
		if(sec_value == 0){
			api_mess = "Zóna zabezpečnia: " + mess + " bola odarmovaná!";
		} else if (sec_value == 1){
			api_mess = "Zabezpečovanie aktívne pre zónu: " + mess;
		} else {
			api_mess = "Poplach v zóne: " + mess;
		}
		//cout << api_mess << endl;
		if(my_main_chat_id != ""){
			push_telegram_notification(api_mess);
		}
		string query = "INSERT INTO NOTIFICATION (text, value, units, status) VALUES ('"+mess+"','"+patch::to_string(sec_value)+"','',"+patch::to_string(sec_value)+")";
		push_db_query(query);
	}
}
string hp_notification::replace_url(string mess)
{
	string res = "";
	std::for_each(mess.begin(), mess.end(), [&res](char ch){
			switch (ch) {
				case ' ':
					res += "%20";
					break;
				case '&': 
					res += "%20";
					break;
				case ':': 
					res += "%3A";
					break;
				default:
					res += ch;
					break;
			}
			});
	return res;
}

void hp_notification::push_notification(string mess_type, string mess)
{
	if(my_simulator){
		//cout <<"Notifikacia: " << mess << ", type: " << mess_type << endl;
	} else {
		if(mess != "" && mess_type != ""){
			mess = replace_url(mess);
			if(my_main_chat_id != "" && mess_type != NN_CHECK){
				push_telegram_notification(mess);
			}
			push_main_information(mess_type, mess);
		}
	}
}
	
void hp_notification::push_main_information(string mess_type, string mess)
{
	if(mess != "" && mess_type != ""){
		// /apiHippoNotification/hippoReciever.php?key1=info
		string query = "/apiHippoNotification/hippoReciever.php?key1=info&sysIdent="+my_system_identifier+"&messType="+mess_type+"&apiMess="+mess;
		create_thread(443, my_main_url,query,mess_type);
	}
}

void hp_notification::push_telegram_notification(string mess)
{
	if(mess != ""){
		string query = "/bot"+my_telegram_token+"/sendMessage?chat_id="+my_main_chat_id+"&text="+mess;
		create_thread(443, "api.telegram.org",query);
	}
}

void hp_notification::push_guard_notification(const hp_guard_notify_t *notify,string condition, string text, string ident, string value, int status, string unit)
{
	string query;
	if(notify != NULL && condition != ""){
		if(notify->type == "local"){
			query = "INSERT INTO NOTIFICATION (text, value, units, status) VALUES ('"+(ident + " " +text)+"','"+value+"','"+unit+"',"+patch::to_string(status)+")";
			push_db_query(query);
		}
		if(notify->type == "telegram"){
			string mess = ident + " " + text + " " + value + " "+unit;
		//	query = "https://api.telegram.org/bot"+my_telegram_token+"/sendMessage?chat_id="+notify->str_1+"&text="+mess;
			create_thread(443, "api.telegram.org",("/bot"+my_telegram_token+"/sendMessage?chat_id="+notify->str_1+"&text="+mess));
		}
	}
	// NOTIFICATION (id int NOT NULL AUTO_INCREMENT PRIMARY KEY, text varchar(1000), value double, units varchar(100)
}
void hp_notification::create_thread(int port, string url, string mess,string mess_type, bool https)
{
	int delay = 0;
	if(mess_type == NN_CHECK){
		delay = rand()%10;
	}
	thread th2(hp_threaded_socket(port, url,mess, THREAD_ASIO,delay,https));
	th2.detach();
}

void hp_notification::push_db_query(string query, int type, int log_level)
{
	if(my_db_data != NULL){
		hp_db_queries_t tmp;
		tmp.query = query;
		tmp.type = type;
		tmp.log_level = log_level;
	
		my_db_data->mtx.lock();
		my_db_data->queries.push_back(tmp);
		my_db_data->mtx.unlock();
	}
}



hp_notification::~hp_notification() {}
