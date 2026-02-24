#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/un.h>
#include <string>

struct Bug_socket {
	std::string socket;
	int sd;
	struct sockaddr_un m_unix_address;
	int open_socket(){
		sd = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sd <0) {
			std::cout <<"Chyba socketu" << std::endl;
			return sd;
		}
		m_unix_address.sun_family = AF_UNIX;
		strcpy(m_unix_address.sun_path,socket.data());
		return 1;
	}
	int send_data(std::string mess) {
		if(connect(sd, (struct sockaddr *)&m_unix_address, sizeof(m_unix_address)) != -1){
			write(sd, mess.data(), sizeof(char)*mess.length());
			shutdown(sd,2);
			close(sd);
			return 0;
		}
		return -1;
	}
};
