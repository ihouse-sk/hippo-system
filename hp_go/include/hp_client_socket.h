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
#define MAX_MSG 1000 

#ifndef HP_CLIENT_SOCKET_H
#define HP_CLIENT_SOCKET_H
class hp_client_socket {
public:
	hp_client_socket(int port, int domain, int type, int protocol, std::string listen_on);
	hp_client_socket(int domain, int type, int protocol, std::string listen_on);

	~hp_client_socket();
//	void create_socket();
	int send_socket(std::string message);

private:
	int m_port; //port for listening and writing
	int m_domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	int m_type; //SOCK_STREAM, SOCK_DGRAM
	int m_protocol; //default protocol 0 
	int m_sockfd;
	int m_len;
	struct sockaddr_in m_address;
	struct sockaddr_un m_unix_address;
	std::string m_listen_on;
};
#endif
