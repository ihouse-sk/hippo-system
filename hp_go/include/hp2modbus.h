#ifndef HP2MODBUS_H
#define HP2MODBUS_H

#include <algorithm>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <jsoncpp/json/json.h>
#include <memory>
#include <math.h>

#include <math.h>

#ifndef HP2MODBUS_DATA_H
#include "hp2modbus_data.h"
#endif

#ifndef HP2MODBUS_DEVICES_H
#include "hp2modbus_devices.h"
#endif

#ifndef HP2MODBUS_SLAVE_H
#include "hp2modbus_slave.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#define READ_HOLDING_REGISTER 0x03
#define READ_INPUT_REGISTER 0x04
#define WRITE_SINGLE_REGISTER 0x06
#define WRITE_MULTIPLE_REGISTER 0x10

#define TIMEOUT 20
#define SLEEP_TIME_THREAD 5000

typedef struct {
	vector<hp2modbus_data> my_data;
	mutex mtx;
} hp2mdb_data_t;

using namespace std;

class hp2modbus {
	public:
		hp2modbus(hp2mdb_data_t *,hp2mdb_data_t *, XMLNode node);
		string process_db(MYSQL *conn);
		Json::Value get_shm_data();
		void operator()();
	private:
		hp2mdb_data_t *my_tx_data;
		hp2mdb_data_t *my_rx_data;
		vector<hp2modbus_devices> my_devices;
	//	vector<hp2modbus_slave> my_slaves;
		vector<std::shared_ptr<hp2modbus_slave> > my_slaves;
		string send_mess(hp2modbus_data,std::shared_ptr<hp2modbus_slave>);
		int execute_query(string query, MYSQL *conn = NULL);


		int my_socket;
		bool my_modbus_log;
		struct sockaddr_in my_address;

		double ConvertNumberToFloat(unsigned long number, int isDoublePrecision);
		void push_resp_data(hp2modbus_data data);
		void process_response(std::shared_ptr<hp2modbus_slave>slave, unsigned char *buffer);
		void process_response_tcp(std::shared_ptr<hp2modbus_slave>slave, unsigned char *buffer);
		char *create_send_data(std::shared_ptr<hp2modbus_slave>slave, string to_value, int *len, string master_type);
		unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);
		void print_packet(char *data, int len, int direction = 0);
		int wait4data(int data_size=0);
		int ioctl_data();
};

#endif
