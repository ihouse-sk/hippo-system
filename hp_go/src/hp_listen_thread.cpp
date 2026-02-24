#include "../include/hp_listen_thread.h"

hp_listen_thread::hp_listen_thread(hp_incoming *data, std::string cls_socket)
{
	m_data_incoming = data;
	m_cls_socket = cls_socket;
}
hp_listen_thread::~hp_listen_thread()
{

}
void hp_listen_thread::operator()()
{
	hp_server_socket serv_socket(AF_UNIX,SOCK_STREAM,0,m_cls_socket);

	serv_socket.create_socket();
//	for (int i=0;i<120;i++) std::cout<<"*";
//	std::cout<<"\n\t\t\t\t\t\tWAITING...\n";
	std::string mess;
	while(1) {
		mess = serv_socket.accept_socket();
		
		if ((!mess.empty()))   {
			//int pos = mess.find_last_not_of('_',998);
			int pos = -1;

			pos = mess.find("__");

			if (pos != (int)std::string::npos) {
				mess = mess.erase(pos,mess.length()-pos);	
			}
			m_data_incoming->mtx_incomming.lock();
			m_data_incoming->mess.push_back(mess);
			m_data_incoming->mtx_incomming.unlock();

			if(mess == "exit") {
				break;	
			}
		}
	}
}
