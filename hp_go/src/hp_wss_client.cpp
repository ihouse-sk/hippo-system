#include "../include/hp_wss_client.h"

hp_wss_client::hp_wss_client(hp_data_provider *provider, string system_ident, bool developer) : WssClient(developer?"hippo.vps.wbsprt.com:8089":"hippo.vps.wbsprt.com:8088")
{
	my_provider = provider;
	my_state = 0;
	my_client_counter = 0;
	my_system_ident = system_ident;
	my_active = true;
}

void hp_wss_client::init()
{
	this->on_message = [&](std::shared_ptr<WssClient::Connection> connection, std::shared_ptr<WssClient::InMessage> in_message) {
		auto out_message = in_message->string();
		bool isCmd = true;
		try {
			Json::Value jrq;
			Json::Reader jr;
			bool b = jr.parse(out_message,jrq);
			if(b){
				if(!jrq["purpose"].empty()){				
					if(jrq["purpose"].asString() != "cmd"){
						if(!jrq["payload"].empty()){
							if(!jrq["payload"]["type"].empty() && !jrq["payload"]["data"].empty()){
								if(jrq["payload"]["type"].asString() == "client_count"){
									my_client_counter = jrq["payload"]["data"].asInt();
									//cout <<"My client counter: " << my_client_counter<< endl;
									isCmd=false;
								}
							}
						}
					}
				}
			}
		} catch (const exception &e) {
			cout << "client catch: " << e.what() << endl;
		}
		//cout <<isCmd << ", Client recieve: " << out_message << endl;
		if(isCmd){
			string resp = my_provider->process_mess(out_message);
			//cout <<"Client response: " << resp<< endl;
			connection->send(resp);
		}
	};
	this->on_ping = [&](std::shared_ptr<WssClient::Connection> connection) {
		//cout <<"Ping recieved" << endl;
		heartbeat = true;
		//connection->send(0x09);
	};
	
	this->on_open = [&](std::shared_ptr<WssClient::Connection> connection) {
		my_state= 1;
		my_active = true;
		my_was_open = true;
		Json::Value val;
		Json::FastWriter fr;
		val["purpose"] = "subscribe";
		val["payload"]["type"] = "server";
		val["payload"]["ident"] = my_system_ident;
	//	cout <<"Sending subscribe: " << val << endl;
		connection->send(fr.write(val));
	};
	
	this->on_close = [&](std::shared_ptr<WssClient::Connection> connection, int status, const std::string &) {
		std::cout << "Client: Closed connection with status code " << status << std::endl;
		my_state= 0;
	};
	
	this->on_error = [&](std::shared_ptr<WssClient::Connection> connection, const SimpleWeb::error_code &ec) {
		std::cout << "Client: Error: " << ec << ", error message: " << ec.message() << std::endl;
		my_state= 0;
	};
}
/*
bool hp_wss_client::check_active() {
	if(!my_active && my_was_open) {
		//cout <<"Stoping client" << endl;
		my_was_open = false;
		my_active = false;
		my_state = 0;
		this->stop();
		return false;
	}
	my_active = false;
	return true;
}
*/
void hp_wss_client::start_ws()
{
	this->start();
}

void hp_wss_client::stop_ws()
{
	this->stop();
}

void hp_wss_client::push_mess(Json::Value &js)
{
	if(my_client_counter){
		if(!js.isNull() && js.size() != 0){
			Json::Value new_js;
			new_js["type"] = "news";
			new_js["data"] = js;
			new_js["purpose"] = "srv";
			//cout << new_js << endl;
			this->connection->send(wr.write(new_js));
		}
	}
}


hp_wss_client::~hp_wss_client(){}
