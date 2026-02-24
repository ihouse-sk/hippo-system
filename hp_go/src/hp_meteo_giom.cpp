#include "../include/hp_meteo_giom.h"

using namespace std;

hp_meteo_giom::hp_meteo_giom(std::string file_socket, int check_interval,std::string meteo_ip)
{
	m_file_socket = file_socket;
	m_check_interval = check_interval;
	m_meteo_ip = meteo_ip;
	mib_data_t tmp;

	tmp.name = "rpressure";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.3.0";
	tmp.value_type = "float";
	m_mib_data.push_back(tmp);

	tmp.name = "windspeed";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.4.0";
	tmp.value_type = "float";
	m_mib_data.push_back(tmp);

	tmp.name = "windgust";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.5.0";
	tmp.value_type = "float";
	m_mib_data.push_back(tmp);

	tmp.name = "winddirection";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.8.0";
	tmp.value_type = "string";
	m_mib_data.push_back(tmp);

	tmp.name = "rhumidity";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.12.0";
	tmp.value_type = "float";
	m_mib_data.push_back(tmp);

	tmp.name = "temperature";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.14.0";
	tmp.value_type = "float";
	m_mib_data.push_back(tmp);

	tmp.name = "windchill";
	tmp.oid = "0.1.3.6.1.4.1.21287.15.15.0";
	tmp.value_type = "float";
	m_mib_data.push_back(tmp);
}

void hp_meteo_giom::do_work()
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;
	oid anOID[MAX_OID_LEN];
	size_t anOID_len;
	netsnmp_variable_list *vars;
	int sleep_counter = m_check_interval;

	string community = "public";
	snmp_set_save_descriptions(1);
	init_snmp("snmpdemoapp");
	snmp_sess_init( &session );
	session.peername = strdup(m_meteo_ip.c_str());
	session.version = SNMP_VERSION_1;
	session.community = (u_char*)community.c_str();
	session.community_len = strlen(community.c_str());
	int status;

	while (1) {
		sleep_counter++;
		if(sleep_counter > m_check_interval){
			sleep_counter = 0;
			SOCK_STARTUP;
			ss = snmp_open(&session);
			if (!ss) {
			      snmp_sess_perror("ack", &session);
			      SOCK_CLEANUP;
			      continue;
			}
			for(vector<mib_data_t>::iterator it = m_mib_data.begin(); it != m_mib_data.end(); ++it){
				pdu = snmp_pdu_create(SNMP_MSG_GET);
				anOID_len = MAX_OID_LEN;
				if (!snmp_parse_oid(it->oid.c_str(), anOID, &anOID_len)) {
					snmp_perror(".1.3.6.1.2.1.1.1.0");
					SOCK_CLEANUP;
					break;
				}
				snmp_add_null_var(pdu, anOID, anOID_len);
				status = snmp_synch_response(ss, pdu, &response);
				if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
				//	for(vars = response->variables; vars; vars = vars->next_variable){
				//		print_variable(vars->name, vars->name_length, vars);
				//	}
					for(vars = response->variables; vars; vars = vars->next_variable) {
						if (vars->type == ASN_OCTET_STR) {
							char *sp = (char *)malloc(1 + vars->val_len);
							memcpy(sp, vars->val.string, vars->val_len);
							sp[vars->val_len] = '\0';
							it->value = sp;
							unsigned char *socket_data = (unsigned char*)malloc(2*PACKET_MAX);
							if(socket_data != NULL){
								sprintf((char *)socket_data, "CLS_meteo_%s_%s", it->name.c_str(), it->value.c_str());
								send_packet(socket_data);
								free(socket_data);
							}
							free(sp);
						} else {
							int bla=0;
							if(bla) {}
						}
					}
				} else {
					if(status == STAT_SUCCESS) {
						fprintf(stderr, "Error in packet\nReason: %s\n",snmp_errstring(response->errstat));
					} else if (status == STAT_TIMEOUT) {
						fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
					} else { 
						snmp_sess_perror("snmpdemoapp", ss);					
					}
				}
				if (response){
					snmp_free_pdu(response);
					response = NULL;
				}
			}
			snmp_close(ss);
			SOCK_CLEANUP;
			//print_meteo_data();
		}
		sleep(1);
	}

}

void hp_meteo_giom::operator()()
{
	do_work();
}

int hp_meteo_giom::send_packet(unsigned char *socket_buf)
{
	if(socket_buf != NULL){
		struct sockaddr_un address_unix;	
		int len_add,buf_len,result,status1,m_sockfd;
		m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		address_unix.sun_family = AF_UNIX;
		strcpy(address_unix.sun_path,m_file_socket.c_str());
		len_add = sizeof(address_unix);
	
		if((result = connect(m_sockfd, (struct sockaddr *)&address_unix, len_add)) == -1){
			//m_log_class->ih_log_write(LOG_ERROR,3,"Cannot open socket to c++,%s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		if(result == 0){
			buf_len=strlen((char*)socket_buf);
			for(int i=0; i<2*PACKET_MAX-buf_len-1; i++){
				strcat((char*)socket_buf,"_");
			}
			status1 = write(m_sockfd,(char*)socket_buf,2*PACKET_MAX);
			if(status1){
			}
		}
		shutdown(m_sockfd,2);
		close(m_sockfd);
	}
	return 0;
}


void hp_meteo_giom::print_meteo_data()
{
	for(vector<mib_data_t>::iterator it = m_mib_data.begin(); it != m_mib_data.end(); ++it){
		cout << it->name << ":\t" << it->value << endl;
	}
	cout << endl;
}


hp_meteo_giom::~hp_meteo_giom(){}
