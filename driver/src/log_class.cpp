#include "../include/log_class.h"

log_class::log_class(int log_verbose,std::string control_socket,bool source)
{
	m_control_socket_name = control_socket;
	m_log_verbose = log_verbose;
	m_source_install = source;

	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );
	char *tlacit;
	if((tlacit =(char*)malloc(sizeof(char) *100)) == NULL){
		//////ERROR
		//ih_log_write(cm_data->log_error,"Allocation error, %s:%d\n",__FILE__,__LINE__);
	}
	std::string path;
	if(m_source_install){
		path = "logs";
	} else {
		path = "/usr/share/driver/logs";
	}

	if(m_log_verbose != 0){
		m_log_error = open((path+"/log_error").c_str(), O_RDONLY);
		if(m_log_error >= 0){
			close(m_log_error);
			//if(!(m_log_error = open("logs/log_error", O_RDWR | O_CREAT, 0666))){
			if(!(m_log_error = open((path+"/log_error").c_str(), O_RDWR | O_APPEND, 0666))){
				fprintf(stderr,"Subor sa nepodarilo otvorit,%s:%d\n",__FILE__,__LINE__);
				/////ERROR
			}
		} else {
			if(!(m_log_error = open((path+"/log_error").c_str(), O_RDWR |O_CREAT, 0666))){
				fprintf(stderr,"Subor sa nepodarilo otvorit,%s:%d\n",__FILE__,__LINE__);
				/////ERROR
			}
		}
		sprintf(tlacit,"%s/log_com%d_%02d_%02d.txt",path.c_str(),timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);
		m_day_number = timeinfo->tm_yday;
		//sprintf(tlacit,"%s/log_com%d.txt",path.c_str(),timeinfo->tm_yday);
		m_log_com = open(tlacit, O_RDONLY);
		if( m_log_com >= 0){
			close(m_log_com);
			if(!(m_log_com = open(tlacit, O_RDWR | O_APPEND))){
				ih_log_write(LOG_ERROR,4,"Nepodarilo sa otvorit subor %s,%s:%d\n",tlacit,__FILE__,__LINE__); 
			/////ERROR
			}
		} else {
			if(!(m_log_com = open(tlacit, O_RDWR | O_CREAT, 0666))){
				ih_log_write(LOG_ERROR,4,"Nepodarilo sa otvorit subor %s,%s:%d\n",tlacit,__FILE__,__LINE__); 
			/////ERROR
			}
		}
	}
	free(tlacit);
}

void log_class::new_day()
{
	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );
	if(m_day_number != timeinfo->tm_yday){
		mtx.lock();
		char *tlacit;
		if((tlacit =(char*)malloc(sizeof(char) *100)) == NULL){
			//////ERROR
			//ih_log_write(cm_data->log_error,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		}
		std::string path;
		if(m_source_install){
			path = "logs";
		} else {
			path = "/usr/share/driver/logs";
		}
	
		if(m_log_verbose != 0){
			//sprintf(tlacit,"%s/log_com_%d_%02d_%02d.txt",path.c_str(),timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_wday);
			sprintf(tlacit,"%s/log_com%d_%02d_%02d.txt",path.c_str(),timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);
			if(!(m_log_com = open(tlacit, O_RDWR | O_CREAT, 0666))){
				ih_log_write(LOG_ERROR,4,"Nepodarilo sa otvorit subor %s,%s:%d\n",tlacit,__FILE__,__LINE__); 
			/////ERROR
			}
		}
		free(tlacit);
		mtx.unlock();
	}
}

void log_class::ih_log_write(int log_type, int verbose,const char *format,...)
{
	mtx.lock();
	int file_desc=0;

	if(verbose >= m_log_verbose){
		if(log_type == LOG_ERROR){
			file_desc = m_log_error;
		}
		if(log_type == LOG_COMM){
			file_desc = m_log_com;
		}
		if(file_desc != 0){
			//char buffer[512],cas[512];
			char buffer[4096],cas[512];
			int status;
			va_list args;
			time_t rawtime;
			struct tm * timeinfo;
			rawtime=time(NULL);
			timeinfo = localtime ( &rawtime );
		
			va_start (args, format);
			vsprintf (buffer,format, args);	
			sprintf(cas,"%02d.%02d.%02d, %02d:%02d:%02d ",timeinfo->tm_mday,timeinfo->tm_mon+1,timeinfo->tm_year+1900,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
		
			status = write(file_desc,cas,strlen(cas));
			status = write(file_desc,buffer,strlen(buffer));
			if(status){
			}
			
			va_end (args);
			if(log_type == LOG_ERROR && verbose >= 3){
				send_packet((unsigned char*)buffer);
			}
		}
	}
	mtx.unlock();
}

int log_class::send_packet(unsigned char *socket_buf)
{
	if(socket_buf != NULL){
		struct sockaddr_un address_unix;	
		int len_add,buf_len,result,status1;
		m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		address_unix.sun_family = AF_UNIX;
		strcpy(address_unix.sun_path,m_control_socket_name.c_str());
		len_add = sizeof(address_unix);
	
		if((result = connect(m_sockfd, (struct sockaddr *)&address_unix, len_add)) == -1){
			//ih_log_write(LOG_ERROR,1,"Cannot open socket to c++,%s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		if(result == 0){
			buf_len=strlen((char*)socket_buf);
			for(int i=0; i<2*PACKET_MAX-buf_len-1; i++){
				strcat((char*)socket_buf,"_");
			}
			status1 = write(m_sockfd,(char*)socket_buf,2*PACKET_MAX);
			if(status1){}
		}
		shutdown(m_sockfd,2);
		close(m_sockfd);
	}
	return 0;
}

