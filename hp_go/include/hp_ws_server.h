#ifndef HP_WS_SERVER_H
#define HP_WS_SERVER_H

#include "../../hp_lib/include/server_ws.hpp"
#include "../../hp_lib/include/server_wss.hpp"
#include <jsoncpp/json/json.h>
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using namespace std;


class hp_ws_server : WsServer
{
	public:
		hp_ws_server();
		void init();
		void start_ws();
		void push_mess(Json::Value &);
		~hp_ws_server();
	private: 
		Json::FastWriter wr;
};

#endif
