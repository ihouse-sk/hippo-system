#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/un.h>
#include <string>
#include <iostream>
#include <vector>
#include "boost/lexical_cast.hpp"
#include <thread>
#include <fstream>

using namespace std;

struct Bug_socket {
	string socket_name;
	int sd;
	struct sockaddr_un m_unix_address;
	int send_data(string mess) {
		sd = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sd <0) {
			cout <<"Chyba socketu" << endl;
			return sd;
		}
		m_unix_address.sun_family = AF_UNIX;
		strcpy(m_unix_address.sun_path,socket_name.data());
		if(connect(sd, (struct sockaddr *)&m_unix_address, sizeof(m_unix_address)) != -1){
			write(sd, mess.data(), sizeof(char)*mess.length());
			shutdown(sd,2);
			close(sd);
			return 0;
		}
		return -1;
	}
};

struct Shared_data {
	vector<int> lines;
	int last_update;
};

struct Bug_listener {
	string socket_name;
	int sd = -1;
	struct sockaddr_un m_server_unixaddress;
	struct sockaddr_un m_client_unixaddress;
	Shared_data *sh;
	Bug_listener(string name, Shared_data *shared_data): socket_name(name), sh(shared_data){};
	void operator()(){
		sh->last_update = time(NULL);
		create_socket();
		while(1){
			accept_socket();
		}
	};
	int create_socket() {
		sh->lines.push_back(0);
		unlink(socket_name.data());
		sd = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sd <0) {
			cout <<"Chyba socketu" << endl;
			return sd;
		}
		m_server_unixaddress.sun_family = AF_UNIX;
		strcpy(m_server_unixaddress.sun_path,socket_name.data());
		if (bind(sd, (struct sockaddr *)&m_server_unixaddress, sizeof(m_server_unixaddress)) < 0) {
			std::cerr << "cannot bind unix socket"<<std::endl; 
			exit(1); 
		}
		listen(sd, 20);
		return 0;
	};
	void accept_socket() {
		char mess[1000];
		socklen_t c_len =  sizeof(m_client_unixaddress);
		int client= accept(sd, (struct sockaddr *)&m_client_unixaddress, &c_len);
		if(client <0) {
			std::cerr << "cannot accept connection"<<std::endl; 
			return;
		}
		int rec_cout = recv(client, mess, 1000, 0);
		mess[rec_cout] = '\0';
		try {
			int line = boost::lexical_cast<int>(mess);
	//		cout << line<< endl;
			if(sh->lines.back() > line){
				//cout <<"Vector size: " << msgs.size() << endl;
				sh->lines.clear();
				sh->last_update = time(NULL);
			}
			sh->lines.push_back(line);
		} catch (boost::bad_lexical_cast const& e) {
                    std::cout << "error" << '\n';
                }
	
		shutdown(client,2);
		close(client);
	};
};

using std::ofstream;

int main() {

	//Bug_listener bl{"/home/user/working_folder/system/hp_go/socket/xbee_socket"};
	Shared_data sh;
	sh.last_update = time(NULL)+10;
	Bug_listener *bl = new Bug_listener("bug_socket", &sh);
	std::thread th2(*bl);
	cout << "Up and running" << endl;
	ofstream fw;

	while(1){
		if(sh.last_update + 5 < time(NULL)){
			fw.open("Bug-"+to_string(time(NULL))+".txt");
			if(fw.is_open()){
				cout <<"Bug" << endl;
				fw<< sh.last_update << " - chyba restart hp_go " <<  endl;
				for(auto &m : sh.lines){
					fw<< m << ",";
				}
				fw << endl;
				fw.close();
			} else {
				cout << sh.last_update << " - chyba restart hp_go " <<  endl;
				for(auto &m : sh.lines){
					cout << m << ",";
				}
				cout << endl;
			}
			system("killall hp_go");
			sh.last_update = time(NULL)+20;

		}
		sleep(1);
	}
	th2.join();
	

	/*
	Bug_socket bgs{"/home/user/working_folder/system/hp_go/socket/xbee_socket"};

	bgs.send_data("Ahoj");
	bgs.send_data(to_string(__LINE__));

	while(1){
		cout <<"Sending staus: " << bgs.send_data("Ahoj") << endl;
		usleep(100000);
	}
	*/
	return 0;
}


