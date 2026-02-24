#include <iostream>
#include <math.h>

#ifndef API_UART_C
#define API_UART_C
#include "api_uart.h"
#endif

#ifndef LOG_CLASS_C
#define LOG_CLASS_C
#include "log_class.h"
#endif

#ifndef COMM_DATA_C 
#define COMM_DATA_C
#include "comm_data.h"
#endif

#define SLEEP_TIME_THREAD 5000

#define XBEE_START_FLAG 0x7e
#define PACKET_MAX 100

#define STATUS_OK		0x00
#define STATUS_ERROR		0x01
#define INVALID_COM		0x02
#define INVALID_PAR		0x03
#define NO_RESPONSE		0x04

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

#define SETUP_PINS		0x30 // asi to dam prec toto, aj tak sa to nepouziva...
#define CHANGE_PINS		0x31 //ok
#define CHANGE_BLIND_TIME	0x32
#define	ENABLE_INPUT		0x34 //ok
#define	CHANGE_RGB		0x35
#define	CHECK_PORTS		0x36 //ok
#define	SETUP_SELF		0x37 // ok
#define SELF_MESS		0x40

#define MESS_U_START		0x01
#define MESS_DI_CHANGED		0x02
#define MESS_LIFE_STATUE 	0x03
#define MESS_CONS_DATA	 	0x04
#define MESS_SHT31	 	0x05
#define MESS_CARD	 	0x06
#define MESS_DALLAS_MAC		0x07	
#define MESS_DALLAS_TEMP	0x08
#define MESS_SELF_VALUES	0x09
#define MESS_DIR_PIN		0x0a

typedef struct{
	unsigned char start_frame;
	unsigned char dummy;
	unsigned char frame_lenght;
	unsigned char frame_type;
	unsigned char xbee_address[11];
	unsigned char *data;
} packet_data_t;

class xbee_listen
{
	public:
		xbee_listen(api_uart *c_api_uart,std::string,log_class*,comm_data*);
		void operator()();

	private:
		api_uart *m_api_uart;
		comm_data *m_comm_data;
		unsigned char *m_rcv_packet;
		FILE *m_frw;
		int m_aes_mess;
		int m_sockfd;
		std::string m_control_socket_name;
		log_class *m_log_class;
		int *m_thread_status;
		unsigned char t[100];

		void print_packet(unsigned char*);
		void free_rcv_packet();
		void free_buffers(unsigned char *buff1,char *buff2, char *buff3);
		int wait4data(int data_size=0);
		unsigned char *proccess_at_com_res(packet_data_t);
		unsigned char *proccess_remote_at_res(packet_data_t);
		unsigned char *proccess_rx_packet(packet_data_t);
		unsigned char *proccess_rx_packet2(packet_data_t);
		unsigned char *proccess_tx_status(packet_data_t);
		unsigned char *proccess_modem_status(packet_data_t);
		int send_packet(unsigned char*);
		char *display_temperature(unsigned int temp2write, int resolution = 12);
		char *sht_calc_temp(unsigned int value);
		char *sht_calc_hum(unsigned int value);
};

