#ifndef HP_MODBUS_DATA_H
#define HP_MODBUS_DATA_H

#include <vector>
#include <iostream>

#define BUF_MAX 100

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

using namespace std;

class hp_modbus_data {
	public:
		hp_modbus_data(string master_ip, int master_port, int start_address_dec, unsigned char function_code, int coils_num,unsigned char slave_address=1, unsigned char *data = NULL, int data_len=0);
		char *create_send_data(uint16_t frame_id, int *data_lenght);
		string get_master_ip() {return my_master_ip;};
		int get_master_port() {return my_master_port;};
		int get_start_address() {return my_start_address_dec;};
		int get_funct_code() {return my_funct_code;};
		int get_coils_num() {return my_coils_num;};
		int get_data_len() { return my_data_len; }
		unsigned char *get_rx_ptr() { return my_rx_data;}
		void free_rx_data();

	private:
		string my_master_ip;
		int my_master_port;
		int my_start_address_dec;
		int my_funct_code;
		int my_coils_num;
		unsigned char my_slave_address;
		unsigned char *my_rx_data;
		int my_data_len;


		unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);

		void print_packet(char *data, int len);

};
#endif
