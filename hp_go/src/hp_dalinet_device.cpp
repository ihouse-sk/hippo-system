#include "../include/hp_dalinet_device.h"

dalinet_device::dalinet_device(string ip, int port, dali_data_t *comm_data, string name)
{
	my_comm_data = comm_data;
	my_dali_ip = ip;
	my_dali_port = port;
	my_name = name;
	my_conn_status = CONN_INACTIVE;
}

void dalinet_device::operator()()
{
	int res,data_len;
	unsigned int  m_len ;
	time_t check_time = time(NULL);
	char *read_data=NULL;
	my_socket = socket(AF_INET,SOCK_STREAM, 0);
	if (my_socket < 0) {
		std::cerr << "cannot create socket"<<std::endl; 
		return ;
	}
	m_address.sin_family = AF_INET;
	m_address.sin_addr.s_addr = inet_addr(my_dali_ip.c_str());
	m_address.sin_port = htons(my_dali_port);
	m_len = sizeof(m_address);
	
	res = connect(my_socket, (struct sockaddr *)&m_address, m_len);
	if (res == -1) {
		std::cout<<"error connect to af_inet socket m_address = "<<my_dali_ip<<" m_port = "<<my_dali_port<<std::endl;	
		return ;
	} else {
		cout << "Vytvoril som a otvoril spojenie na dalinet device, ip: " << my_dali_ip << " port: " << my_dali_port << endl;
		my_conn_status = CONN_ACTIVE;
		while (1) {
			data_len = wait4data();
			if(data_len > 10){
				read_data = (char *) malloc (data_len +1);
				int read_count = read(my_socket,read_data, data_len);
				if(read_count > 0){
					read_data[data_len] = '\0';
					//cout << "Nacitane data: " << read_data << endl;
					string response = this->process_mess(read_data);
					if(response != ""){
						my_comm_data->mtx.lock();
						my_comm_data->dali_responses.push_back(response);
						my_comm_data->mtx.unlock();
					}
					check_time = time(NULL);
				}

				if(read_data != NULL){
					free(read_data);
					read_data = NULL;
				}
			} else {
				usleep(10000);
			}
			if(check_time+5 < time(NULL)){
				char buffer[1];
				buffer[0] ='z';
				if(write(my_socket,&buffer, 1) != 1){
					cout << "Error sending data... " << endl;
					my_conn_status = CONN_INACTIVE;
				}
				check_time = time(NULL);
			}
			if(this->my_comm_data->dali_commands.size() > 0){
				my_comm_data->mtx.lock();
				char *buffer = NULL;
	//			cout << endl << "Message: " <<my_comm_data->dali_commands[0]  << ", commands size: " << my_comm_data->dali_commands.size()<< endl;
				buffer = this->create_send_data(my_comm_data->dali_commands[0]);
				if(buffer != NULL){
					control_data_t tmp_data;
					tmp_data.buffer = buffer;
					tmp_data.send_time = time(NULL);
					tmp_data.resend_counter = 3;
					tmp_data.interval = 2;
					tmp_data.mess = my_comm_data->dali_commands[0];
					if(write(my_socket, buffer, strlen(buffer)) != (unsigned int)strlen(buffer)){
						my_conn_status = CONN_INACTIVE;
						tmp_data.send_time += 5;
					}
					my_control_data.push_back(tmp_data);
				} 
				my_comm_data->dali_commands.erase(my_comm_data->dali_commands.begin());
				my_comm_data->mtx.unlock();
			}
			if(my_conn_status == CONN_INACTIVE){
				reset_dali_connection("Neaktivne spojenie");
			}
			if(my_conn_status == CONN_ACTIVE){
				for(unsigned int i=0; i<my_control_data.size(); i++){
					if(my_control_data[i].send_time < time(NULL)){
						reset_dali_connection("Neprisla odpoved do stanovenho casu");
						if(--my_control_data[i].resend_counter == 0){
							my_comm_data->dali_responses.push_back("noResponse_"+my_control_data[i].mess);
							my_control_data.erase(my_control_data.begin()+i);
							i--;
							continue;
						}
						if(write(my_socket, my_control_data[i].buffer, strlen(my_control_data[i].buffer)) != (unsigned int)strlen(my_control_data[i].buffer)){
							my_conn_status = CONN_INACTIVE;
							my_control_data[i].send_time += 5;
						} else {
							my_control_data[i].send_time = time(NULL);
						}

					}

				}
			}
			if(my_comm_data->finish){
				break;
			}
		}
	}
}
int dalinet_device::reset_dali_connection(string reason)
{
	int res;
	unsigned int  m_len ;
	m_len = sizeof(m_address);
	cout << "Reset dali connection: "<<reason  << endl;
	shutdown(my_socket,2);
	close(my_socket);
	usleep(100000);
	my_socket = socket(AF_INET,SOCK_STREAM, 0);
	res = connect(my_socket, (struct sockaddr *)&m_address, m_len);
	if(res == -1){
		my_comm_data->dali_responses.push_back("error_conn inactive");
		sleep(1);
	} else {
		my_conn_status = CONN_ACTIVE;
	}

	return res;
}

