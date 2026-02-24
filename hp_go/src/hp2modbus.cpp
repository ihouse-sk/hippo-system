#include "../include/hp2modbus.h"


hp2modbus::hp2modbus(hp2mdb_data_t *rx,hp2mdb_data_t *tx, XMLNode node)
{
	my_tx_data = tx;
	my_rx_data = rx;
	for(int i=0; i<node.nChildNode("master"); i++){
		my_devices.push_back(hp2modbus_devices(node.getChildNode(i)));
	}
	for(int i=0; i<node.getChildNode("slaves").nChildNode("slave"); i++){
		my_slaves.push_back(std::make_shared<hp2modbus_slave>(node.getChildNode("slaves").getChildNode("slave",i)));
	}
	my_modbus_log = false;
}

void hp2modbus::operator()()
{
	while(1){
		while(my_tx_data->my_data.size() > 0){
			for(auto i: my_slaves){
				if(i->get_id() == my_tx_data->my_data[0].get_desc()){
					send_mess(my_tx_data->my_data[0], i);
					break;
				}
			}
			my_tx_data->mtx.lock();
			my_tx_data->my_data.erase(my_tx_data->my_data.begin());
			my_tx_data->mtx.unlock();
		}
		for(auto i: my_slaves){
			if(i->run_autoread()){
				string tmp = send_mess(hp2modbus_data(),i);
			}
			//break;
		}
		/*
		std::for_each(my_slaves.begin(), my_slaves.end(), [](hp2modbus_slave &slave) {
				slave.run_autoread();
				});
		for(int i=0; i<my_slaves.size(); i++){
			my_slaves[i].run_autoread();
		}
		*/
		std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME_THREAD));
	}
}

