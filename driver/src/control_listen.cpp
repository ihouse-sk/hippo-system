#include "../include/control_listen.h"

using namespace std;

control_listen::control_listen(api_uart *c_api_uart,std::string socket_name,log_class *c_log_class, comm_data *c_comm_data)
{
	m_comm_data = c_comm_data;
	m_log_class = c_log_class;
	m_api_uart = c_api_uart;
	m_socket_name = socket_name;
	m_aes_key = NULL;
}

void control_listen::free_buffers(unsigned char *buff1,char *buff2, char *buff3)
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

int control_listen::init()
{
	FILE *fr;
	int status;
	if(m_comm_data->get_source_install()){
		if((fr=fopen("parametre_mesh","r"))==NULL){
			//m_log_class->ih_log_write(LOG_ERROR,3,);
			m_log_class->ih_log_write(LOG_ERROR,3,"Nepodarilo sa otvorit subor %s,%s:%d\n","parametre_mesh",__FILE__,__LINE__);
			return -1;
		}
	} else {
		if((fr=fopen("/usr/share/driver/parametre_mesh","r"))==NULL){
			//m_log_class->ih_log_write(LOG_ERROR,3,);
			m_log_class->ih_log_write(LOG_ERROR,3,"Nepodarilo sa otvorit subor %s,%s:%d\n","parametre_mesh",__FILE__,__LINE__);
			return -1;
		}
	}
	
	while(1){
		ih_command_t tmp;	
		status = fscanf(fr,"%s %x %x %x\n",tmp.at_comm, &tmp.default_value,&tmp.max_value, &tmp.min_value);
		if(status != -1){
			m_valid_command.push_back(tmp);
		} else {
			break;
		}
	}
	if(fr != NULL){
		fclose(fr);
	}

	return 0;
}


void control_listen::operator()()
{
	unlink(m_socket_name.c_str());
	
	char *read_data;

	int server_sockfd, client_sockfd;
	int server_len;
	unsigned int  client_len;

	struct sockaddr_un server_unixaddress;
	struct sockaddr_un client_unixaddress;
	
	server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	server_unixaddress.sun_family = AF_UNIX;
	strcpy(server_unixaddress.sun_path,m_socket_name.c_str());
	server_len = sizeof(server_unixaddress);
	if(bind(server_sockfd, (struct sockaddr *)&server_unixaddress, server_len)<0){
		printf("error binding\n");
		m_comm_data->set_c_status(-1);
		m_comm_data->set_x_status(-1);
		m_log_class->ih_log_write(LOG_ERROR,4,"Error binding socket! ,%s:%d\n","parametre_mesh",__FILE__,__LINE__);
		return ;
	}

	listen(server_sockfd, 5);

	while (1){
		client_len = sizeof(client_unixaddress);
	        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_unixaddress, &client_len);

		if((read_data=(char *)malloc(sizeof(char)*100)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
        		close(client_sockfd);
			continue;
		}
		int tmp=read(client_sockfd, read_data, 100);
		if(tmp == -1){
			free(read_data);
        		close(client_sockfd);
			continue;
		}else {
			read_data[99]='\0';
		}
        	close(client_sockfd);

		if(!strncmp(read_data,"quit",4)){
			m_comm_data->set_x_status(-1);
			m_comm_data->set_c_status(-1);
			sleep(2);
			free(read_data);
			//////////// ukoncenie threadu
			break;
		} else if(!strncmp(read_data,"driver_reset",12)) {
			free(read_data);
			m_comm_data->set_reset_xbee();
		} else {
	//		printf("\nOd control layer: %s\n",read_data);
			resolve_data(read_data);
			free(read_data);
		}
	}
}

