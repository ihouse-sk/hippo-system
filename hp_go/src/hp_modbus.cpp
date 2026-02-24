#include "../include/hp_modbus.h"


hp_modbus::hp_modbus(hp_mdb_data_t *rx,hp_mdb_data_t *tx)
{
	my_tx_data = tx;
	my_rx_data = rx;
}

void hp_modbus::operator()()
{
	while(1){
		if(my_tx_data->my_data.size() > 0){
			string res = send_mess(my_tx_data->my_data[0]);
			my_tx_data->mtx.lock();
			my_tx_data->my_data.erase(my_tx_data->my_data.begin());
			my_tx_data->mtx.unlock();
		}
		usleep(50000);
	}
}

string hp_modbus::send_mess(hp_modbus_data data)
{
	int res,data_len;
	string response = "NOTOK";
	unsigned int  m_len ;
	unsigned char *read_data=NULL;
	unsigned int wait_counter = 0;
	my_socket = socket(AF_INET,SOCK_STREAM, 0);
	if (my_socket < 0) {
		std::cerr << "cannot create socket"<<std::endl; 
		return response;
		//return "Canoot create socket: " + my_moxa_ip + " port: " + patch::to_string(my_moxa_port);
	}
	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = inet_addr(data.get_master_ip().c_str());
	my_address.sin_port = htons(data.get_master_port());
	m_len = sizeof(my_address);
	
	res = connect(my_socket, (struct sockaddr *)&my_address, m_len);
	if (res == -1) {
		std::cout<<"error connect to af_inet socket my_address = "<<data.get_master_ip().c_str()<<" m_port = "<<data.get_master_port()<<std::endl;	
		return response;
		//return "error connect to af_inet socket my_address = "+my_moxa_ip+" m_port = "+patch::to_string(my_moxa_port);
	} else {
		char *buffer = NULL;
		buffer = data.create_send_data(2,&data_len);
		if(buffer != NULL){
			if(write(my_socket, buffer, data_len) != data_len){
				cout <<"Chyba posielania dat... " << endl;
				return response;
			} 
			free(buffer);
			buffer = NULL;
		}
		while(1){
			data_len = wait4data(4);
			if(data_len > 5){
				read_data = (unsigned char *) malloc (data_len);
				int read_count = read(my_socket,read_data, data_len);
				if(read_count == data_len){
					/*
					cout << "[TCP]>RX: ";
					for(int i=0; i<read_count; i++){
						printf("%02x ",read_data[i]);
					}
					cout << endl;
					*/
					//00 02 00 00 00 15 01 03 0c 00 00 00 c9 00 00 00 00 00 14 00 c8
					//response = create_response(read_data, value, read_count);
					my_rx_data->my_data.push_back(hp_modbus_data(data.get_master_ip(), data.get_master_port(), data.get_start_address(), data.get_funct_code(),data.get_coils_num(),1,read_data,read_data[5]));
					break;
				}
				/*
				if(read_5yydata != NULL){
					free(read_data);
					read_data = NULL;
					break;
				}*/
			} else {
				usleep(1000);
				if(wait_counter++ > 5){
					cout <<"Neodpovedalo zariadenie... " << endl;
					break;
				}
			}
		}
		cout <<endl;
		shutdown(my_socket,2);
		close(my_socket);
	}
	return response;
}
int hp_modbus::ioctl_data()
{
	int bytes=0;
	ioctl(my_socket,FIONREAD,&bytes);
	return bytes;
}


int hp_modbus::wait4data(int data_size)
{
	if(data_size > 0){
		//std::cout << "Data size: " << data_size<< std::endl;
	}
	int bytes=0;
	for(int i=0; i< TIMEOUT; i++){
		bytes = ioctl_data();
		if(bytes > data_size){
			return bytes;
		}
		usleep(SLEEP_TIME_THREAD);
	}
	return bytes;
}