string hp2modbus::send_mess(hp2modbus_data data, shared_ptr<hp2modbus_slave> slave)
{
	string resp = "";
	unsigned int  len;
	unsigned int wait_counter = 0;
	my_socket = socket(AF_INET,SOCK_STREAM, 0);
	if (my_socket < 0) {
		std::cerr << "cannot create socket"<<std::endl;
		return resp;
	}

	for(auto &i: my_devices){
		if(i.get_id() == slave->get_master_id()){
			my_address.sin_family = AF_INET;
			my_address.sin_addr.s_addr = inet_addr(i.get_ip().c_str());
			my_address.sin_port = htons(i.get_port());
			len = sizeof(my_address);
			int res = connect(my_socket, (struct sockaddr *)&my_address, len);
			if (res == -1) {
				string tmp = "cannot connect to af_inet socket, ip: " + i.get_ip() + " port: " + patch::to_string(i.get_port());
				push_resp_data(hp2modbus_data(slave->get_id(), data.get_value(),0, ("Error - "+tmp+": "+ patch::to_string(__LINE__))));
				return resp;
			} else {
				char *buffer = NULL;
				int buff_len = 0;
				buffer = this->create_send_data(slave,data.get_value(),&buff_len, i.get_type());
				if(buffer != NULL){
					int send_count = write(my_socket, buffer, buff_len);
					//cout <<"Send size: " << buff_len << ", poslal som: " << send_count << endl;
					if(send_count != buff_len){
						push_resp_data(hp2modbus_data(slave->get_id(), data.get_value(),0, ("Error - chyba posielania dat: "+ patch::to_string(__LINE__))));
						free(buffer);
						buffer = NULL;
						shutdown(my_socket,2);
						close(my_socket);
						return resp;
					} 
				}
				int data_len;
				wait_counter = 0;
				while(1){
					if(i.get_type() == "TCP"){
						if(buffer[7] != WRITE_SINGLE_REGISTER && buffer[7] != WRITE_MULTIPLE_REGISTER){
							data_len = 9;
							len = wait4data(data_len);
							if((int)len >= data_len){
								unsigned char tmp[1024];
								int read_count = read(my_socket, tmp, data_len);
								if(read_count == data_len){
									len = wait4data(tmp[data_len-1]);
									if((int)len >= tmp[data_len -1]){
										read_count = read(my_socket, tmp+data_len, tmp[data_len -1]);
										print_packet((char*)tmp, tmp[data_len]+data_len+2);
										if(read_count == tmp[data_len-1]){
											process_response_tcp(slave, tmp);
										}
									}
									break;
								}
							} else {
								if(wait_counter++ > 5){
									push_resp_data(hp2modbus_data(slave->get_id(), data.get_value(),0, ("Error - neodpovedalo zariadenie: "+ patch::to_string(__LINE__))));
									break;
								}
							}
						} else {
							len = wait4data(buff_len);
							if(len >= (unsigned int)buff_len){
								unsigned char tmp[100];
								int read_count = read(my_socket, tmp, buff_len);
								if(read_count == buff_len){
									process_response_tcp(slave, tmp);
								}
							} else {
								usleep(1000);
								if(wait_counter++ > 3){
									push_resp_data(hp2modbus_data(slave->get_id(), data.get_value(),0, ("Error - neodpovedalo zariadenie: "+ patch::to_string(__LINE__))));
									break;
								}
							}
						}
					} else {
						if(buffer[1] != WRITE_SINGLE_REGISTER && buffer[1] != WRITE_MULTIPLE_REGISTER){
							len = wait4data(2);
							if(len > 2){
								unsigned char tmp[100];
								int read_count = read(my_socket, tmp, 3);
								if(read_count == 3){
									len = wait4data(tmp[2]+2);
									if((int)len >= tmp[2]+2){
										read_count = read(my_socket, tmp+3, tmp[2]+2);
										print_packet((char*)tmp, tmp[2]+5);
										if(read_count == tmp[2]+2){
											process_response(slave, tmp);
										}
									} else {
										read_count = read(my_socket, tmp+3, len);
										print_packet((char*)tmp, len+3);
									}
									break;
								}
							} else {
								if(wait_counter++ > 5){
									push_resp_data(hp2modbus_data(slave->get_id(), data.get_value(),0, ("Error - neodpovedalo zariadenie: "+ patch::to_string(__LINE__))));
									break;
								}
							}
						} else {
							len = wait4data(buff_len);
							if(len >= (unsigned int)buff_len){
								unsigned char tmp[100];
								int read_count = read(my_socket, tmp, buff_len);
								if(read_count == buff_len){
									process_response(slave, tmp);
								}
							} else {
								usleep(1000);
								if(wait_counter++ > 3){
									push_resp_data(hp2modbus_data(slave->get_id(), data.get_value(),0, ("Error - neodpovedalo zariadenie: "+ patch::to_string(__LINE__))));
									break;
								}
							}
						}
					}
				}
				if(buffer != NULL){
					free(buffer);
					buffer = NULL;
				}
				shutdown(my_socket,2);
				close(my_socket);
			}
		}
	}

	return resp;
}

void hp2modbus::process_response_tcp(shared_ptr<hp2modbus_slave> slave, unsigned char *buffer)
{
	string value = "";
	int data_len = 8;
	int i_val = 0;
	if(buffer[7] == READ_HOLDING_REGISTER || buffer[7] == READ_INPUT_REGISTER){
		if(buffer[data_len] != 2){
			push_resp_data(hp2modbus_data(slave->get_id(), value,0, ("Error - chybne data: "+ patch::to_string(__LINE__))));
			return ;
		}
		i_val = (buffer[data_len+1] << 8) + buffer[data_len+2];
		if(slave->get_type() == "analog"){
			value = patch::to_string(static_cast<float>(i_val)/10);
		} else if (slave->get_type() == "digital"){
			value = patch::to_string(i_val);
		}
	} else if(buffer[7] == WRITE_SINGLE_REGISTER){
		i_val = (buffer[data_len+2] << 8) + buffer[data_len+3];
		if(slave->get_type() == "analog"){
			value = patch::to_string(static_cast<float>(i_val)/10);
		} else if (slave->get_type() == "digital"){
			value = patch::to_string(i_val);
		}
	} else {
	}
	if(value != ""){
		slave->set_status(static_cast<float>(i_val)/10);
		push_resp_data(hp2modbus_data(slave->get_id(), value,0, "OK"));
	} else {
		push_resp_data(hp2modbus_data(slave->get_id(), value,0, ("Error - chybne data: "+ patch::to_string(__LINE__))));
	}

}
double hp2modbus::ConvertNumberToFloat(unsigned long number, int isDoublePrecision)
{
	int mantissaShift = isDoublePrecision ? 52 : 23;
	unsigned long exponentMask = isDoublePrecision ? 0x7FF0000000000000 : 0x7f800000;
	int bias = isDoublePrecision ? 1023 : 127;
	int signShift = isDoublePrecision ? 63 : 31;
	
	int sign = (number >> signShift) & 0x01;
	int exponent = ((number & exponentMask) >> mantissaShift) - bias;
	
	int power = -1;
	double total = 0.0;
	for ( int i = 0; i < mantissaShift; i++ ){
		int calc = (number >> (mantissaShift-i-1)) & 0x01;
		total += calc * pow(2.0, power);
		power--;
	}
	double value = (sign ? -1 : 1) * pow(2.0, exponent) * (total + 1.0);
	
	return value;
}

