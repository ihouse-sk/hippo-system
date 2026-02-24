#include "../include/ih_server_socket.h"

ih_server_socket::ih_server_socket(int port, int domain, int type, int protocol, int num_front/* = 5*/)
{
	if (domain != AF_INET) {
		std::cout<<"for using this constructor only unix socket (AF_INET)\n";
		exit(1);
	}

	m_port = port;
	m_domain = domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	m_type = type; //SOCK_STREAM, SOCK_DGRAM
	m_protocol =  protocol; //default protocol 0 
	m_num_front = num_front;
	
	counter = 0;
}
ih_server_socket::ih_server_socket(int domain, int type, int protocol, std::string listen_on, int num_front/* = 5*/)
{
	if (domain != AF_UNIX) {
		std::cout<<"for using this constructor only unix socket (AF_UNIX)\n";
		exit(1);
	}
	m_domain = domain; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	m_type = type; //SOCK_STREAM, SOCK_DGRAM
	m_protocol =  protocol; //default protocol 0 
	m_num_front = num_front;
	m_listen_on = listen_on;	
	
	unlink(m_listen_on.c_str());

	counter = 0;
}

ih_server_socket::~ih_server_socket()
{

}
int ih_server_socket::get_server_socket()
{
	return m_server_sockfd;
}
void ih_server_socket::create_socket()
{
	m_server_sockfd = socket(m_domain, m_type, m_protocol);	
	if (m_server_sockfd < 0) { 
		std::cerr << "cannot create listen socket"<<std::endl; 
		exit(1); 
	} 

	//naming socket
	if (m_domain == AF_INET) {
		for(int i=0; i<20; i++){
			m_server_address.sin_family = m_domain;
			m_server_address.sin_addr.s_addr = htonl(INADDR_ANY);
			m_server_address.sin_port = htons(m_port);
			m_server_len = sizeof(m_server_address);
			if (bind(m_server_sockfd, (struct sockaddr *)&m_server_address, m_server_len) < 0) {
				std::cerr << "cannot bind socket"<<" on port = "<<m_port<<std::endl; 
			} else {
				std::cout << "Pocuvam na porte: " << m_port << "\n";
				break;
			}
			if(i == 19){
				std::cout << "Ukoncujem program...\n";
				exit(1); 
			} else {
				std::cout << "Cakam 20 sekund, potom skusim znova\n";
			}
			sleep(20);
		}
	} else if (m_domain == AF_UNIX){
		unlink("control_layer_socket");
		m_server_unixaddress.sun_family = m_domain;
		//strcpy(m_server_unixaddress.sun_path,"/home/laufem/iHouse/vyvoj/thread/control_layer_socket");
		strcpy(m_server_unixaddress.sun_path,m_listen_on.c_str());
		m_server_len = sizeof(m_server_unixaddress);
		if (bind(m_server_sockfd, (struct sockaddr *)&m_server_unixaddress, m_server_len) < 0) {
			std::cerr << "cannot bind unix socket"<<std::endl; 
			exit(1); 
		}
	}
	// Vytvoříme frontu spojení a začneme čekat na klienty:
    	listen(m_server_sockfd, m_num_front);
}
std::string ih_server_socket::accept_socket()
{
        char buffer[MAX_MSG];
	std::string data;
	std::string comm;
	int i;	

	memset(buffer,0x0,MAX_MSG);
	
	if (m_domain == AF_INET) {
		m_client_len = sizeof(m_client_address);
		m_client_sockfd = accept(m_server_sockfd, (struct sockaddr *)&m_client_address, &m_client_len);
	}else if (m_domain == AF_UNIX) {
		m_client_len = sizeof(m_client_unixaddress);
		m_client_sockfd = accept(m_server_sockfd, (struct sockaddr *)&m_client_unixaddress, &m_client_len);
	}
	if (m_client_len < 0) {
		std::cerr << "cannot accept connection"<<std::endl; 
		exit(1); 
	}
	
	
	recv(m_client_sockfd, buffer, MAX_MSG, 0);
	
	close_client_socket();

	for (i=0;i<MAX_MSG; i++) {
		data+=buffer[i];
	}
	return data;
}
void ih_server_socket::close_client_socket()
{
	shutdown(m_client_sockfd,2);
	close(m_client_sockfd);
}
void ih_server_socket::close_server_socket()
{	
	shutdown(m_server_sockfd,2);
	close(m_server_sockfd);
}

