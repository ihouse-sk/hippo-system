#include <iostream>
#include <vector>
#include <mutex>
#include "hp_server_socket.h"


#ifndef HP_INCOMING_S
#define HP_INCOMING_S
typedef struct hp_incoming {
	std::vector<std::string> mess;
	std::mutex mtx_incomming;	
}hp_incoming;
#endif

#ifndef HP_LISTEN_THREAD_H
#define HP_LISTEN_THREAD_H
class hp_listen_thread{
public:
	hp_listen_thread(hp_incoming *data, std::string cls_socket);
	~hp_listen_thread();
	void operator()();

private:
	hp_incoming *m_data_incoming;
	std::string m_cls_socket;
};
#endif