void hp2modbus::process_response(shared_ptr<hp2modbus_slave> slave, unsigned char *buffer)
{
	string value = "";
	long int i_val = 0;
	if(buffer[1] == READ_HOLDING_REGISTER){
		if(buffer[2] != slave->get_register_size()*2){
			push_resp_data(hp2modbus_data(slave->get_id(), value,0, ("Error - chybne data: "+ patch::to_string(__LINE__))));
			return ;
		}
		if(slave->get_register_size() == 2){
			i_val = (buffer[3] << 24) +(buffer[4] << 16) +(buffer[5] << 8) + buffer[6];
		} else {
			i_val = (buffer[3] << 8) + buffer[4];
		}
		if(slave->get_type() == "analog"){
			value = patch::to_string(static_cast<float>(i_val)/10);
			slave->set_status(static_cast<float>(i_val)/10);
		} else if (slave->get_type() == "ieee32"){
			value = patch::to_string(this->ConvertNumberToFloat(i_val,0));
			slave->set_status(this->ConvertNumberToFloat(i_val,0));
		} else if (slave->get_type() == "digital"){
			value = patch::to_string(i_val);
			slave->set_status(i_val);
		}
	} else if(buffer[1] == WRITE_SINGLE_REGISTER){
		i_val = (buffer[4] << 8) + buffer[5];
		if(slave->get_type() == "analog"){
			value = patch::to_string(static_cast<float>(i_val)/10);
		} else if (slave->get_type() == "digital"){
			value = patch::to_string(i_val);
		}
	} else {
	}
	if(value != ""){
		push_resp_data(hp2modbus_data(slave->get_id(), value,0, "OK"));
	} else {
		push_resp_data(hp2modbus_data(slave->get_id(), value,0, ("Error - chybne data: "+ patch::to_string(__LINE__))));
	}
}

void hp2modbus::push_resp_data(hp2modbus_data data)
{
	my_rx_data->mtx.lock();
	my_rx_data->my_data.push_back(data);
	my_rx_data->mtx.unlock();
}
char *hp2modbus::create_send_data(shared_ptr<hp2modbus_slave> slave, string to_value, int *len, string master_type)
{
	int cc=0;
	unsigned short checksum = 0;
	unsigned char *res = (unsigned char *)malloc(BUF_MAX);
	if(res == NULL){
		return (char*)res;
	}
	if(master_type == "TCP"){
		res[cc++] = 0x00;
		res[cc++] = 0x01;
		res[cc++] = 0x00;
		res[cc++] = 0x00;
		res[cc++] = 0x00;
		res[cc++] = 0x06;
	}
	res[cc++] = static_cast<unsigned char> (slave->get_address());
	if(to_value == ""){
		res[cc++] = READ_HOLDING_REGISTER;
		res[cc++] = static_cast<unsigned char> (slave->get_register()>>8);
		res[cc++] = static_cast<unsigned char> (slave->get_register());
		res[cc++] = 0x00;
		res[cc++] = static_cast<unsigned char> (slave->get_register_size());
	} else {
		res[cc++] = WRITE_SINGLE_REGISTER;
		res[cc++] = static_cast<unsigned char> (slave->get_register()>>8);
		res[cc++] = static_cast<unsigned char> (slave->get_register());
		if(slave->get_type() == "analog"){
			int i_value = (int)(100*patch::string2float(to_value));
			res[cc++] = static_cast<unsigned char>(i_value >> 8);
			res[cc++] = static_cast<unsigned char>( i_value & 0xff);
		} else if (slave->get_type() == "digital"){
			res[cc++] = 0x00;
			res[cc++] = static_cast<unsigned char>(patch::string2int(to_value));
		} else {
			free(res);
			return NULL;
		}
	}
	if(master_type == "moxa"){
		checksum = CRC16(res, cc);
		res[cc++] = checksum >> 8;
		res[cc++] = checksum & 0xff;
	}
	*len = cc;

	this->print_packet((char*)res, *len, 1);
	res[cc] = '\0';

	return (char *)res;
}

