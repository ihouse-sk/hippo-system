#include "../include/xbee_listen.h"

xbee_listen::xbee_listen(api_uart *c_api_uart,std::string socket_name,log_class *c_log_class, comm_data *c_comm_data)
{
	m_api_uart = c_api_uart;
	m_control_socket_name =socket_name; 
	m_log_class = c_log_class;
	m_comm_data = c_comm_data;
/*

	unsigned char cout=0;
	t[cout++] = 0x7e;
	t[cout++] = 0x00;
	t[cout++] = 0x14;
	t[cout++] = 0x90;
	t[cout++] = 0x00;
	t[cout++] = 0x13;
	t[cout++] = 0xa2;
	t[cout++] = 0x00;
	t[cout++] = 0x41;
	t[cout++] = 0x66;
	t[cout++] = 0xbc;
	t[cout++] = 0x39;
	t[cout++] = 0xff;
	t[cout++] = 0xfe;

	t[cout++] = 0x01;
	t[cout++] = 0x43;
	t[cout++] = 0x36;
	t[cout++] = 0x30;

	t[cout++] = 0x01;
	t[cout++] = 0x48;
	t[cout++] = 0x30;
	t[cout++] = 0x68;
	t[cout++] = 0xf3;
	t[cout++] = 0x32;
	t[cout++] = 0x5e;
	t[cout++] = 0x2f;
	t[cout++] = 0xa3;

	t[cout++] = 0x01;
	t[cout++] = 0x41;
	t[cout++] = 0x31;
	t[cout++] = 0x28;
	t[cout++] = 0x80;
	t[cout++] = 0x7a;
	t[cout++] = 0x18;
	t[cout++] = 0x06;
	t[cout++] = 0x00;
	t[cout++] = 0x00;
	t[cout++] = 0xd1;
	t[cout++] = 0x02;
	t[cout++] = 0x10;

	t[cout++] = 0x11;
	t[cout++] = '\0';

	packet_data_t *packet_data;
	packet_data = (packet_data_t *)t;
	unsigned char *socket_buf=NULL;
	socket_buf = proccess_rx_packet2(*packet_data);
	printf("socket buf: %s\n", socket_buf);
	*/
}

void xbee_listen::free_buffers(unsigned char *buff1,char *buff2, char *buff3)
{
	if(buff1){
		free(buff1);
		buff1=NULL;
	}
	if(buff2){
		free(buff2);
		buff2=NULL;
	}
	if(buff3){
		free(buff3);
		buff3=NULL;
	}
}

void xbee_listen::free_rcv_packet()
{
	free(m_rcv_packet);
	m_rcv_packet = NULL;
}

int xbee_listen::wait4data(int data_size)
{
	if(data_size > 0){
		//std::cout << "Data size: " << data_size<< std::endl;
	}
	int bytes=0;
	for(int i=0; i< TIMEOUT; i++){
		bytes = m_api_uart->xb_ioctl_data();
		if(bytes > data_size){
			return bytes;
		}
		usleep(SLEEP_TIME_THREAD);
	}
	return bytes;
}

void xbee_listen::print_packet(unsigned char *print)
{
	for(int i=0; i<print[2]+4; i++){
		printf("%02x ",print[i]);
	}
	printf("\n");
}