int control_listen::check_xbee_command(const char *par_value)
{
	int i,status=0;

	if(!strncmp("KY",(char *)this->m_at_command,2)){
		return 0;
	}
	for(i=0; i<(int)m_valid_command.size(); i++){
		if(!strncmp(m_at_command,m_valid_command[i].at_comm,2)){
			status=1;
			break;
		}
	}
	if(status == 0){
		char tmp_cmd[3];
		strncpy(tmp_cmd,m_at_command,2);
		tmp_cmd[2] = '\0';
		m_log_class->ih_log_write(LOG_ERROR,4,"Neplatny AT prikaz: %s, %s:%d\n",tmp_cmd,__FILE__,__LINE__);
		return -1;
	}
	if( m_valid_command[i].min_value == -1){
		m_present_parameter=READ_PAR;
	}else {	
		if(strcmp(par_value,"-1") == 0){
			m_present_parameter=READ_PAR;
		} else {
		//	m_present_parameter = atoi(par_value);

			if(m_present_parameter < m_valid_command[i].min_value || m_present_parameter > m_valid_command[i].max_value){
				char tmp_cmd[3];
				strncpy(tmp_cmd,m_at_command,2);
				tmp_cmd[2] = '\0';
				m_log_class->ih_log_write(LOG_ERROR,3,"Nepravna hodnota parametru pre prikaz %s, hodnota je: %x, rozsah je od %x do %x\n", tmp_cmd,m_present_parameter,m_valid_command[i].min_value,m_valid_command[i].max_value);
				return -1;
			}
		}
	}
	return 0;
}

