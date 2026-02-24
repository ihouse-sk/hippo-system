#include "../include/hp_gui_listener.h"



hp_gui_listener::hp_gui_listener(int port,hp_gui_data_t *data): my_port(port), my_gui_data(data)
{
}


int hp_gui_listener::init()
{
	gui_socket = new ih_server_socket(my_port,AF_INET,SOCK_STREAM,0);

	return 0;
}

void hp_gui_listener::operator()()
{
	string mess;
	gui_socket->create_socket();

	while(1) {
		mess = gui_socket->accept_socket();
		if ((!mess.empty())) {
			//std::cout<<"icnomming mess = "<<mess<<std::endl;
			my_gui_data->mtx.lock();
			my_gui_data->data.push_back(mess);
			my_gui_data->mtx.unlock();
		}
	}
}
hp_gui_listener::~hp_gui_listener()
{
	delete gui_socket;

}
