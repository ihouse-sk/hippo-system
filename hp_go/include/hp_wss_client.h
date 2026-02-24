#ifndef HP_WSS_CLIENT_H
#define HP_WSS_CLIENT_H

#include "../../hp_lib/include/client_wss.hpp"

#ifndef HP_DATA_PROVIDER
#include "hp_data_provider.h"
#endif


#define WS_CLOSED 0
#define WS_OPEN 1
#define WS_CONNECTING 2

using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;
using namespace std;


class hp_wss_client : WssClient
{
	public:
		hp_wss_client(hp_data_provider *,string, bool developer = false);
		void init();
		void start_ws();
		void stop_ws();
		void push_mess(Json::Value &);
		int is_running() { return my_state;};
		void check_heartbeat() {
		        if (!heartbeat) {
		            my_state = WS_CLOSED;
		        }
		        heartbeat = false;
		}
		bool check_active() {
		        if (my_state == WS_CLOSED) {
		            this->stop();
		            return false;
		        }
		        return true;
		};
		~hp_wss_client();
	private:
		Json::FastWriter wr;
		hp_data_provider *my_provider;

		bool heartbeat = true;
		int my_state = 0;

		int my_active;
		int my_client_counter;
		bool my_was_open = false;
		string my_system_ident;
};

#endif
