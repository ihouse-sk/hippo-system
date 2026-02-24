#include <iostream>
#include <vector>
#include "xmlParser.h"
#include <stdlib.h>

#ifndef API_UART_C
#define API_UART_C
#include "api_uart.h"
#endif

#ifndef LOG_CLASS_C
#define LOG_CLASS_C
#include "log_class.h"
#endif

#ifndef XBEE_DEV_C
#define XBEE_DEV_C
#include "xbee_dev.h"
#endif

#ifndef COMM_DATA_C 
#define COMM_DATA_C
#include "comm_data.h"
#endif


#define MESS_SIZE 	5	
#define BUF_LEN 	100
#define PACKET_MAX 	100
#define READ_PAR	-1

#define PACKET_HEADER 		0
#define PACKET_LEN_MSB		1
#define PACKET_LEN_LSB 		2
#define PACKET_IDENT 		3
#define PACKET_FRAMEID		4
#define PACKET_OPTIONAL_BIT 	15

#define MODEM_STATUS		0x8A
#define	AT_COM			0x08
#define AT_COM_QUEUE		0x09
#define AT_COM_RES		0x88
#define	TX_REQ			0x10
#define EX_AD_COM		0x11
#define	TX_STATUS		0x8B
#define	RX_PACKET_64		0x90
#define EX_RX_INDICATOR		0x91
#define ND_IDENT		0x95
#define	REMOTE_AT_COM		0x17
#define	REMOTE_AT_RES		0x97

typedef struct {
	char at_comm[3];
	int min_value;
	int max_value;
	int default_value;
} ih_command_t;

class control_listen
{
	public:
		control_listen(api_uart *c_api_uart,std::string,log_class*,comm_data*);
		int init();
		void operator()();

	private:
		api_uart *m_api_uart;
		comm_data *m_comm_data;
		int m_sockfd;
		std::string m_socket_name;
		unsigned char *m_snd_data;
		char m_at_command[2];
		std::string m_uc_data;
		log_class *m_log_class;
		std::vector<ih_command_t> m_valid_command;
		int m_present_parameter;
		int *m_thread_status;
		unsigned char m_remote_setting;
		unsigned char *m_aes_key;

		int check_xbee_name(int);
		int send_packet(std::vector<std::string>, int);
		int resolve_data(char *);
		int check_xbee_command(const char *);
		int create_packet(int, int frame_id = 0xa2);
		void free_buffers(unsigned char *buff1,char *buff2, char *buff3);

};

