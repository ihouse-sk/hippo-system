#include "../include/hp2modbus_data.h"


hp2modbus_data::hp2modbus_data(string desc, string value, int mess_id, string status)
{
	my_slave_desc = desc;
	my_value = value;
	my_status = status;
	my_mess_id = mess_id;
}

/*

char *hp_modbus_data::create_send_data(uint16_t frame_id, int *data_lenght)
{
	int res_counter=0;
	unsigned short checksum = 0;
	unsigned char *res = (unsigned char *)malloc(BUF_MAX);
	if(res == NULL){
		return (char*)res;
	}
	res[res_counter++] = frame_id >> 8;
	res[res_counter++] = frame_id & 0xff;
	res[res_counter++] = 0x00;
	res[res_counter++] = 0x00;
	res[res_counter++] = 0x00;
	res[res_counter++] = 0x06;///len
	res[res_counter++] = my_slave_address;
	res[res_counter++] = this->my_funct_code;
	res[res_counter++] = my_start_address_dec >> 8;
	res[res_counter++] = my_start_address_dec & 0xff;
	res[res_counter++] = this->my_coils_num >> 8;
	res[res_counter++] = my_coils_num& 0xff;

//	checksum = CRC16(res, res_counter);
//
//	res[res_counter++] = checksum >> 8;
//	res[res_counter++] = checksum & 0xff;
	*data_lenght = res_counter;

	print_packet((char*) res, *data_lenght);
	
	return (char *)res;
}

*/


