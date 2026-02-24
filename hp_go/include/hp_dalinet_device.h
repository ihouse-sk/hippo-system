#ifndef DALI_DEVICE_H
#define DALI_DEVICE_H

#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unistd.h>
#include <thread>
#include <mutex>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#define TIMEOUT 20
#define SLEEP_TIME_THREAD 5000
#define BUF_MAX 100

#define DALI_CMD 		0x01
#define DALI_CMD_RESPONSE	0x03

#define CONN_ACTIVE 		0x01
#define CONN_INACTIVE 		0x02

#define DALI_LIGHT_OFF_VALUE		0x00
#define DALI_LIGHT_UP			0x01
#define DALI_LIGHT_DOWN			0x02
#define DALI_LIGHT_STEP_UP		0x03
#define DALI_LIGHT_STEP_DOWN		0x04
#define DALI_LIGHT_MAX_VALUE		0x05
#define DALI_LIGHT_MIN_VALUE		0x06
#define DALI_LIGHT_ONSTEPUP		0x08
#define DALI_SCENE_START		0x10
#define DALI_SCENE_END			0x1f
#define DALI_LIGHT_GET_VALUE		0xA0
#define DALI_LIGHT_GET_BALLAST		0x91
#define DALI_LIGHT_VALUE 		

#define BASE_DEC 0
#define BASE_HEX 1

#define CHECK() fprintf(stderr, "%s:%d:check\n", __FILE__, __LINE__)

using namespace std;

#ifndef DALI_DATA_S
#define DALI_DATA_S

typedef struct {
	vector<string> dali_commands;
	vector<string> dali_responses;
	mutex mtx;
	bool finish;
} dali_data_t;
#endif

typedef struct {
	char *buffer;
	time_t send_time;
	int interval;
	int resend_counter;
	string mess;
} control_data_t;

class dalinet_device {

	public:
		dalinet_device(string, int, dali_data_t *, string my_name);
		string get_name() { return my_name; }
		int init();
		void operator()();
	private:
		dali_data_t *my_comm_data;
		string my_dali_ip;
		string my_name;
		int my_dali_port;
		int my_socket;
		int my_conn_status;
		struct sockaddr_in m_address;
		vector<control_data_t> my_control_data;

		int reset_dali_connection(string reason);
		int wait4data(int data_size=0);
		int xb_ioctl_data();
		char *create_send_data(string command);
		string process_mess(char *data);
		char get_char(char source, int pos);
		char translate_command(string cmd);
		string translate_resp_value(unsigned char resp_value);
};


#endif
