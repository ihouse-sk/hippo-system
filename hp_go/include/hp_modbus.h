#ifndef HP_MODBUS_H
#define HP_MODBUS_H

#include <vector>
#include <iostream>
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
#include <math.h>

#define READ_HOLDING_REGISTER 0x03
#define READ_INPUT_REGISTER 0x04
#define WRITE_SINGLE_REGISTER 0x06
#define WRITE_MULTIPLE_REGISTER 0x10

#define TIMEOUT 20
#define SLEEP_TIME_THREAD 5000

#ifndef HP_MODBUS_DATA_H
#include "hp_modbus_data.h"
#endif

#ifndef MODBUS_DEVICES_S
#define MODBUS_DEVICES_S
typedef struct {
	string ident;
	string type;
	string ip;
	int port;
} modbus_devices_t;
#endif
	
typedef struct {
	vector<hp_modbus_data> my_data;
	mutex mtx;
} hp_mdb_data_t;

using namespace std;

class hp_modbus {
	public:
		hp_modbus(hp_mdb_data_t *,hp_mdb_data_t *);
		void operator()();
	private:
		int my_socket;
		struct sockaddr_in my_address;
		hp_mdb_data_t *my_tx_data;
		hp_mdb_data_t *my_rx_data;

		string send_mess(hp_modbus_data);
		int wait4data(int data_size=0);
		int ioctl_data();
};
#endif