int control_listen::create_packet(int packet_type, int frame_id)
{
	int i,checksum=0;
	int start_poradie;
	m_snd_data[PACKET_HEADER] = 0x7e;
	m_snd_data[PACKET_LEN_MSB] = 0x00;
	m_snd_data[PACKET_FRAMEID] = frame_id;
	m_snd_data[PACKET_IDENT] = packet_type;
//	std::cout << "Frame id: " << frame_id << "\n";

	if(packet_type == AT_COM){
		start_poradie = 5;
	}
	if(packet_type == REMOTE_AT_COM){
		start_poradie = 16;
		m_snd_data[PACKET_OPTIONAL_BIT]=m_remote_setting;
	}
	if(packet_type == TX_REQ){
		m_snd_data[15] = 0x00;
		m_snd_data[16] = 0x01;
		// c1B11
		//podla noveho pride:  c1B1009 
		if(m_uc_data[1] == '1'){
			if((m_uc_data.length()-2)%5 != 0){
				m_log_class->ih_log_write(LOG_ERROR,3,"Nespravna dlzka spravy pre uc, %s:%d\n",__FILE__,__LINE__);
				return -1;
			}
			int sample_number = (m_uc_data.length()-2)/5;
			m_snd_data[17] = m_uc_data[0];
			m_snd_data[18] = m_uc_data[1];
			for(int i = 0; i < sample_number; i++){
				m_snd_data[19+i*3] = m_uc_data[2+i*5];
				m_snd_data[19+i*3+1] = m_uc_data[2+i*5+1];
				int value;
				sscanf(m_uc_data.substr(2+i*5+2,3).c_str(),"%d",&value);
				m_snd_data[19+i*3+2] = (unsigned char)value;
			}
			m_snd_data[PACKET_LEN_LSB]=m_uc_data.length()+14-sample_number*2;
		} else if (m_uc_data[1] == '2'){
			m_snd_data[17] = m_uc_data[0];
			m_snd_data[18] = m_uc_data[1];
			m_snd_data[19] = m_uc_data[2];
			int mess_value =0;
			sscanf(m_uc_data.substr(3).c_str(), "%d", &mess_value);
			m_snd_data[20] = (mess_value >> 8) & 0xff;
			m_snd_data[21] = mess_value & 0xff;
			m_snd_data[PACKET_LEN_LSB]=m_uc_data.length()+14;
		} else {
			for ( i=0; i<(int)m_uc_data.length(); i++){
				m_snd_data[17+i] = m_uc_data[i];
			}
			m_snd_data[PACKET_LEN_LSB]=m_uc_data.length()+14;
		}
	}
	if(packet_type == AT_COM || packet_type == REMOTE_AT_COM){
		if(m_present_parameter == READ_PAR){
			m_snd_data[PACKET_LEN_LSB] = start_poradie-1;
			m_snd_data[start_poradie]=m_at_command[0];
			m_snd_data[start_poradie+1]=m_at_command[1];
		} else {
			if(!strncmp("KY",(char *)m_at_command,2)){
			//	std::cout << " Key: " << m_aes_key<< std::endl;
				m_snd_data[start_poradie]=m_at_command[0];
				m_snd_data[start_poradie+1]=m_at_command[1];
				for(int i=0; i<(int)strlen((char*)m_aes_key); i++){
					m_snd_data[start_poradie+2+i] = m_aes_key[i];
				}
				m_snd_data[PACKET_LEN_LSB]=start_poradie + 15;
				/////////AES
				/*m_snd_data[5]=m_at_command[0];
				m_snd_data[6]=m_at_command[1];
				for(i=0; i<strlen((char *)m_aes_key_ptr); i++){
					m_snd_data[7+i]=m_aes_key_ptr[i];
				}
				m_snd_data[PACKET_LEN_LSB]=0x04+16;*/
			} else {
				m_snd_data[start_poradie]=m_at_command[0];
				m_snd_data[start_poradie+1]=m_at_command[1];
				if (m_present_parameter< 0 ){
					m_log_class->ih_log_write(LOG_ERROR,3,"Nespravne nastavena hodnota snd_at_command, %s:%d\n",__FILE__,__LINE__);
					return -1;
				}
				if ((m_present_parameter >= 0) && (m_present_parameter <= 0xff)) {
					m_snd_data[start_poradie+2] = m_present_parameter;
					m_snd_data[PACKET_LEN_LSB]=start_poradie;
				}
				if ((m_present_parameter > 0xff) && (m_present_parameter <= 0xffff)) {
					m_snd_data[start_poradie+2] = (m_present_parameter >> 8);
					m_snd_data[start_poradie+3] = m_present_parameter & 0xff;
					m_snd_data[PACKET_LEN_LSB]=start_poradie+1;
				}
				if ((m_present_parameter > 0xffff) && (m_present_parameter <= 0xffffff)) {
					m_snd_data[start_poradie+2] = m_present_parameter >> 16;
					m_snd_data[start_poradie+3] = m_present_parameter >> 8;
					m_snd_data[start_poradie+4] = m_present_parameter & 0xff;
					m_snd_data[PACKET_LEN_LSB]=start_poradie+2;
				}
				if ((unsigned int)(m_present_parameter > 0xffffff) && ((unsigned int)m_present_parameter <= 0xffffffff)) {
					m_snd_data[start_poradie+2] = m_present_parameter >> 24;
					m_snd_data[start_poradie+3] = m_present_parameter >> 16;
					m_snd_data[start_poradie+4] = m_present_parameter >> 8;
					m_snd_data[start_poradie+5] = m_present_parameter & 0xff;
					m_snd_data[PACKET_LEN_LSB]=start_poradie+3;
				}
				if ((unsigned int)m_present_parameter > 0xffffffff){
					m_log_class->ih_log_write(LOG_ERROR,3,"Snd_at_value out of range, %s:%d\n",__FILE__,__LINE__);
					return -1;
				}
			}
		}
	}

	for(i=0; i<m_snd_data[PACKET_LEN_LSB]; i++){
		checksum += m_snd_data[i+3];
	}
	m_snd_data[m_snd_data[PACKET_LEN_LSB]+3]=0xff-(checksum &0xff);
	return m_snd_data[PACKET_LEN_LSB]+4;
}

