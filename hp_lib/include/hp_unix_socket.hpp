#ifndef HP_UNIX_SOCKET_H
#define HP_UNIX_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>

#define MAX_MSG 1024

#ifndef HP_DEFAULTS_H
#include "hp_defaults.hpp"
#endif

using namespace std;

class hp_unix_socket {
	public:
		hp_unix_socket(shared_ptr<hp_gui_data_t> data, std::string cls_socket, int socket_type = SOCK_STREAM): my_data(data), my_socket_name(cls_socket), my_socket_type(socket_type) {
			unlink(my_socket_name.data());
		}
		int create_socket() {
			my_socket_fg = socket(AF_UNIX, my_socket_type, 0);	
			if (my_socket_fg < 0) { 
				std::cerr << "cannot create listen socket"<<std::endl; 
				return -1;
			} 
		
			unlink(my_socket_name.data());
			m_server_unixaddress.sun_family = AF_UNIX;
			//strcpy(m_server_unixaddress.sun_path,"/home/laufem/iHouse/vyvoj/thread/control_layer_socket");
			strcpy(m_server_unixaddress.sun_path,my_socket_name.data());
			socklen_t m_server_len = sizeof(m_server_unixaddress);
			if (bind(my_socket_fg, (struct sockaddr *)&m_server_unixaddress, m_server_len) < 0) {
				std::cerr << "cannot bind unix socket"<<" on domain = "<<my_socket_name<< ", erno: " << errno << endl; 
				return -1;
			}
			// Vytvoříme frontu spojení a začneme čekat na klienty:
		    	listen(my_socket_fg, 100);
			return 0;
		}
		string accept_socket() {
			string res = "";
        		char buffer[MAX_MSG];

			memset(buffer,0x0,MAX_MSG);
			unsigned int client_len = sizeof(client_unixaddress);
			int client_sockfd = accept(my_socket_fg, (struct sockaddr *)&client_unixaddress, &client_len);
			if(client_sockfd < 0) {
				return res;
			}
			recv(client_sockfd, buffer, MAX_MSG, 0);
			
			close_client_socket(client_sockfd);
		
			for (int i=0;i<MAX_MSG; i++) {
				res+=buffer[i];
			}

			return res;
		}
		void run() {
			string mess;
			if(create_socket() == 0){
				while(1){
					mess = accept_socket();
					if(!mess.empty()){
						size_t pos ;
			
						pos = mess.find("__");
			
						if (pos != string::npos) {
							mess = mess.erase(pos,mess.length()-pos);	
						}
						my_data->mtx.lock();
						my_data->data.push_back(mess);
						my_data->mtx.unlock();
			
						if(mess == "exit") {
							break;	
						}
					}
				}
			}
		}
		void operator()() {
			run();
		}
		void close_client_socket(int& socket ) {
			shutdown(socket,2);
			close(socket);
		}
		~hp_unix_socket(){}
	private:
		shared_ptr<hp_gui_data_t> my_data;
		string my_socket_name;
		int my_socket_type;
		int my_socket_fg;
		struct sockaddr_un m_server_unixaddress;
		struct sockaddr_un client_unixaddress;
};

#endif
/*
	struct sockaddr_in m_server_address;
	struct sockaddr_in m_client_address;
	
	struct sockaddr_un m_server_unixaddress;
	struct sockaddr_un m_client_unixaddress;

	socklen_t m_server_len;
	socklen_t m_client_len;
	*/
