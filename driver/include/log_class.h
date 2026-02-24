#include <iostream>
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
#include <stdarg.h>
#include <boost/thread.hpp>

#define BUF_LEN 	100
#define PACKET_MAX 	100


#define LOG_ERROR 1
#define LOG_COMM 2
#define CHECK() fprintf(stderr, "%s:%d:check\n", __FILE__, __LINE__)

class log_class {

	public:
		log_class(int, std::string,bool);
		void ih_log_write(int, int,const char *format,...);
		void new_day();
		~log_class(){ 
			ih_log_write(LOG_ERROR, 4, "Driver is closing. \n"); 
		}
	private:
		int m_log_com;
		int m_log_error;
		int m_log_verbose;
		int m_sockfd;
		int m_day_number;
		std::string m_control_socket_name;
		int send_packet(unsigned char *socket_buf);
		bool m_source_install;
			
		boost::mutex mtx;
		
};

