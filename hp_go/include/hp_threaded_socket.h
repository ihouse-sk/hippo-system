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
//#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include "../../hp_lib/include/client_https.hpp"
#include "../../hp_lib/include/client_http.hpp"
#include <thread>

#ifndef HP_THREADED_SOCKET_H
#define HP_THREADED_SOCKET_H
#define MAX_MSG 1000 
#define LIFETIME_ONE 0
#define LIFETIME_INF 1

#define THREAD_SOCKET 	0
#define THREAD_CURL	1
#define THREAD_ASIO 	2

#ifndef HP_LOGGER_H
#include "../../hp_lib/include/hp_logger.hpp"
#endif

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

struct MemoryStruct {
  char *memory;
  size_t size;
};

using namespace std;

class hp_threaded_socket {
public:
	hp_threaded_socket(int port, std::string ip, std::string mess,int type = THREAD_SOCKET,int delay =0, bool use_https = true);
	void operator()();
	~hp_threaded_socket();
//	void create_socket();

private:
	int my_port; //port for listening and writing
	int my_domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	int my_type; //SOCK_STREAM, SOCK_DGRAM
	int my_protocol; //default protocol 0 
	int my_sockfd;
	int my_len;
	int my_delay;
	int my_mess_type;
	bool my_use_https;
	struct sockaddr_in m_address;
	struct sockaddr_un m_unix_address;
	std::string my_ip, my_mess;
	bool my_lifetime_type;
	string process_url_asio();

	int send_socket(std::string message);
};
#endif
