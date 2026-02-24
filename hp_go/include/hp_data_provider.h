#ifndef HP_DATA_PROVIDER
#define HP_DATA_PROVIDER

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif
#include <jsoncpp/json/json.h>

#ifndef HP_INCOMING_S
#define HP_INCOMING_S
typedef struct hp_incoming {
	std::vector<std::string> mess;
	std::mutex mtx_incomming;	
}hp_incoming;
#endif

using namespace std;

class hp_data_provider {
	public:
		hp_data_provider(hp_incoming *data, string cnf_data);
		string process_mess(string);
		void set_shm_data(string &);
		~hp_data_provider();
	private: 
		Json::FastWriter wr;
		string my_shm_data="";
		string my_cnf_data="";
		hp_incoming *my_data_incoming;
		Json::Value read_config_data();
		Json::Value get_scenarios();
		Json::Value get_scenarioable_devices();
		Json::Value get_device_in_scenario(string scen_id);
		Json::Value get_automatable_devices();
		Json::Value get_automated_devices();
		Json::Value get_thermostat_settings();
		Json::Value get_conspumption_data(string type, string period);
		Json::Value get_history_data(Json::Value jrq, string type);
		string json_stringify(Json::Value val);

};



#endif
