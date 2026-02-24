#ifndef HP_WSS_SERVER_H
#define HP_WSS_SERVER_H

#include "../../hp_lib/include/server_ws.hpp"

#ifndef HP_DATA_PROVIDER
#include "hp_data_provider.h"
#endif

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using namespace std;


class hp_wss_server : WsServer
{
	public:
		hp_wss_server(hp_data_provider *);
		void init();
		void start_ws();
		void stop_ws();
		void push_mess(Json::Value &);
		~hp_wss_server();
	private:
		Json::FastWriter wr;
		hp_data_provider *my_provider;

};

#endif
