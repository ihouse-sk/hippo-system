#include "../include/hp_ws_server.h"

hp_ws_server::hp_ws_server() : WsServer()
{
	this->config.port = 8080;
}

void hp_ws_server::init()
{
	auto &data_collector = this->endpoint["^/news/?$"];

	data_collector.on_message = [] (shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
		auto out_message = in_message->string();
		//cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << endl;
		//connection->send_close(1000);
	};

	data_collector.on_open = [](shared_ptr<WsServer::Connection> connection) {
		cout << "Server: Opened connection " << connection.get() << endl;
	};

	// See RFC 6455 7.4.1. for status codes
	data_collector.on_close = [](shared_ptr<WsServer::Connection> connection, int status, const string & /*reason*/) {
		cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
	};

	// Can modify handshake response headers here if needed
	data_collector.on_handshake = [](shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
		return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
	};

	// See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
	data_collector.on_error = [](shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
		cout << "Server: Error in connection " << connection.get() << ". " << "Error: " << ec << ", error message: " << ec.message() << endl;
	};
}
void hp_ws_server::start_ws()
{
	this->start();
}
void hp_ws_server::push_mess(Json::Value &js)
{
	if(!js.isNull() && js.size() != 0){
		for(auto &a_connection : this->get_connections()){
			//cout << a_connection->query_string <<", " << a_connection->path <<  endl;
			//cout << a_connection->remote_endpoint() << endl;
			if(a_connection->path.find("news") != std::string::npos){
				a_connection->send(wr.write(js));
			}
		}
		js.clear();
	}
}

hp_ws_server::~hp_ws_server(){}
