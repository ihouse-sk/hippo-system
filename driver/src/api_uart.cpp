#include "../include/api_uart.h"

using namespace std;

api_uart::api_uart(log_class *c_log_class)
{
	m_socket_state = 0;
	m_log_class = c_log_class;
}

void api_uart::xb_close_serial()
{
	if(m_socket != -1){
		close(m_socket);
	}
	m_socket = -1;
	m_socket_state = -1;
}

int api_uart::xb_read_data(unsigned char *packet,int data_size)
{
	mtx.lock();
	int pocet = read(m_socket, packet, data_size);
	mtx.unlock();
	return pocet;
}

int api_uart::xb_write_data(unsigned char *packet, int data_size)
{
	mtx.lock();
	int pocet = write(m_socket,packet,data_size);
	mtx.unlock();
	return pocet;
}

void api_uart::clear_input()
{
	unsigned char znak;
	int bytes;
	unsigned char counter =0;
	while(1){
		bytes = xb_ioctl_data();
		if(bytes > 0){
			for(int i=0; i< bytes; i++){
				xb_read_data(&znak,1);
			}
			if(counter > 0){
				counter--;
			}
		} else {
			break;
		}
		usleep(100000);
		counter++;
		if(counter > 20){
			break;
		}
	}
}

bool api_uart::check_xbee_baud()
{
	clear_input();
	sleep(1);
	int bytes;
	string data = "+++";
	xb_write_data((unsigned char *)data.c_str(),3);
	sleep(1);
	for(int i=0; i<10; i++){
		ioctl(m_socket, FIONREAD, &bytes);
		if(bytes > 2){
			unsigned char *read_data = (unsigned char *)malloc(bytes+2);
			xb_read_data(read_data,bytes);
			read_data[bytes] = '\0';
			/*for(int i=0; i<bytes; i++){
				if(read_data[i] == 0x4f){
					printf("\n");
				}
				printf("%02x ", read_data[i]);
			}
			printf("\n");*/
			std::string str((char*)read_data);
			free(read_data);
			if(str.find("OK") != std::string::npos){
				return true;
			} else {
				return false;
			}
		}
		usleep(150000);
	}
	return false;

	/*
	clear_input();
	int bytes;
	string data = "+++";
	xb_write_data((unsigned char *)data.c_str(),3);
	usleep(500000);
	for(int i=0; i<10; i++){
		ioctl(m_socket, FIONREAD, &bytes);
		if(bytes > 2){
			break;
		}
		usleep(150000);
	}
	cout << "Pocet bytov v check_xbee_baud: " << bytes << endl;
	if(bytes > 0){
		unsigned char *read_data = (unsigned char *)malloc(bytes+2);
		xb_read_data(read_data,bytes);
		read_data[bytes] = '\0';
		for(int i=0; i<bytes; i++){
			printf("%02x ", read_data[i]);
		}
		printf("\n");
		std::string str((char*)read_data);
		free(read_data);
		if(str.find("OK") != std::string::npos){
			return true;
		} else {
			return false;
		}
	}
	return false;
	*/
}

void api_uart::setup_xbee_at()
{
	int bytes;
	vector<string> data;
	data.push_back("ATAP01\r");
	data.push_back("ATD005\r");
	data.push_back("ATBD06\r");
	data.push_back("ATCN\r");

	clear_input();

	for(vector<string>::iterator it = data.begin(); it != data.end(); ++it){
		xb_write_data((unsigned char *)it->c_str(), it->length());
		for(int i = 0; i< TIMEOUT; i++){
			ioctl(m_socket, FIONREAD, &bytes);
			if(bytes > 1){
				break;
			}
			usleep(SLEEP_TIME);
		}
		if(bytes > 0){
			unsigned char *read_data = (unsigned char *)malloc(bytes+2);
			memset(read_data,0,bytes+2);
			xb_read_data(read_data,bytes);
			if(read_data != NULL){
				//cout << "!!read_data setting: " << read_data<<endl;
			}
			free(read_data);
		} 	
	}
}

