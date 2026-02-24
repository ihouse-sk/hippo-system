#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/un.h>

#define MAX_MSG 1000
#define CHECK() fprintf(stderr, "%s:%d:check\n", __FILE__, __LINE__)

class ih_server_socket {

public:
	ih_server_socket(int port, int domain, int type, int protocol ,int num_front = 5); //constructor for AF_INET type socket
	ih_server_socket(int domain, int type, int protocol, std::string listen_on ,int num_front = 5); //constructor for AF_UNIX type socket

	~ih_server_socket();
	
	//vytvara socket, pomenuje ho - vyplni struktury sockaddr, 
	//bind a listen - defaultny pocet fronty 5, da sa nastavit v konstruktore
	void create_socket();
	std::string accept_socket();
	
	//vrati privatnu premennu m_server_socket - identifikator socketu
	int get_server_socket();
	void close_client_socket();
	void close_server_socket();
	int counter;
private:
	int m_server_sockfd;
	int m_client_sockfd;
	int m_port; //port for listening and writing
	int m_domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	int m_type; //SOCK_STREAM, SOCK_DGRAM
	int m_protocol; //default protocol 0 
	int m_num_front; //parameter pre funkciu listen - definuje kolko bude fronta socketov, default = 5;
	std::string m_listen_on;
	
	//sockaddr - adresy socketov
	/*
	*	struct sockaddr_un {
	*		sa_family_t sun_family; //AF_UNIX
	*		char sun_path[]; //nazov cesty
	*	};
	*
	*	struct sockaddr_in {
	*		short int sin_family; //AF_INET
	*		unsigned short int sin_port //port number
	*		struct in_addr in_addr sin_addr //internet addres
	*	};
	*
	*	struct in_addr {
	*		unsigned long int s_addr;
	*	};
	*/
	
	struct sockaddr_in m_server_address;
	struct sockaddr_in m_client_address;
	
	struct sockaddr_un m_server_unixaddress;
	struct sockaddr_un m_client_unixaddress;

	socklen_t m_server_len;
	socklen_t m_client_len;
};
