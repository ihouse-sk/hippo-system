#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <bitset>
#include <boost/thread.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef LOG_CLASS_C
#define LOG_CLASS_C
#include "log_class.h"
#endif

#define SLEEP_TIME 50000
#define TIMEOUT	20
#define CHECK() fprintf(stderr, "%s:%d:check\n", __FILE__, __LINE__)
#define DEBUG 0


class api_uart {
	public:
		api_uart(log_class*);
		int xb_init();
		int xb_read_data(unsigned char *packet, int data_size);
		int xb_write_data(unsigned char *packet, int data_size);
		int xb_ioctl_data();
		void clear_input();
		void xb_close_serial();
	private:
		log_class *m_log_class;
		int m_socket;
		int m_socket_state;
		std::string m_device_name;
		boost::mutex mtx;

		int xb_open_serial(std::string );
		bool check_xbee_baud();
		void setup_xbee_at();
};
