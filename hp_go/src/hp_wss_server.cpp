#include "../include/hp_wss_server.h"

hp_wss_server::hp_wss_server(hp_data_provider *provider) : WsServer()
{
	this->config.port = 8081;
	my_provider = provider;
}

void hp_wss_server::init()
{
	auto &data_collector = this->endpoint["^/data_handler/?$"];

	data_collector.on_message = [&] (shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
		auto out_message = in_message->string();
		string resp = my_provider->process_mess(out_message);
		connection->send(resp);
		//connection->send_close(1000);
	};

	data_collector.on_open = [](shared_ptr<WsServer::Connection> connection) {
		cout << "Server: Opened connection " << connection.get() << ", path: " << connection->path<<  endl;
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

void hp_wss_server::start_ws()
{
	this->start();
}

void hp_wss_server::stop_ws()
{
	this->stop_accept();
	this->stop();
}

void hp_wss_server::push_mess(Json::Value &js)
{
	if(!js.isNull() && js.size() != 0){
		Json::Value new_js;
		new_js["type"] = "news";
		new_js["data"] = js;
		if(this->get_connections().size() > 0){
		//	cout<<"Conn count: "<<this->get_connections().size()<< ", Posielam event: " << wr.write(new_js)<< endl;
		}
		for(auto &a_connection : this->get_connections()){
			//cout << a_connection->query_string <<", " << a_connection->path <<  endl;
			//cout << a_connection->remote_endpoint() << endl;
			if(a_connection->path.find("data_handler") != std::string::npos){
				a_connection->send(wr.write(new_js));
			}
		}
		//js.clear();
	}
}


hp_wss_server::~hp_wss_server(){}