void xbee_listen::operator()()
{
	int bytes=0,i,checksum=0;
	unsigned char znak;

	bytes = m_api_uart->xb_ioctl_data();
	if(bytes > 0){
		for(i=0; i< bytes; i++){
			m_api_uart->xb_read_data(&znak,1);
		}
	}

	unsigned char *cls_info;
	if((cls_info = (unsigned char *)malloc(2*PACKET_MAX)) != NULL ){
		strcpy((char *)cls_info,"working");
		send_packet(cls_info);
		free(cls_info);
		m_comm_data->set_x_status(1);
	} 

	while(1){
		if(m_comm_data->get_x_status() == -1){
			break;
		}
		bytes = wait4data(1);
		if(bytes > 0){
			m_api_uart->xb_read_data(&znak,1);
			if(znak == XBEE_START_FLAG){
				m_aes_mess = 0;
				if((m_rcv_packet=(unsigned char *)malloc(sizeof(char )*PACKET_MAX)) == NULL){
					m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
					continue;
				}
				m_rcv_packet[0]=znak;
				bytes = wait4data(1);
				if(bytes >= 2){
					if((m_api_uart->xb_read_data(m_rcv_packet+1, 2)) != 2){
						m_log_class->ih_log_write(LOG_ERROR,3,"Error reading %d bytes,%s:%d\n",2,__FILE__,__LINE__);
						free_rcv_packet();
						continue;
					}
				} else {
					m_log_class->ih_log_write(LOG_ERROR,3,"No data, %d bytes,%s:%d\n",2,__FILE__,__LINE__);
					free_rcv_packet();
					continue;
				}
				bytes = wait4data(m_rcv_packet[2]);
				if(bytes >= m_rcv_packet[2]+1){
					if((m_api_uart->xb_read_data(m_rcv_packet+3, m_rcv_packet[2]+1)) != m_rcv_packet[2]+1){
						m_log_class->ih_log_write(LOG_ERROR,3,"Error reading %d bytes,%s:%d\n",m_rcv_packet[2]+1,__FILE__,__LINE__);
						free_rcv_packet();
						continue;
					} else {
						m_rcv_packet[m_rcv_packet[2]+4]= '\0';
					}
				} else {
					m_log_class->ih_log_write(LOG_ERROR,3,"No data, %d bytes,%s:%d\n",2,__FILE__,__LINE__);
					free_rcv_packet();
					continue;
				}
				packet_data_t *packet_data;
				packet_data = (packet_data_t *)m_rcv_packet;
				checksum = 0;
				for(i=0; i<packet_data->frame_lenght; i++){
					checksum += m_rcv_packet[i+3];
				}
				if(m_rcv_packet[packet_data->frame_lenght+3] == (0xff - (checksum & 0xff))){
					char buf[3];
					char *buffer;
					if((buffer = (char *)malloc(300)) != NULL){
						buffer[0] = '\0';
						for(i=0;i<packet_data->frame_lenght+4; i++){
							sprintf(buf,"%02x ",m_rcv_packet[i]);
							strcat(buffer,buf);
						}
						m_log_class->ih_log_write(LOG_COMM, 1, "Recieve data: %s\n",buffer);
						free(buffer);
					}
					if(DEBUG){
						//print_packet(m_rcv_packet);
					}
				} else {
					m_log_class->ih_log_write(LOG_ERROR,3,"Error checksum in read thread, %s:%d\n",__FILE__,__LINE__);
					free_rcv_packet();
					continue;
				}
				unsigned char *socket_buf=NULL;
				if(packet_data->frame_type == AT_COM_RES){
					socket_buf = proccess_at_com_res(*packet_data);
				}
				if(packet_data->frame_type == REMOTE_AT_RES){
					socket_buf = proccess_remote_at_res(*packet_data);
				}
				if(packet_data->frame_type == RX_PACKET_64){
					//socket_buf = proccess_rx_packet2(*packet_data);
				}
				if(packet_data->frame_type == RX_PACKET_64){
					socket_buf = proccess_rx_packet(*packet_data);
				}
				if(packet_data->frame_type == EX_RX_INDICATOR){
					/////////// DOROBIT a zistit ci to vobec treba
				}
				if(packet_data->frame_type == ND_IDENT){
					/////////// DOROBIT a zistit ci to vobec treba
				}
				if(packet_data->frame_type == TX_STATUS){
					socket_buf = proccess_tx_status(*packet_data);
				}
				if(socket_buf != NULL){
					m_comm_data->set_recieve_time(time(NULL));
					if(DEBUG){
	//					std::cout <<"socket_buffer: " <<  socket_buf<< std::endl;
					}
					if(m_aes_mess == 0){
						send_packet(socket_buf);
					} else {
						m_comm_data->add_aes_mess(socket_buf);
						/// PROCESS AES MESSAGE
					}
					free(socket_buf);
					socket_buf = NULL;
				}
				free_rcv_packet();
			} else {
				m_log_class->ih_log_write(LOG_ERROR,3,"Chybny start delimeter, znak: %02x\n",znak);
			}
		} 
	}
}
int xbee_listen::send_packet(unsigned char *socket_buf)
{
	if(socket_buf != NULL){
		struct sockaddr_un address_unix;	
		int len_add,buf_len,result,status1;
		m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		address_unix.sun_family = AF_UNIX;
		strcpy(address_unix.sun_path,m_control_socket_name.c_str());
		len_add = sizeof(address_unix);
	
		if((result = connect(m_sockfd, (struct sockaddr *)&address_unix, len_add)) == -1){
			m_log_class->ih_log_write(LOG_ERROR,3,"Cannot open socket to c++,%s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		if(result == 0){
			buf_len=strlen((char*)socket_buf);
			for(int i=0; i<2*PACKET_MAX-buf_len-1; i++){
				strcat((char*)socket_buf,"_");
			}
			status1 = write(m_sockfd,(char*)socket_buf,2*PACKET_MAX);
			if(DEBUG){
				std::cout <<"socket_buffer: " <<  socket_buf<< " a poslal si: " << status1 << " bajtov" <<std::endl;
			}
			if(status1){
			}
		}
		shutdown(m_sockfd,2);
		close(m_sockfd);
	}
	return 0;
}
unsigned char *xbee_listen::proccess_rx_packet2(packet_data_t packet_data)
{
	unsigned char *res=NULL;
	char *val_buf, *address_buf;
	char buf[6];
	if((res =(unsigned char *)malloc(sizeof(unsigned char)*(2*PACKET_MAX))) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	memset(res,0x0,2*PACKET_MAX);
	if ((address_buf=(char *)malloc(sizeof(char)*40))== NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		free_buffers(res,NULL,NULL);
		return NULL;
	}
	address_buf[0]='\0';
	for(int i =0; i<10; i++){
		sprintf(buf,"%02x",packet_data.xbee_address[i]);
		strcat(address_buf,buf);
	}	

	if((val_buf=(char *)malloc(sizeof(char)*100)) == NULL){
		free_buffers(res,address_buf,NULL);
		return NULL;
	}
	val_buf[0]='\0';

	unsigned char *core_data= m_rcv_packet+14;
	size_t core_size= packet_data.frame_lenght - 11;//strlen((char *) core_data)-1;
	int i;
	bool broadcast = (core_data[0] == 0x02 || core_data[0] == 0xc2);
	bool act_mess = (core_data[1] == 'A') ||(core_data[1] == 'B') ||(core_data[1] == 'C') ||(core_data[1] == 'D') ||(core_data[1] == 'E') ||(core_data[1] == 'R') ||(core_data[1] == 'T') ||(core_data[1] == 'X') ||(core_data[1] == 'E') || (core_data[1] == 'L') || (core_data[1] == 'W') ;
	
	printf("core_size: %d\n", (int)core_size);	

	char *part_data;

	if(act_mess) {
		int mess_type = 0;

		if(core_size == 2) {
			mess_type = MESS_U_START;
		}
		if(core_size == 4) {
			mess_type = MESS_DI_CHANGED;
		}
		if(core_size == 6){
			mess_type = MESS_LIFE_STATUE;
		}
		if(core_size == 8){
			mess_type = MESS_CONS_DATA;
		}
		if(core_size == 9 && core_data[1] == 'H'){
			mess_type = MESS_SHT31;
		}  
		if(core_size == 10 && core_data[1] != 'H'){
			mess_type = MESS_SELF_VALUES;
		}
		if(core_size == 10 && core_data[1] == 'D' && core_data[2] == 'I' ){
			mess_type = MESS_DIR_PIN;
		}
		if(core_size > 9  && core_data[1] == 'C' && core_data[2] == 'A'){
			mess_type = MESS_CARD;
		} else if(core_data[1] == 'D' && core_data[2] == 'S'){
			mess_type = MESS_DALLAS_MAC;
		} else if (core_size == 13){
			mess_type = MESS_DALLAS_TEMP;
		}

		switch (mess_type) {
			case MESS_U_START: {
				/// spracovanie spravy, kde je dlzka uzitocnych dat 2  - sprava od procaka typu: U-cko ( posiela sa po restarte) a W-cko (posiela sa vtedy, ked procak restartje xbee lebo mu neprisla diagnostika)
				sprintf((char *)res,"%s%x_%s_%c_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data[1]);
				break;
			}
			case MESS_DI_CHANGED: {
				/// spracovanie spravy, kde je dlzka uzitocnych dat 4  - zmena stavu piny na procesore
				core_data[4] = '\0';
				sprintf((char *)res,"%s%x_%s_%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf, core_data+1);
				break;
			}
			case MESS_LIFE_STATUE: {
				unsigned int life_time = (core_data[2]*0.0085*255*255*255) + (core_data[3]*0.0085*255*255) + (core_data[4]*0.0085*255) + core_data[5]*0.0085;
				printf("Life time: %d\n",life_time);
				sprintf((char *)res,"%s%x_%s_X%d_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,life_time);
				break;
			}
			case MESS_CONS_DATA: {
				sprintf((char *)res,"%s%x_%s_A%c0%04d_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf, core_data[5]+1, ((core_data[6] << 8)+core_data[7]));
				break;
			}
			case MESS_SHT31: {
				/// spracovanie spravy, kde je dlzka uzitocnych dat 9  - sprava od MHT31 senzora o teplote a vlhkosti
				char *temp_value = sht_calc_temp((core_data[3] <<8) + core_data[4]);
				char *hum_value = sht_calc_hum((core_data[6] <<8) + core_data[7]);
				core_data[3] = '\0';
				sprintf((char *)res,"%s%x_%s_H%s%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf, core_data+2,temp_value, hum_value);
				if(temp_value != NULL){
					free(temp_value);
					temp_value = NULL;
				}
				if(hum_value != NULL){
					free(hum_value);
					hum_value = NULL;
				}
				break;
			}
			case MESS_CARD: {
				int sample_count = (core_size-6);
				part_data = (char*)malloc(sample_count);
				part_data[0] = '\0';
				for(i=0; i<sample_count; i++){
					sprintf(buf,"%02x",core_data[i+6]);
					strcat(part_data,buf);
				}
				core_data[6] = '\0';
				sprintf((char *)res,"%s%x_%s_%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data+1,part_data);
				free(part_data);
				part_data = NULL;
				break;
			}
			case MESS_DALLAS_MAC: {
				char dallas_mac[16];
				dallas_mac[0] = '\0';
				for(i=0; i<8; i++){
					if(core_data[i+4] != 0){
						sprintf(buf,"%02x",core_data[i+4]);
						strcat(dallas_mac,buf);
					}
				}
				core_data[4] = '\0';
				sprintf((char *)res,"%s%x_%s_%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data+1,dallas_mac);
				break;
			}
			case MESS_DALLAS_TEMP: {
				/// spracovanie spravy, kde je dlzka uzitocnych dat 13  - sprava od dallas teplomera
				char *temp_value = display_temperature((core_data[11] << 8)+core_data[12]);
				char dallas_mac[16];
				dallas_mac[0] = '\0';
				for(i=0; i<8; i++){
					if(core_data[i+3] != 0){
						sprintf(buf,"%02x",core_data[i+3]);
						strcat(dallas_mac,buf);
					}
				}
				sprintf((char *)res,"%s%x_%s_L1%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,dallas_mac, temp_value);
				if(temp_value != NULL){
					free(temp_value);
					temp_value = NULL;
				}
				break;
			}
			case MESS_SELF_VALUES: {
				// 42 32 09 01 03 ff ff ff 00
				char pin[3];
				pin[0] = core_data[1];
				pin[1] = core_data[2];
				pin[2] = '\0';
				sprintf((char *)res,"%s%x_%s_%s,%d,%d,%d,%d,%d,%d,%d_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,pin,core_data[3],core_data[4],core_data[5],core_data[6],core_data[7],core_data[8],core_data[9]);
				break;
			}
			case MESS_DIR_PIN: {
				break;
			}
			default: {
				sprintf((char *)res,"%s%x_%s_messageUnknown_error",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf);
				break;
			}
		}

/*
		if(core_size == 6){
			unsigned int life_time = (core_data[2]*0.0085*255*255*255) + (core_data[3]*0.0085*255*255) + (core_data[4]*0.0085*255) + core_data[5]*0.0085;
			printf("Life time: %d\n",life_time);
			sprintf((char *)res,"%s%x_%s_X%d_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,life_time);
		} else if(core_size == 4){
			/// spracovanie spravy, kde je dlzka uzitocnych dat 4  - zmena stavu piny na procesore
			core_data[4] = '\0';
			sprintf((char *)res,"%s%x_%s_%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf, core_data+1);
		} else if(core_size == 9) {
			/// spracovanie spravy, kde je dlzka uzitocnych dat 9  - sprava od MHT31 senzora o teplote a vlhkosti
			char *temp_value = sht_calc_temp((core_data[3] <<8) + core_data[4]);
			char *hum_value = sht_calc_hum((core_data[6] <<8) + core_data[7]);
			core_data[3] = '\0';
			sprintf((char *)res,"%s%x_%s_H%s%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf, core_data+2,temp_value, hum_value);
			if(temp_value != NULL){
				free(temp_value);
				temp_value = NULL;
			}
			if(hum_value != NULL){
				free(hum_value);
				hum_value = NULL;
			}
		} else if(core_size == 8){
			/// spotreba
			sprintf((char *)res,"%s%x_%s_A%c0%04d_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf, core_data[5]+1, ((core_data[6] << 8)+core_data[7]));
		} else if(core_size > 9 && core_data[1] == 'C' && core_data[2] == 'A') {
			int sample_count = (core_size-6);
			part_data = (char*)malloc(sample_count);
			part_data[0] = '\0';
			for(i=0; i<sample_count; i++){
				sprintf(buf,"%02x",core_data[i+6]);
				strcat(part_data,buf);
			}
			core_data[6] = '\0';
			sprintf((char *)res,"%s%x_%s_%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data+1,part_data);
			free(part_data);
			part_data = NULL;
		} if(core_size == 10 && core_data[1] == 'D' && core_data[2] == 'I') {
			// DIRECT PIN ... to chcem ale zrusit
			core_data[10] = '\0';
			sprintf((char *)res,"%s%x_%s_%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data+1);
		} else if (core_size == 12 && core_data[1] == 'D' && core_data[2] == 'S'){
			char dallas_mac[16];
			dallas_mac[0] = '\0';
			for(i=0; i<8; i++){
				if(core_data[i+4] != 0){
					sprintf(buf,"%02x",core_data[i+4]);
					strcat(dallas_mac,buf);
				}
			}
			core_data[4] = '\0';
			sprintf((char *)res,"%s%x_%s_%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data+1,dallas_mac);
		} else if (core_size == 13){
			/// spracovanie spravy, kde je dlzka uzitocnych dat 13  - sprava od dallas teplomera
			char *temp_value = display_temperature((core_data[11] << 8)+core_data[12]);
			char dallas_mac[16];
			dallas_mac[0] = '\0';
			for(i=0; i<8; i++){
				if(core_data[i+3] != 0){
					sprintf(buf,"%02x",core_data[i+3]);
					strcat(dallas_mac,buf);
				}
			}
			sprintf((char *)res,"%s%x_%s_L1%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,dallas_mac, temp_value);
			if(temp_value != NULL){
				free(temp_value);
				temp_value = NULL;
			}
		} else if(core_size == 2){
			/// spracovanie spravy, kde je dlzka uzitocnych dat 2  - sprava od procaka typu: U-cko ( posiela sa po restarte) a W-cko (posiela sa vtedy, ked procak restartje xbee lebo mu neprisla diagnostika)
			sprintf((char *)res,"%s%x_%s_%c_ok",broadcast?"drv_rec_brc_":"drv_rec_act_",m_rcv_packet[4],address_buf,core_data[1]);
		} 
	*/
	} else {
		if(core_size == 2){
			sprintf((char *)res,"%s%x_%s_%c_ok",broadcast?"drv_rec_brc_":"drv_rec_ack_",m_rcv_packet[4],address_buf,core_data[1]);
		}
		if(core_size > 2){
			if(core_data[2] == CHANGE_PINS){
				// spracovanie odpovede procesora pri zmene pinu
				int sample_count = (core_size-3)/3;
				part_data = (char*)malloc(sample_count*5);
				part_data[0] = '\0';
				for(i=0; i<sample_count; i++){
					sprintf(buf,"%c%c%03d",core_data[i*3+3],core_data[i*3+4],core_data[i*3+5]);
					strcat(part_data,buf);
				}
				core_data[3] = '\0';
				sprintf((char *)res,"%s%x_%s_%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_ack_",m_rcv_packet[4],address_buf,core_data+1,part_data);
				free(part_data);
				part_data = NULL;
			}
			if(core_data[2] == CHECK_PORTS || core_data[2] == ENABLE_INPUT){
				// spracovanie info o stave pinu alebo vypnutie/zapnutie vstupu
				int sample_count = (core_size-3)/3;
				part_data = (char*)malloc(sample_count*5);
				part_data[0] = '\0';
				for(i=0; i<sample_count; i++){
					sprintf(buf,"%c%c%c",core_data[i*3+3],core_data[i*3+4],core_data[i*3+5]);
					strcat(part_data,buf);
				}
				core_data[3] = '\0';
				sprintf((char *)res,"%s%x_%s_%s%s_ok",broadcast?"drv_rec_brc_":"drv_rec_ack_",m_rcv_packet[4],address_buf,core_data+1,part_data);
				free(part_data);
				part_data = NULL;
			}
			if(core_data[2] == SETUP_SELF){
				char setup_res = core_data[5];
				core_data[5] = '\0';
				sprintf((char *)res,"%s%x_%s_%s%c_ok",broadcast?"drv_rec_brc_":"drv_rec_ack_",m_rcv_packet[4],address_buf,core_data+1,setup_res+48);
			}
			if(core_data[2] == SETUP_PINS){
				core_data[9] = '\0';
				sprintf((char *)res,"%s%x_%s_%s_ok",broadcast?"drv_rec_brc_":"drv_rec_ack_",m_rcv_packet[4],address_buf,core_data+1);
			}
		}
	}
	
	printf("res: %s\n", res );	

	return res;
}

char *xbee_listen::sht_calc_temp(unsigned int value)
{
	char *res;
	res = (char *) malloc(6);
	double f_value = -45 +175*value/(pow(2,16)-1);

	sprintf(res,"%06.02f", f_value);
	return res;
}

char *xbee_listen::sht_calc_hum(unsigned int value)
{
	char *res;
	res = (char *) malloc(6);
	double f_value = 100*value/(pow(2,16)-1);
	sprintf(res,"%03.2f",f_value);
	return res;

}
unsigned char *xbee_listen::proccess_rx_packet(packet_data_t packet_data)
{
	unsigned char *res=NULL;
	char *val_buf, *address_buf;
	char buf[3];
	std::string mess_type;
	if((res =(unsigned char *)malloc(sizeof(unsigned char)*(2*PACKET_MAX))) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	memset(res,0x0,2*PACKET_MAX);
	if ((address_buf=(char *)malloc(sizeof(char)*40))== NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		free_buffers(res,NULL,NULL);
		return NULL;
	}
	address_buf[0]='\0';
	for(int i =0; i<10; i++){
		sprintf(buf,"%02x",packet_data.xbee_address[i]);
		strcat(address_buf,buf);
	}	

	if((val_buf=(char *)malloc(sizeof(char)*100)) == NULL){
		free_buffers(res,address_buf,NULL);
		return NULL;
	}
	val_buf[0]='\0';

	int values_shift=0, blind_value=0;
	for(int i=15;i<packet_data.frame_lenght+3; i++){
		//if(m_rcv_packet[i] != 0){
			bool convert_hex = false;
			if((i > 16 && val_buf[0] == 'A' && val_buf[1] == '1') || (i>17 &&  val_buf[0] == 'D' && val_buf[1] == 'S')){
				convert_hex = true;
				if(m_rcv_packet[i] == 0){
					sprintf(buf,"00");
				} else{
					sprintf(buf,"%02x",m_rcv_packet[i]);
				}
			}
			if(i > 19 && val_buf[0] == 'C' && val_buf[1] == 'A'){
				convert_hex = true;
				sprintf(buf,"%02x",m_rcv_packet[i]);
			}
			if(i > 18){
				if(val_buf[0] != 'A' && val_buf[0] != 'B' && val_buf[0] != 'C' && val_buf[0] != 'D' && val_buf[0] != 'R' && val_buf[0] != 'T'){
					int value_pos = 19 + values_shift*3;  
					if(i == value_pos && val_buf[1] == '1'){
						values_shift++;
						convert_hex = true;
						sprintf(buf,"%03d",m_rcv_packet[value_pos]);
					}
					
				}
			}
			if(i > 18 && val_buf[1] == '7'){
				convert_hex = true;
				sprintf(buf,"%x",m_rcv_packet[i]);
			}
			if(i > 17){
				if(val_buf[1] == '2'){
					if(i == 18){
						blind_value = m_rcv_packet[i] >> 8;
					}
					if(i == 19){
						blind_value += m_rcv_packet[i];
						convert_hex = true;
						sprintf(buf,"%d",blind_value);
					}
				}
			}
			if(!convert_hex){
				sprintf(buf,"%c",m_rcv_packet[i]);
			}
			if(i-15 < 97){
				strcat(val_buf,buf);
			} else {
				break;
			}
	//	} 
	}
	//printf("val buf: %s\n",val_buf);
	if(m_rcv_packet[14] == 0x01 || m_rcv_packet[14] == 0xc1 ){
		strcpy((char *)res,"drv_rec_act_");
	} else {
		strcpy((char *)res,"drv_rec_brc_");
	}
	int kerat;
	float live_time;
	if(val_buf[0] == 'A' || val_buf[0] == 'B' || val_buf[0] == 'C' || val_buf[0] == 'D' || val_buf[0] == 'R' || val_buf[0] == 'T' || val_buf[0] == 'X'|| val_buf[0] == 'E'|| val_buf[0] == 'H'){
		if( val_buf[0] == 'C' && val_buf[1] == 'O' && val_buf[4] == '0'){
			kerat = 0;
			kerat = m_rcv_packet[20] << 8;
			kerat += m_rcv_packet[21];
			sprintf((char *)res,"%s%x_%s_%s%d%04d_ok",(char *)res,m_rcv_packet[4],address_buf,"A1",0,kerat);  //// lebo A2 je D2 pin
		} else if( val_buf[0] == 'C' && val_buf[1] == 'O' && val_buf[4] == '1'){
			kerat = 0;
			kerat = m_rcv_packet[20] << 8;
			kerat += m_rcv_packet[21];
			sprintf((char*)res,"%s%x_%s_%s%d%04d_ok",(char *)res,m_rcv_packet[4],address_buf,"A2",0,kerat);
		} else if( val_buf[0] == 'C' && val_buf[1] == 'O' && val_buf[4] == '2'){
			kerat = 0;
			kerat = m_rcv_packet[20] << 8;
			kerat += m_rcv_packet[21];
			sprintf((char*)res,"%s%x_%s_%s%d%04d_ok",(char *)res,m_rcv_packet[4],address_buf,"A3",0,kerat);
		} else if(val_buf[0] == 'A' && val_buf[1] == '1') {
			int tmp_temp = (m_rcv_packet[25] << 8) + m_rcv_packet[26];
			char *text_tmp = display_temperature(tmp_temp,val_buf[2] == '1'?9:12);
			val_buf[0] = 'L';
			val_buf[18] = '\0';
			sprintf((char*)res,"%s%x_%s_%s%s_ok",(char *)res,m_rcv_packet[4],address_buf,val_buf,text_tmp);
			if(text_tmp != NULL){
				free(text_tmp);
				text_tmp = NULL;
			}
		} else if(val_buf[0] == 'X') {
			live_time = 0;
	//		printf("small: %d, medium: %d, big: %d, too_big: %d, live: %f\n",m_rcv_packet[19]==0?1:m_rcv_packet[19],m_rcv_packet[18]==0?1:m_rcv_packet[18], m_rcv_packet[17]==0?1:m_rcv_packet[17], m_rcv_packet[16]==0?1:m_rcv_packet[16]);
			if(m_rcv_packet[19] != 0){
				live_time+= m_rcv_packet[19]*0.00885;
			}
			if(m_rcv_packet[18] != 0){
				live_time+=m_rcv_packet[18]*0.00885*255;
			}
			if(m_rcv_packet[17] != 0){
				live_time+=m_rcv_packet[17]*0.00885*255*255;
			}
			if(m_rcv_packet[16] != 0){
				live_time+=m_rcv_packet[16]*0.00885*255*255*255;
			}
			sprintf((char *)res,"%s%x_%s_%s%d_ok",(char *)res,m_rcv_packet[4],address_buf,"X",(unsigned int)live_time);
		} else {
			sprintf((char*)res,"%s%x_%s_%s_ok",(char *)res,m_rcv_packet[4],address_buf,val_buf);
		}
	} else {
		if(m_rcv_packet[14] == 0x01 || m_rcv_packet[14] == 0xc1 ){
			strcpy((char *)res,"drv_rec_ack_");
		} else {
			strcpy((char *)res,"drv_rec_brc_");
		}
		sprintf((char*)res,"%s%x_%s_%s_ok",(char *)res,m_rcv_packet[4],address_buf,val_buf);
	}
	free_buffers(NULL,address_buf,val_buf);

	
	for(int i=0;i<packet_data.frame_lenght+4; i++){
	//	printf("%02x ", m_rcv_packet[i]);
	}
//	printf("\nres: %s\n",res);
//	printf("\n");


	return res;
}

unsigned char *xbee_listen::proccess_remote_at_res(packet_data_t packet_data)
{
	unsigned char *res=NULL;
	char *val_buf, *address_buf;
	char buf[3];
	if((res =(unsigned char *)malloc(sizeof(unsigned char)*(2*PACKET_MAX))) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	memset(res,0x0,2*PACKET_MAX);
	if ((address_buf=(char *)malloc(sizeof(char)*40))== NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		free_buffers(res,NULL,NULL);
		return NULL;
	}
	address_buf[0]='\0';

	for(int i =0; i<10; i++){
		sprintf(buf,"%02x",packet_data.xbee_address[i+1]);
		strcat(address_buf,buf);
	}

	if ( m_rcv_packet[17] == STATUS_OK){
		if ((m_rcv_packet[15] == 'D') && (m_rcv_packet[16] == 'B')){
		//	m_rcv_par_value=m_rcv_packet[18];
		}
		if ((m_rcv_packet[15] == 'K') && (m_rcv_packet[16] == 'Y')){
			m_aes_mess=1;
		}
		if ((m_rcv_packet[15] == 'E') && (m_rcv_packet[16] == 'E')){
			m_aes_mess=1;
		}
		if ((m_rcv_packet[15] == 'A') && (m_rcv_packet[16] == 'C')){
			m_aes_mess=1;
		}

		if(packet_data.frame_lenght > 0x0f){
			if((val_buf=(char *)malloc(sizeof(char)*100)) == NULL){
				free_buffers(res,address_buf,NULL);
				return NULL;
			}
			val_buf[0]='\0';	
			for(int i=18;i<packet_data.frame_lenght+3; i++){
				sprintf(buf,"%02x",m_rcv_packet[i]);
				strcat(val_buf,buf);
			}
			sprintf((char *)res,"drv_ack_%02x_%c%c_%s_%s_ok",m_rcv_packet[4],m_rcv_packet[15],m_rcv_packet[16],address_buf,val_buf);
			free_buffers(NULL,val_buf,address_buf);
		}else {
			sprintf((char *)res,"drv_ack_%02x_%c%c_%s_ok",m_rcv_packet[4],m_rcv_packet[15],m_rcv_packet[16],address_buf);
			free_buffers(NULL,NULL,address_buf);
		}
	}

	if ( m_rcv_packet[17] == STATUS_ERROR){
		sprintf((char *)res,"drv_ack_%02x_%c%c_%s_error",m_rcv_packet[4],m_rcv_packet[15],m_rcv_packet[16],address_buf);
		free_buffers(NULL,NULL,address_buf);
	} 
	if ( m_rcv_packet[17] == INVALID_COM){
		sprintf((char *)res,"drv_ack_%02x_%c%c_%s_Invalidcommand",m_rcv_packet[4],m_rcv_packet[15],m_rcv_packet[16],address_buf);
		free_buffers(NULL,NULL,address_buf);
	} 
	if ( m_rcv_packet[17] == INVALID_PAR){
		sprintf((char *)res,"drv_ack_%02x_%c%c_%s_Invalidparameter",m_rcv_packet[4],m_rcv_packet[15],m_rcv_packet[16],address_buf);
		free_buffers(NULL,NULL,address_buf);
	} 
	if ( m_rcv_packet[17] == NO_RESPONSE){
		if((m_rcv_packet[15]== 'K') && (m_rcv_packet[16] == 'Y')){
			m_aes_mess=1;
		}
		if((m_rcv_packet[15]== 'E') && (m_rcv_packet[16] == 'E')){
			m_aes_mess=1;
		}
		sprintf((char *)res,"drv_ack_%02x_%c%c_%s_Norespones",m_rcv_packet[4],m_rcv_packet[15],m_rcv_packet[16],address_buf);
		free_buffers(NULL,NULL,address_buf);
	}
	return res;
}

unsigned char *xbee_listen::proccess_at_com_res(packet_data_t packet_data)
{
	unsigned char *res=NULL;
	char *val_buf;
	char buf[3];
	if((res =(unsigned char *)malloc(sizeof(unsigned char)*(2*PACKET_MAX))) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	memset(res,0x0,2*PACKET_MAX);

	if(m_rcv_packet[7] == 0){
		if(packet_data.frame_lenght > 5){
			if((val_buf=(char *)malloc(sizeof(char)*100)) == NULL){
				m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
				return NULL;
			}		
			val_buf[0]='\0';	
			for(int i=8;i<packet_data.frame_lenght+3; i++){
				sprintf(buf,"%02x",m_rcv_packet[i]);
				strcat(val_buf,buf);
			}
			sprintf((char *)res,"drv_ack_%c%c_%s_ok",m_rcv_packet[5],m_rcv_packet[6],val_buf);
			free(val_buf);
			val_buf=NULL;
		} else {
			sprintf((char *)res,"drv_ack_%c%c_ok",m_rcv_packet[5],m_rcv_packet[6]);
		}

		if((m_rcv_packet[5]== 'N') && (m_rcv_packet[6] == 'D')){
			if (packet_data.frame_lenght > 5 ){
				m_frw=fopen("start_network","a");

				for(int i=8; i<packet_data.frame_lenght+3; i++){
					fprintf(m_frw,"%02x.",m_rcv_packet[i]);
				}
				fprintf(m_frw,"\n");
				fclose(m_frw);
			} 
		}
		if((m_rcv_packet[5]== 'K') && (m_rcv_packet[6] == 'Y')){
			m_aes_mess=1;
		}
		if((m_rcv_packet[5]== 'E') && (m_rcv_packet[6] == 'E')){
			m_aes_mess=1;
		}
		if((m_rcv_packet[5]== 'A') && (m_rcv_packet[6] == 'C')){
			m_aes_mess=1;
		}
	}
	if (m_rcv_packet[7] == STATUS_ERROR){
		sprintf((char*)res,"drv_ack_%x_%c%c_error",m_rcv_packet[4],m_rcv_packet[5],m_rcv_packet[6]);
	} 
	if (m_rcv_packet[7] == INVALID_COM){
		sprintf((char*)res,"drv_ack_%x_%c%c_Invalid command",m_rcv_packet[4],m_rcv_packet[5],m_rcv_packet[6]);
	} 
	if (m_rcv_packet[7] == INVALID_PAR){
		sprintf((char*)res,"drv_ack_%x_%c%c_Invalidparameter",m_rcv_packet[4],m_rcv_packet[5],m_rcv_packet[6]);
	}
	return res;
}


unsigned char *xbee_listen::proccess_modem_status(packet_data_t packet_data)
{
	unsigned char *res=NULL;
	if((res =(unsigned char *)malloc(sizeof(unsigned char)*(2*PACKET_MAX))) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	memset(res,0x0,2*PACKET_MAX);

	switch(m_rcv_packet[4]){
	 	case 0x00: 
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		case 0x01:
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		case 0x02:
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		case 0x03:
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		case 0x04:
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		case 0x05:
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		case 0x06:
			sprintf((char *)res,"drv_info_%d",m_rcv_packet[4]);
			break;
		default:
			break;
	}
	return res;
}
unsigned char *xbee_listen::proccess_tx_status(packet_data_t packet_data)
{
	unsigned char *res=NULL;
	if((res =(unsigned char *)malloc(sizeof(unsigned char)*(2*PACKET_MAX))) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	memset(res,0x0,2*PACKET_MAX);
	switch (m_rcv_packet[8]){
		case 0x00:
			sprintf((char *)res,"drv_txstat_%x_%d",m_rcv_packet[4],m_rcv_packet[8]);
			break;
		case 0x01:
			sprintf((char *)res,"drv_txstat_%x_%d",m_rcv_packet[4],m_rcv_packet[8]);
				break;
		case 0x15:
			sprintf((char *)res,"drv_txstat_%x_%d",m_rcv_packet[4],m_rcv_packet[8]);
			break;
		case 0x21:
			sprintf((char *)res,"drv_txstat_%x_%d",m_rcv_packet[4],m_rcv_packet[8]);
			break;
		case 0x25:
			sprintf((char *)res,"drv_txstat_%x_%d",m_rcv_packet[4],m_rcv_packet[8]);
			break;
		default:
			break;
	}
	return res;
}

char *xbee_listen::display_temperature(unsigned int temp2write, int resolution) 
{
	const unsigned short TEMP_RESOLUTION = resolution;
	char *text;
	text = (char *) malloc(sizeof(char)*10);
	if(text == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	strcpy(text,"000.00");

	const unsigned short RES_SHIFT = TEMP_RESOLUTION - 8;
	char temp_whole;
	unsigned int temp_fraction;
	
	if (temp2write & 0x8000) {
	       text[0] = '-';
		temp2write = ~temp2write + 1;
	}
	
	temp_whole = temp2write >> RES_SHIFT ;
	
	if(text[0] != '-'){
		if (temp_whole/100){
			text[0] = temp_whole/100  + 48;
		} else {
			text[0] = '0';
		}
	}
	
	text[1] = (temp_whole/10)%10 + 48;         
	text[2] =  temp_whole%10     + 48;          
	
	temp_fraction  = temp2write << (4-RES_SHIFT);
	temp_fraction &= 0x000F;
	temp_fraction *= 625;
	
	text[4] =  temp_fraction/1000    + 48;        
	text[5] = (temp_fraction/100)%10 + 48;       
	return text;
}
