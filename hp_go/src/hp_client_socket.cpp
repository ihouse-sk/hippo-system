#include "../include/hp_client_socket.h"


hp_client_socket::hp_client_socket(int port, int domain, int type, int protocol, std::string listen_on)
{
	if (domain != AF_INET) {
		std::cout<<"for using this constructor only unix socket (AF_INET)\n";
		exit(1);
	}
	m_port = port;
	m_domain = domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	m_type = type; //SOCK_STREAM, SOCK_DGRAM
	m_protocol =  protocol; //default protocol 0 
	m_listen_on = listen_on;
	m_sockfd = -1;
}
hp_client_socket::hp_client_socket(int domain, int type, int protocol, std::string listen_on)
{
	if (domain != AF_UNIX) {
		std::cout<<"for using this constructor only unix socket (AF_UNIX)\n";
		exit(1);
	}

	m_domain = domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	m_type = type; //SOCK_STREAM, SOCK_DGRAM
	m_protocol =  protocol; //default protocol 0 
	m_listen_on = listen_on;
	m_sockfd = -1;
}

hp_client_socket::~hp_client_socket()
{

}

int hp_client_socket::send_socket(std::string message)
{
	char buffer[MAX_MSG];
	int res;
	unsigned int i;
	const char *listen_on = m_listen_on.c_str();
	for (i=0;i<MAX_MSG;i++) {
		if (i< message.length()) {
		 	buffer[i] = message[i];
		}else {	
			buffer[i] = '_';
		}
	}
	buffer[MAX_MSG-1] = '\0';

	
	m_sockfd = socket(m_domain, SOCK_STREAM, 0);
	if (m_sockfd < 0) {
		std::cerr << "cannot create listen socket"<<std::endl; 
		res = -1;
		return res;
	}
		
	if (m_domain == AF_INET) {
		m_address.sin_family = m_domain;
		m_address.sin_addr.s_addr = inet_addr(listen_on);
		m_address.sin_port = htons(m_port);
		m_len = sizeof(m_address);
		
		res = connect(m_sockfd, (struct sockaddr *)&m_address, m_len);
		if (res == -1) {
			std::cout<<"error connect to af_inet socket m_address = "<<listen_on<<" m_port = "<<m_port<<std::endl;	
		}
	} else if (m_domain == AF_UNIX){
		m_unix_address.sun_family = m_domain;
		//strcpy(m_address.sun_path,"server_socket");
		strcpy(m_unix_address.sun_path,listen_on);
		m_len = sizeof(m_unix_address);
		res = connect(m_sockfd, (struct sockaddr *)&m_unix_address, m_len);
		if (res == -1) {
			std::cout<<"error connect to af_unix client socket"<<std::endl;	
		}

	}
	if(res != -1){
	//	std::cout << "Posielam spravu: " << buffer << std::endl;
		write(m_sockfd, &buffer, MAX_MSG);
	}
	shutdown(m_sockfd,2);
	close(m_sockfd);
}