int api_uart::xb_open_serial(std::string dev_name)
{
	unsigned int baud_rate;
	struct termios newtio;
	vector<unsigned int> vec_baud;
	vec_baud.push_back(B57600);
	vec_baud.push_back(B9600);
	vec_baud.push_back(B38400);
	vec_baud.push_back(B115200);

	memset(&newtio, 0, sizeof(newtio));
	m_socket = -1;

	if((m_socket = open(dev_name.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0){
		fprintf(stderr,"Device open error: %s\n",dev_name.c_str());
		m_log_class->ih_log_write(LOG_ERROR,4,"Device open error: %s\n",dev_name.c_str());
		xb_close_serial();
		m_socket_state = -1;
		return -1;
	} else {
		fprintf(stderr,"Device open successfully: %s\n",dev_name.c_str());
		m_log_class->ih_log_write(LOG_ERROR,3,"Device open successfully: %s\n",dev_name.c_str());
		//m_log_class->ih_log_write(LOG_ERROR,3,);
	}
	
	newtio.c_iflag	= IGNPAR;
	newtio.c_oflag	= 0;
	newtio.c_lflag	= 0;
	newtio.c_cc[VTIME] = 0;	
	newtio.c_cc[VMIN] = 0;
	newtio.c_cflag	= CS8|CLOCAL|CREAD|B57600;
	newtio.c_iflag	= IGNPAR;
	tcflush(m_socket, TCIFLUSH);
	tcsetattr(m_socket, TCSANOW, &newtio);

	if(this->check_xbee_baud()){
		setup_xbee_at();
	} else {
		for(vector<unsigned int>::iterator it = vec_baud.begin(); it != vec_baud.end();++it){
			tcgetattr(m_socket, &newtio);
			newtio.c_cflag	= 0;
			newtio.c_cflag	= CS8|CLOCAL|CREAD|*it;
			tcflush(m_socket, TCIFLUSH);
			tcsetattr(m_socket, TCSANOW, &newtio);
			if(this->check_xbee_baud()){
				setup_xbee_at();
				newtio.c_cflag	= 0;
				newtio.c_cflag	= CS8|CLOCAL|CREAD|B57600;
				tcflush(m_socket, TCIFLUSH);
				tcsetattr(m_socket, TCSANOW, &newtio);
				break;
			}
		}
	}
	if(DEBUG){
		tcgetattr(m_socket, &newtio);
		baud_rate = cfgetispeed(&newtio);
		cout << "Baud: " << baud_rate << endl;
	}

	baud_rate = cfgetispeed(&newtio);
	if(baud_rate != B57600){
		m_socket_state = -1;
		m_log_class->ih_log_write(LOG_ERROR,3,"Nespravny baudrate: %d, pozadovany: %d,%s:%d\n",baud_rate,B57600,__FILE__,__LINE__ );
		return -1;
	}
	m_socket_state = 1;
	return 0;
}

int api_uart::xb_init()
{
	std::vector<string> dev_list;
//	dev_list.push_back("/dev/ttyAMA0");
	dev_list.push_back("/dev/xbee");
	dev_list.push_back("/dev/ttyUSB0");
	dev_list.push_back("/dev/ttyUSB1");
	
	for(std::vector<string>::iterator it = dev_list.begin(); it != dev_list.end(); ++it){
		if(xb_open_serial(*it) != -1){
			break;
		} 
	}
	if(m_socket_state == -1){
		m_log_class->ih_log_write(LOG_ERROR,3,"Nepodarilo sa spravne vytvorit spojenie s xbee! %s:%d\n",__FILE__,__LINE__ );
		return -1;
	}
	return 0;
}

int api_uart::xb_ioctl_data()
{
	int bytes=0;
	if(m_socket_state){
		ioctl(m_socket,FIONREAD,&bytes);
		return bytes;
	}
	return 0;
}