string dalinet_device::process_mess(char *data)
{
	string res;
	string find_str;
	res = "";
	unsigned char res_char[BUF_MAX];
	int num, res_char_counter = 0;
	unsigned int i;
//	cout << "Nacitane data len: " << strlen(data) << "\t";
//	for(i=0; i<strlen(data); i++){
//		printf("%02x ",data[i]);
		//01 30 34 31 30 30 33 30 30 45 38 17
		//01 30 34 31 30 38 34 44 43 38 42 17
		//01 30 34 31 30 38 35 30 30 36 36 17
//	}
	//cout << endl;
	for(i=1; i<strlen(data)-1; i+=2){
		char tmp[3];
		tmp[0] = data[i];
		tmp[1] = data[i+1];
		tmp[2] = '\0';
		sscanf(tmp,"%02x",&num);
		res_char[res_char_counter++] = (char )num & 0xff;
	}
	res_char[res_char_counter] = '\0';
//	for(i=0; i<(unsigned int)res_char_counter; i++){
//		printf("%02x ", 0xff & res_char[i]);
//	}
//	cout << endl;
	if(data[2] == 0x34){
		//01 30 34 31 30 30 32 39 36 35 33 17
		int number = 1;

		if(res_char[2]%2 == 0){
			number = 0;
		}
		if(res_char[2] < 0x80){
			res = "A";
			res += ((res_char[2]-number)/2)+48;
			res += "_";
		} else if(res_char[2] == 0xff){
			res = "BC_";
		} else {
			res = "G";
			res += ((res_char[2]-0x80-number)/2)+48;
			res += "_";
		}
		if(number == 1){
			res += translate_resp_value(res_char[3]);
		} else{
			res += "acr_"+patch::to_string((int)res_char[3]);
		}
		find_str = res;
	//	cout << endl << "res: " << res << endl;;
	}
	if(data[2] == 0x33){
		//raw data 01 30 33 31 30 30 33 41 30 30 38 30 30 34 31 17 
		//res char      03   10    03    a0    08     00    41
		if(res_char[2] < 0x80){
			res = "A";
			res += ((res_char[2]-1)/2)+48;
			res += "_";
			res += translate_resp_value(res_char[3])+"_";
			res += patch::to_string((int)res_char[5]);
			find_str = res.substr(0,res.find_last_of("_"));
		}
	}
	if(res[res.length()-1] == '_'){
		cout << endl << "Chyba pri preklade spravy: " << res << endl;
		return "";
	}
	for(i=0; i<my_control_data.size(); i++){
	//	cout << "Find str: " << find_str << " vs " << my_control_data[i].mess << endl;
		if(my_control_data[i].mess.find(find_str) != std::string::npos){
	//		cout << "Mazem v dalinet zariadeni: " << my_control_data[i].mess << endl;
			free(my_control_data[i].buffer);
			my_control_data.erase(my_control_data.begin()+i);
			break;
		}
	}

	return res;
}

string dalinet_device::translate_resp_value(unsigned char resp_value)
{
	string res = "";
	switch (resp_value){
		case DALI_LIGHT_OFF_VALUE: {
						   res = "off";
						   break;
					   }
		case DALI_LIGHT_MAX_VALUE: {
						   res = "on";
						   break;
					   }
		case DALI_LIGHT_MIN_VALUE: {
						   res = "min";
						   break;
					   }
		case DALI_LIGHT_DOWN: {
						   res = "down";
						   break;
					   }
		case DALI_LIGHT_UP: {
						   res = "up";
						   break;
					   }
		case DALI_LIGHT_GET_VALUE: {
						   res = "A0";
						  // res = patch::to_string(resp_value);
						   break;
					   }
		default: {
				 if(resp_value >= DALI_SCENE_START && resp_value <= DALI_SCENE_END){
				 }
				 break;
			 }
	}
	return res;
}

