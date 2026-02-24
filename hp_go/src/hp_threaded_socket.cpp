#include "../include/hp_threaded_socket.h"

using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

hp_threaded_socket::hp_threaded_socket(int port, std::string ip,std::string mess, int type,int delay,bool use_https)
{
	my_domain = AF_INET; //domain : AF_INET, AF_UNIX, AF_ISO, AF_XNS, AF_IPX, AF_APPLETALK
	my_type = SOCK_STREAM; //SOCK_STREAM, SOCK_DGRAM
	my_protocol =  0; //default protocol 0 
	my_sockfd = -1;
	my_lifetime_type = LIFETIME_ONE;
	my_mess_type = type;

	my_port = port;
	my_ip = ip;
	my_mess = mess;
	my_delay = delay;
	my_use_https = use_https;
	//std::cout <<"Vytvaram threaded socket, ip: " << my_ip << " port: " << my_port << " mess: " << mess << std::endl;
}

void hp_threaded_socket::operator()()
{
	if(my_mess_type == THREAD_SOCKET){
		send_socket(my_mess);
	}
	if(my_mess_type == THREAD_CURL){
		//process_url(my_mess);
	}
	if(my_mess_type == THREAD_ASIO){
		process_url_asio();
	}
}

string hp_threaded_socket::process_url_asio()
{
	if(my_delay != 0){
		std::this_thread::sleep_for(chrono::seconds(my_delay));
	}
	cout <<"Starting asio request: " << my_mess<< endl;
	try {
		string ip = "";
		if(my_use_https){
			HttpsClient client(my_ip+":"+patch::to_string(my_port),false); // false znaci, ze neoveruje certifikat
			
			auto r1 = client.request("GET",my_mess);
			if(r1->status_code.find("20") == std::string::npos){
				throw r1->content.string();
			} else {
				cout << r1->content.string() << endl;
			}
		}
	} catch (const SimpleWeb::system_error &e) {
		hp_logger logger(".", true);
		logger.log_mess("Request: "+my_mess+"\nError: "+ string(e.what()));
	}
	return "";
}




int hp_threaded_socket::send_socket(std::string message)
{
	char buffer[MAX_MSG];
	int res = -1;
	unsigned int i;
	const char *listen_on = my_ip.c_str();
	for (i=0;i<MAX_MSG;i++) {
		if (i< message.length()) {
		 	buffer[i] = message[i];
		}else {	
			buffer[i] = '_';
		}
	}
	buffer[MAX_MSG-1] = '\0';

	
	my_sockfd = socket(my_domain, my_type, my_protocol);
	if (my_sockfd < 0) {
		std::cerr << "cannot create listen socket"<<std::endl; 
		res = -1;
		return res;
	}
		
	if (my_domain == AF_INET) {
		m_address.sin_family = my_domain;
		m_address.sin_addr.s_addr = inet_addr(listen_on);
		m_address.sin_port = htons(my_port);
		my_len = sizeof(m_address);
		
		res = connect(my_sockfd, (struct sockaddr *)&m_address, my_len);
		if (res == -1) {
			std::cout<<"error connect to af_inet socket m_address = "<<listen_on<<" m_port = "<<my_port<<std::endl;	
		}
	} 
	if(res != -1){
	//	std::cout << "Posielam spravu z threaded socketu: " << buffer << std::endl;
		write(my_sockfd, &buffer, MAX_MSG);
	}
	shutdown(my_sockfd,2);
	close(my_sockfd);
	return 0;
}

hp_threaded_socket::~hp_threaded_socket()
{
}