int control_listen::send_packet(std::vector<std::string> str_data, int mess_type)
{
	int data_lenght;
	char *buffer,*buf;//,*ky_buf;
	if((m_snd_data = (unsigned char*) malloc (sizeof(unsigned char)*PACKET_MAX)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	if((buffer=(char *)malloc(sizeof(char)*200)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		free_buffers(m_snd_data,NULL,NULL);
		return -1;
	}
		
	if((buf=(char *)malloc(sizeof(char)*4)) == NULL ){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		free_buffers(m_snd_data,buffer,NULL);
		return -1;
	}
	buffer[0]='\0';
	
	if(mess_type == AT_COM ){
		strncpy(m_at_command,str_data[0].c_str(),2);
		for(int i=0; i< 2; i++){
			m_at_command[i] = toupper(m_at_command[i]);
		}
		sscanf(str_data[1].c_str(),"%x",&m_present_parameter);
	
		if(!strncmp("KY",m_at_command,2)){
			if((m_aes_key = (unsigned char*)malloc(17)) == NULL){
				m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
				free_buffers(m_snd_data,buffer,buf);
				return -1;
			}
			strncpy((char *)m_aes_key,str_data[1].c_str(),16);
			m_aes_key[16] = '\0';
			//std::cout << "Strdata[1] : " << m_aes_key <<std::endl;
			m_present_parameter = 1;
			////////// AES
		} 
		if(check_xbee_command(str_data[1].c_str()) == -1){
			free_buffers(m_snd_data,buffer,buf);
			if(m_aes_key != NULL){
				free(m_aes_key);
				m_aes_key = NULL;
			}
			return -1;
		}
	}
	if(mess_type == REMOTE_AT_COM){
		strncpy(m_at_command,str_data[2].c_str(),2);
		for(int i=0; i< 2; i++){
			m_at_command[i] = toupper(m_at_command[i]);
		}
		sscanf(str_data[3].c_str(),"%x",&m_present_parameter);
	
		if(!strncmp("KY",m_at_command,2)){
			if((m_aes_key = (unsigned char*)malloc(17)) == NULL){
				m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
				free_buffers(m_snd_data,buffer,buf);
				return -1;
			}
			strncpy((char *)m_aes_key,str_data[3].c_str(),16);
			m_aes_key[16] = '\0';
			m_present_parameter = 1;
				////////// AES
		} 
		if(check_xbee_command(str_data[3].c_str()) == -1){
			free_buffers(m_snd_data,buffer,buf);
			if(m_aes_key != NULL){
				free(m_aes_key);
				m_aes_key = NULL;
			}
			return -1;
		}
		if(check_xbee_name(atoi(str_data[0].c_str())) == -1){
			free_buffers(m_snd_data,buffer,buf);
			m_log_class->ih_log_write(LOG_ERROR,4,"Remote xbee neexistuje:%d, %s:%d\n",atoi(str_data[0].c_str()),__FILE__,__LINE__);
			if(m_aes_key != NULL){
				free(m_aes_key);
				m_aes_key = NULL;
			}
			return -1;
		}
	}
	if(mess_type == TX_REQ){
		m_uc_data = str_data[2];
		if(check_xbee_name(atoi(str_data[0].c_str())) == -1){
			free_buffers(m_snd_data,buffer,buf);
			m_log_class->ih_log_write(LOG_ERROR,4,"Remote xbee neexistuje:%d, %s:%d\n",atoi(str_data[0].c_str()),__FILE__,__LINE__);
			return -1;
		}
	}
	//std::cout << "konvertujem: " << str_data[1].c_str() <<" a konvertovane: " << strtol(str_data[1].c_str(), NULL, 16)<<"\n";
	//if((data_lenght = create_packet(mess_type,strtol(str_data[1].c_sr(), NULL, 16))) == -1){
	if((data_lenght = create_packet(mess_type,atoi(str_data[1].c_str()))) == -1){
		free_buffers(m_snd_data,buffer,buf);
		if(m_aes_key != NULL){
			free(m_aes_key);
			m_aes_key = NULL;
		}
		return -1;
	}
	for (int i=0; i<data_lenght; i++){
		sprintf(buf,"%02x ",m_snd_data[i]);
		strcat(buffer,buf);
	}

	m_log_class->ih_log_write(LOG_COMM,1,"Send data: %s\n",buffer);
	m_comm_data->set_send_time(time(NULL));
	if(m_comm_data->get_recieve_time()+58 < time(NULL)){
		m_comm_data->set_recieve_time(time(NULL));
	}
	int result = m_api_uart->xb_write_data(m_snd_data, data_lenght);
	if(result != data_lenght){
		m_log_class->ih_log_write(LOG_ERROR,3,"Failed to write data, %s:%d\n",__FILE__,__LINE__);
		free_buffers(m_snd_data,buffer,buf);
		if(m_aes_key != NULL){
			free(m_aes_key);
			m_aes_key = NULL;
		}
		return -1;
	} else {
		free_buffers(m_snd_data,buffer,buf);
		if(m_aes_key != NULL){
			free(m_aes_key);
			m_aes_key = NULL;
		}
		return result;
	}

}

int control_listen::check_xbee_name(int xbee_name)
{
	for(int i=0; i<m_comm_data->get_xbee_size(); i++){
		if(m_comm_data->get_xbee_ident(i) == xbee_name){
			sscanf(m_comm_data->get_xbee_mac(i).c_str(),"%x %x %x %x %x %x %x %x %x %x",
					(unsigned int*)&m_snd_data[5], (unsigned int*)&m_snd_data[6],(unsigned int*)&m_snd_data[7],(unsigned int*)&m_snd_data[8],
					(unsigned int*)&m_snd_data[9], (unsigned int*)&m_snd_data[10],(unsigned int*)&m_snd_data[11],(unsigned int*)&m_snd_data[12],
					(unsigned int*)&m_snd_data[13],(unsigned int*)&m_snd_data[14]);
			return 0;
		}
	}
	m_log_class->ih_log_write(LOG_ERROR,4,"Unknown xbee name: %d, %s:%d\n",xbee_name,__FILE__,__LINE__);
	return -1;
}

int control_listen::resolve_data(char *read_data)
{
	char **buf;
	int i;
	//int frame_id;

	if((buf=(char **)malloc(sizeof(char *)*MESS_SIZE))==NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
		
	for(i=0; i<MESS_SIZE; i++){
		if((buf[i]=(char *)malloc(sizeof(char)*BUF_LEN)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	char *pch;
	i=0;
	read_data[99]='\0';
	
	pch = strtok (read_data,"_");

	while( pch != NULL){
		strcpy(buf[i],pch);
		i++;
		pch=strtok(NULL,"_");
		if (i > MESS_SIZE-1){
			break;
		}
	}
	if(i >=2){
		std::vector<std::string> str_data;
		int mess_type;
		str_data.push_back(buf[0]);
		str_data.push_back(buf[1]);
			//remote mesage 3
			//remote at 4
		if(i == 2){
			mess_type = AT_COM;
			/// send at command
		}
		if(i == 3){
			mess_type = TX_REQ;
			str_data.push_back(buf[2]);
			//send remote message
		}
		if(i == 4){
			mess_type = REMOTE_AT_COM;
			m_remote_setting = 0x02;
			str_data.push_back(buf[2]);
			str_data.push_back(buf[3]);
			//send remote at
		}
		if( i == 5){
			mess_type = REMOTE_AT_COM;
			m_remote_setting = 0x00;
			str_data.push_back(buf[2]);
			str_data.push_back(buf[3]);
			str_data.push_back(buf[4]);
		}
	//	std::copy(str_data.begin(), str_data.end(), std::ostream_iterator<std::string>(std::cout));
//		std::copy(str_data.begin(), str_data.end(),std::ostream_iterator<std::string>(std::cout, "\n"));
//		std::cout << std::endl;
		send_packet(str_data,mess_type);
	} else {
		m_log_class->ih_log_write(LOG_ERROR,3,"Priliz kratka (%d) sprava od control layer,%s:%d\n",i,__FILE__,__LINE__);
	}

	for(i=0; i<MESS_SIZE; i++){
		free(buf[i]);
	}
	
	free(buf);
	
	return 0;
}