string hp2modbus::process_db(MYSQL *conn)
{	
	string query= "";
	int query_state;
	MYSQL_RES *result;
	MYSQL_ROW row;
	vector<string> items_idents;

	query = "SELECT ITEM from STATUSES";
	query_state = mysql_query(conn, query.c_str());
	if(query_state != -1){
		result = mysql_store_result(conn);
		if(result->row_count > 0){
			while ((row = mysql_fetch_row(result)) != NULL ) {
				if(row[0] != NULL){
					items_idents.push_back(row[0]);
				}
			}
		}
		if(result != NULL){
			mysql_free_result(result);
		}
		query = "INSERT INTO STATUSES (ITEM, STATUS, MODIFICATION) VALUES ";
		for_each(my_slaves.begin(), my_slaves.end(), [&](shared_ptr<hp2modbus_slave> slave){
				bool add_item = true;
				for(auto i: items_idents){
					if(i == slave->get_id()){
						add_item = false;
						break;
					}
				}
				if(add_item){
					query += "('"+slave->get_id()+"', '0', NOW()),";
				}
			});				
		if(query[query.length()-1] != ' '){
			execute_query(query.substr(0,query.length()-1), conn);
		}
	}
	return "";
}

Json::Value hp2modbus::get_shm_data()
{
	Json::Value res;
	int cc=0;
	for(auto i: my_slaves){
		res[cc]["ident"] = i->get_id();
		res[cc++]["status"] = i->get_status();
	}
	//cout << res << endl;
	return res;
}

int hp2modbus::ioctl_data()
{
	int bytes=0;
	ioctl(my_socket,FIONREAD,&bytes);
	return bytes;
}


int hp2modbus::wait4data(int data_size)
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
		std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME_THREAD));
	}
	return bytes;
}
void hp2modbus::print_packet(char *data, int len, int direction)
{
	if(my_modbus_log){
		if(direction == 0){ // Rx
			cout << "[TCP]>Rx: ";
		} else {
			cout << "[TCP]>Tx: ";
		}
		for(int i=0; i<len; i++){
			printf("%02x ",data[i]&0xff);
		}
		cout << endl;
	}
}

unsigned short hp2modbus::CRC16(unsigned char *puchMsg,unsigned short usDataLen)
{
	static unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
} ;
static unsigned char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
} ;
	unsigned char uchCRCHi = 0xFF ; 
	unsigned char uchCRCLo = 0xFF ; 
	unsigned uIndex ; 
	while (usDataLen--) {
		uIndex = uchCRCHi ^ *puchMsg++ ;
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (uchCRCHi << 8 | uchCRCLo) ;
}
int hp2modbus::execute_query(string query, MYSQL *conn)
{
	int query_state;
	//cout << "query: " << query << endl;
	if(conn != NULL){
		query_state = mysql_query(conn, query.c_str());
		if (query_state != 0) {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			cout << bla << endl;
			//m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
#ifndef AXON_SERVER 
			mysql_library_end();
#endif
			conn = NULL;
			return -1;
		}
	} else {
		return -1;
	}
	return 0;
}
							/*
							len = wait4data(1);
							if(len > 0){
								char znak;
								read(my_socket,&znak,1);
								printf("%02x ",znak & 0xff);
							} else {
								break;
							}
							*/
	

