#ifndef HP_SOCKET_LISTENER_H
#define HP_SOCKET_LISTENER_H

#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <memory>
#include "hp_tcp_stream.h"
#include "hp_tcp_acceptor.h"

#ifndef HP_DEFAULTS_H
#include "hp_defaults.hpp"
#endif

using namespace std;

class hp_socket_listener {
	public:
		hp_socket_listener(int port, shared_ptr<hp_gui_data_t> data,string str_end = "NeverFindMeee"): my_port(port),my_str_end(str_end), my_gui_data(data) {};
		//hp_socket_listener(int port, hp_gui_data_t *data): my_port(port), my_gui_data(data) {};
		int init() {
			my_acceptor = make_shared<hp_tcp_acceptor> (my_port, "127.0.0.1");//new TCPAcceptor(port,ip.c_str());
		//	my_acceptor = new hp_tcp_acceptor(my_port,"127.0.0.1");
			if(my_acceptor->start() == 0){
				return 0;
			} else { 
				return -1;
			}
		}
		void operator()() {
			hp_tcp_stream *stream = nullptr;
			bool finish = false;
			while(1){
				stream = my_acceptor->accept();
				if(stream != nullptr){
					ssize_t len;
					char line[1024];
					while ((len = stream->receive(line, sizeof(line))) > 0) {
						while((line[len-1] == '\n' || line[len-1] == '\r') && len > 0){
							len--;
						}
						line[len] = 0;
						//printf("received - %s\n", line);
						string str = line;
						if(!str.empty()){
							my_gui_data->mtx.lock();
							my_gui_data->data.push_back(str);
							my_gui_data->mtx.unlock();
						}
						if(str.find(my_str_end) != string::npos){
							finish = true;
							break;
						}
					}
					delete stream;
					stream = nullptr;
				}
				if(finish){
					break;
				}
				if(my_gui_data->finish){
					break;
				}
			}
		}
		~hp_socket_listener(){};
	private:
		int my_port;
		int my_socket;
		string my_str_end;
		shared_ptr<hp_tcp_acceptor> my_acceptor = nullptr;
		shared_ptr<hp_gui_data_t> my_gui_data;
		
		//hp_tcp_acceptor *my_acceptor = nullptr;
		//hp_gui_data_t *my_gui_data;
};


#endif
