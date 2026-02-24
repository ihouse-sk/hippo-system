#ifndef DALI_GUI_LISTENER_H
#define DALI_GUI_LISTENER_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>
#include <vector>

#include "ih_server_socket.h"

#ifndef HP_DEFAULTS_H
#include "../../hp_lib/include/hp_defaults.hpp"
#endif

using namespace std;

class hp_gui_listener {
	public:
		hp_gui_listener(int port, hp_gui_data_t *);
		int init();
		void operator()();
		~hp_gui_listener();
	private:
		int my_port;
		int my_socket;
		ih_server_socket *gui_socket;
		hp_gui_data_t *my_gui_data;
};

#endif