char *dalinet_device::create_send_data(string command)
{
	int res_counter=0;
	int checksum = 0;
	int command_value = 0;
	char *res = (char *)malloc(BUF_MAX);
	if(res == NULL){
		return res;
	}
	//cout << "create data: " << command  << endl;

	res[res_counter++] = 0x01;

	vector<string> parsed_command;

	while(command.find("_") != std::string::npos){
		parsed_command.push_back(command.substr(0,command.find("_")));
		command= command.substr(command.find("_")+1);
	}
	parsed_command.push_back(command);

	for(int i=0; i<(int)parsed_command.size(); i++){
		//cout << parsed_command[i] << endl;
	}
	if(parsed_command.size() == 2 || parsed_command.size() == 3){
		checksum += DALI_CMD;
		res[res_counter++] = get_char(DALI_CMD,0);
		res[res_counter++] = get_char(DALI_CMD,1);

		res[res_counter++] = get_char(0x00,0);
		res[res_counter++] = get_char(0x00,1);

		checksum += 0x10;
		res[res_counter++] = get_char(0x10,0);
		res[res_counter++] = get_char(0x10,1);
		
		int number = 1;
		if(parsed_command.size() == 3){
			number = 0;
		}

		if(parsed_command[0][0] == 'A'){
			checksum += ((parsed_command[0][1]-48)*2+number);
			res[res_counter++] = get_char((parsed_command[0][1]-48)*2+number,0);
			res[res_counter++] = get_char((parsed_command[0][1]-48)*2+number,1);
		} else if(parsed_command[0][0] == 'G'){
			checksum += ((parsed_command[0][1]-48)*2+0x80 + number) ;
			res[res_counter++] = get_char((parsed_command[0][1]-48)*2+0x80+number,0);
			res[res_counter++] = get_char((parsed_command[0][1]-48)*2+0x80+number,1);
		}

		if(number == 1){
			command_value = translate_command(parsed_command[1]);	
		} else {
			command_value = atoi(parsed_command[2].c_str());	
		}

		checksum += command_value;
		res[res_counter++] = get_char(command_value,0);
		res[res_counter++] = get_char(command_value,1);

		checksum = checksum % 0x100;
		checksum = (0xff - checksum);
//		printf("checksum: %02x\n",checksum);
		res[res_counter++] = get_char(checksum,0);
		res[res_counter++] = get_char(checksum,1);
		res[res_counter++] = 0x17;
	}
	/*
	cout << "send packet: ";
	for(int i=1; i<res_counter-1; i++){
		printf("%02x ",res[i]);
	}
	cout << endl;
	*/
	res[res_counter] = '\0';

	return res;
}

char dalinet_device::translate_command(string cmd)
{
	char res = 0;

	if(cmd == "off"){
		res = DALI_LIGHT_OFF_VALUE; /// vypni 
	} else if (cmd == "on"){
		return DALI_LIGHT_MAX_VALUE; // zapni na maximum
	} else if(cmd == "A0"){
		return DALI_LIGHT_GET_VALUE; // vyziadaj si aktualny hodnotu
	} else if(cmd == "min"){
		return DALI_LIGHT_MIN_VALUE;  // zapni na minimum
	} else if(cmd == "ballast"){
		return DALI_LIGHT_GET_BALLAST;  // ziskaj ballast
	} else if(cmd == "getMax"){
		return 0xA1; // ziskaj max hodnotu
	} else if(cmd == "getMin"){
		return 0xA2; // ziskaj min hodnotu
	} else if(cmd == "getFalilureLevel"){
		return 0xA4;
	} else if(cmd == "getStatus"){
		return 0x90;
	} else if(cmd.find("addGroup") != std::string::npos){
		return 0x60 + atoi(cmd.substr(cmd.length()-2,1).c_str());
	} else if(cmd.find("removeGroup") != std::string::npos){
		return 0x70 + atoi(cmd.substr(cmd.length()-2,1).c_str());
	} else if(cmd == "getGroup1"){
		return 0xC1;
	} else if(cmd == "getGroup2"){
		return 0xC2;
	}

	return res;
}

char dalinet_device::get_char(char source, int pos)
{
	char res[3];
	if(pos == 0){
		sprintf(res,"%x",source & 0xf0);
		//cout << "res: " << res << endl;
		return toupper(res[0]);
	} 
	if(pos == 1){
		sprintf(res,"%x",source & 0x0f);
	//	cout << "res: " << res << endl;
		return toupper(res[0]);
	}
	return 0;
}


int dalinet_device::xb_ioctl_data()
{
	int bytes=0;
	ioctl(my_socket,FIONREAD,&bytes);
	return bytes;
}

int dalinet_device::wait4data(int data_size)
{
	if(data_size > 0){
		//std::cout << "Data size: " << data_size<< std::endl;
	}
	int bytes=0;
	for(int i=0; i< TIMEOUT; i++){
		bytes = xb_ioctl_data();
		if(bytes > data_size){
			return bytes;
		}
		usleep(SLEEP_TIME_THREAD);
	}
	return bytes;
}
