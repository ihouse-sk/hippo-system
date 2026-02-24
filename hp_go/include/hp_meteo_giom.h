#ifndef HP_METEO_GIOM_H
#define HP_METEO_GIOM_H

#include <iostream>
#include <vector>
#include <map>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>


#define PACKET_MAX 100

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

typedef struct {
	std::string name;
	std::string oid;
	std::string value;
	std::string value_type;
} mib_data_t;

class hp_meteo_giom{
public:
	hp_meteo_giom(std::string,int check_interval = 59, std::string meteo_ip = "192.168.0.2");
	~hp_meteo_giom();
	void operator()();
	void do_work();
private:
	std::string m_file_socket;
	std::string m_meteo_ip;
	int m_check_interval;

	std::vector<mib_data_t> m_mib_data;
	int send_packet(unsigned char *socket_buf);
	void print_meteo_data();
};
#endif
