#include "../include/mesh_xbee.h"
#include <stdexcept>

using namespace std;

#ifdef TESTING
static int global_term=0;

void sig_handler(int signum)
{
	global_term++;
	if(global_term >1){
		exit(1);
	}
}
#endif

xbee::xbee()
{
	bool source_install =true;
	m_comm_data = new comm_data(0,0,source_install);

	read_xml_startup();

	m_status_cthread = 0;
	m_status_xthread= 0;

	m_log_class = new log_class(m_log_verbose,m_cls_socket,source_install);
	m_api_uart = new api_uart(m_log_class);
	m_xbee_listen = new xbee_listen(m_api_uart,m_cls_socket,m_log_class,m_comm_data);
	m_control_listen = new control_listen(m_api_uart,m_xbee_socket,m_log_class,m_comm_data);
	srand(time(NULL));
#ifdef TESTING
	if (signal(SIGINT, sig_handler) == SIG_ERR){
		printf("\ncan't catch SIGINT\n");
	}
	signal(SIGKILL, sig_handler);
	signal(SIGTERM, sig_handler);
#endif

}

int xbee::init()
{	
	if((m_api_uart->xb_init()) == -1){
		CHECK();
		delete_objects();
		return -1;
	}
	if((m_control_listen->init()) == -1){
		CHECK();
		delete_objects();
		return -1;
	}
	return 0;
}

int xbee::run()
{
	int aes_checking_time=10;

//	boost::thread th1(*m_xbee_listen);
//	boost::thread th2(*m_control_listen);
	th1 = boost::thread(*m_xbee_listen);
	th2 = boost::thread(*m_control_listen);

	if(m_aes_init){
		if(first_aes_init() == -1){
			m_aes_option = false;
		}
	}
		
	if(m_nd_option){
		sleep(1);
		send_ND_command();
	}

	while(1){
		sleep(1);
#ifdef TESTING
		if(global_term > 0){
			send_quit();
			cout << "Exiting . . ."<<endl;
		}
#endif
		if(m_comm_data->get_c_status() == -1 && m_comm_data->get_x_status() == -1){
			break;
		}
		if(m_aes_option && (time(NULL)%aes_checking_time == 0)){
			int key_status = check_key_validity();	
			if(key_status == 0){
				if(change_aes_key() != 0){
					aes_checking_time = 600;
				} else {
					aes_checking_time = 10;
				}
			}
		}
		if(m_comm_data->get_send_time() > m_comm_data->get_recieve_time()){
			if(m_comm_data->get_recieve_time()+45 < time(NULL)){
				m_comm_data->set_send_time(time(NULL));
				m_comm_data->set_recieve_time(time(NULL));
				reset_driver();
			}
		}
		if(m_comm_data->get_reset_xbee()){
			m_comm_data->set_reset_xbee();
			reset_driver();
		}
		if(m_comm_data->get_add_new()){
			this->add_new_xbee("macccc");
		}
	//	m_log_class->new_day();
	}
	th2.join();
	th1.join();
	delete_objects();
	return 0;
}

void xbee::reset_driver()
{
	m_log_class->ih_log_write(LOG_ERROR,3,"drv_reinit %s:%d\n",__FILE__,__LINE__);
	m_comm_data->set_x_status(-1);
	m_api_uart->xb_close_serial();
	th1.join();
	m_api_uart->xb_init();
	th1 = boost::thread(*m_xbee_listen);
	m_comm_data->set_send_time(time(NULL));
	m_comm_data->set_recieve_time(time(NULL));
}

int xbee::ee_koordinator(bool active, unsigned char *key)
{
	unsigned char *mess=NULL;
	if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	if(key != NULL){
		sprintf((char*)mess,"KY_%s%c",key,'\0');
		send_command(mess);
	}
	if(active){
		sprintf((char*)mess,"EE_%s%c","1",'\0');
	} else {
		sprintf((char*)mess,"EE_%s%c","0",'\0');
	}
	send_command(mess);
	sprintf((char*)mess,"AC_%s%c","-1",'\0');
	send_command(mess);
	free(mess);
	mess=NULL;

	return 0;
}

int xbee::add_new_xbee(std::string xbee_mac)
{
	int i;
	unsigned char *key;
	unsigned char *mess;
	int counter = 0;
	
	MYSQL *conn;
	conn = mysql_init(NULL);

	if((mysql_real_connect(conn, m_mysql_server.c_str(),"root", m_mysql_pass.c_str(),"driver",0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
		mysql_close(conn);
		mysql_library_end();
		return -1;
	} else {
		key = get_key(conn,m_comm_data->get_aes_actual());
		if(key == NULL){
			m_log_class->ih_log_write(LOG_ERROR, 3, "Chyba stahovania aes kluca, %s:%d", __FILE__, __LINE__);
			mysql_close(conn);
			mysql_library_end();
			return -1;
		}
		m_comm_data->add_xbee_dev(xbee_dev(xbee_mac, m_comm_data->get_xbee_size()+1));	
		this->ee_koordinator(false);
		m_comm_data->set_aes_changing(m_comm_data->get_xbee_size()-1);

		if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		sprintf((char *)mess,"%d_%x_KY_%s_%s%c",m_comm_data->get_xbee_ident(m_comm_data->get_xbee_size()-1),27, key,"OPTIONAL",'\0');
		send_command(mess);
		while(1){
			usleep(100000);
			if(m_comm_data->get_xbee_status(m_comm_data->get_xbee_size()-1) == AES_CHAN_OK){
				
			}
			counter++;
			if(counter > 40){
				m_log_class->ih_log_write(LOG_ERROR,4,"Chyba pridania noveho xbee, AES zapnute, %s:%d\n", __FILE__,__LINE__);
				free(mess);
				mess=NULL;
				//vyprsal casovy limit na odpoved od xbee
				return -1;
			}
		}

	}

	std::string query = "INSERT INTO xbees (id_actual,id_key, mac_xbee, ee_enable, status) VALUES ("+patch::to_string(m_comm_data->get_xbee_size()+",(SELECT id_key from aes_keys ORDER by id_key DESC LIMIT 1), '"+xbee_mac+"', 1, 1");
	if(mysql_update(query.c_str()) == -1){
		return -1;
	}
	return 0;
}

int xbee::change_aes_key()
{
	m_log_class->ih_log_write(LOG_ERROR,3,"Changing AES key\n");
	unsigned char *new_key = generate_new_key();

	if(new_key != NULL){
		applied_first_key(new_key);
		int counter=0;
		while(1){
			usleep(100000);
			bool all_response=true;
			std::vector<int> xbee_error;
			for(int i=0; i< m_comm_data->get_xbee_size(); i++){
				int status=m_comm_data->get_xbee_status(i);
				if(status == AES_CHAN_NOW){
					all_response = false;
				}
				if(status != AES_CHAN_OK){
					xbee_error.push_back(i);
				}
			}
			if(all_response){
				if(xbee_error.size() == 0){
					/// ZAPNI AES a potvrd kluc
					apply_changes(new_key);
				} else {
					std::string xbee_numbers;
					for(std::vector<int>::iterator it = xbee_error.begin(); it != xbee_error.end(); ++it){
						xbee_numbers.append(patch::to_string(*it)+" ");
					}
					m_log_class->ih_log_write(LOG_ERROR,4,"AES key nebol zmeneny, No response from xbee: %s, %s:%d\n",xbee_numbers.c_str(), __FILE__,__LINE__);
					free(new_key);
					return -1;
					/// niektore xbee nie su zapnute, chyba, neaktivuje sa AES
				}
				break;
			}
			counter++;
			if(counter > 40){
				free(new_key);
				m_log_class->ih_log_write(LOG_ERROR,4,"Chyba zmeny AES kluca, niektore xbee neodpovedali, %s:%d\n", __FILE__,__LINE__);
				//vyprsal casovy limit na odpoved od xbee
				return -1;
			}
		}
		free(new_key);
		return 0;
	} else {
		return -1;
	}
}

int xbee::apply_changes(unsigned char *new_key)
{
	int i;
	unsigned char *mess;
	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		sprintf((char *)mess,"%d_%x_AC_%s%c",m_comm_data->get_xbee_ident(i),22, "-1",'\0');
		send_command(mess);
		free(mess);
		mess=NULL;
		usleep(100000);
	}
	if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	sprintf((char*)mess,"KY_%s%c",new_key,'\0'); /// nastav novy kluc koordinatorovi
	send_command(mess);
	sprintf((char*)mess,"AC_%s%c","-1",'\0');   /// apply changes na koordinatorovi
	send_command(mess);
	free(mess);
	mess=NULL;
/*
 /////////////////// POSLANIE PRIKAZU WR, aby si zapisali vsetko do non-volatile pamate
	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		sprintf((char *)mess,"%d_%x_WR_%s%c",m_comm_data->get_xbee_ident(i),i+30, "-1",'\0');
		send_command(mess);
		free(mess);
		mess=NULL;
		usleep(100000);
	}
	if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	sprintf((char*)mess,"WR_%s%c","-1",'\0');
	send_command(mess);
	free(mess);
	mess=NULL;*/

	std::string query = "INSERT INTO aes_keys (value,time_start, time_end) VALUES ('";
	query.append((char*)new_key);
	query+="',NOW(), NOW() + INTERVAL "+patch::to_string(m_key_time)+" MINUTE)";
	cout <<" Key query: " << query << endl;
	mysql_update(query.c_str());
	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		query = "UPDATE xbees set id_key = (SELECT id_key from aes_keys ORDER BY id_key DESC LIMIT 1), status = 1 where mac_xbee = '"+m_comm_data->get_xbee_mac(i)+"'";
		mysql_update(query.c_str());
	}
	m_log_class->ih_log_write(LOG_ERROR,4,"AES changed OK, valid until: %d\n", time(NULL)+m_key_time);
	return 0;
}

unsigned char *xbee::generate_new_key()
{
	int i;
	unsigned char *aes_ky_gen,znak;
	if((aes_ky_gen = (unsigned char *) malloc(sizeof(unsigned char)*17)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}
	for(i=0; i<16;i++){
	//	aes_ky_gen[i] ='l'; /// geerovanie od 40 po 122
		while(1){
			znak = (unsigned char) (rand() %82)+40;
			if(znak != '_' && znak != '\\'){
				aes_ky_gen[i] = znak;
			 	break;
			}
		}
	}
	aes_ky_gen[i] = '\0';
	return aes_ky_gen;
}

int xbee::check_key_validity()
{
	MYSQL *conn;
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string key_validity="";

	conn = mysql_init(NULL);

	if((mysql_real_connect(conn, m_mysql_server.c_str(),"root", m_mysql_pass.c_str(),"driver",0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
		mysql_close(conn);
		mysql_library_end();
		return -1;
	} else {
		int query_state = mysql_query(conn,"SELECT IF((SELECT time_end from aes_keys ORDER BY id_key DESC LIMIT 1) > NOW(), '1','0'), id_key from aes_keys order BY id_key DESC LIMIT 1");
		//cout << "IF((SELECT time_end from aes_keys ORDER BY id_key DESC LIMIT 1) > NOW(), '1','0')" << endl;
		if(query_state != 0){
			char bla[300];
			strcpy(bla, mysql_error(conn));
			m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
			mysql_library_end();
			return -1;
		} else {
			result = mysql_store_result(conn);
			while ((row = mysql_fetch_row(result)) != NULL ) {
				key_validity = row[0];
				if(row[1] != NULL){
					m_comm_data->set_aes_actual(atoi(row[1]));	
				}
			}
			if(result != NULL){
				mysql_free_result(result);
			}
		}
	}
	mysql_close(conn);
	mysql_library_end();
	if(key_validity == "1"){
		return 1;
	} else {
		return 0;
	}
}

int xbee::first_aes_init()
{
	m_log_class->ih_log_write(LOG_ERROR,3,"First aes initialisation . .");
	std::string command = "mysql -uroot -ppwd < driver.sql";
	int status;
	unsigned char *init_key;

	status = system(command.c_str());
	//status = system("mysql -uroot -ppwd driver -e \"select * from aes_keys\"");
	MYSQL *conn;
	conn = mysql_init(NULL);

	if((mysql_real_connect(conn, m_mysql_server.c_str(),"root", m_mysql_pass.c_str(),"driver",0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
		mysql_close(conn);
		mysql_library_end();
		return -1;
	} else {
		if(add_xbee2db() == -1){
			mysql_close(conn);
			mysql_library_end();
			return -1;
		}
		init_key = get_key(conn,1);
		if(init_key == NULL){
			m_log_class->ih_log_write(LOG_ERROR, 3, "Chyba stahovania aes kluca, %s:%d", __FILE__, __LINE__);
			mysql_close(conn);
			mysql_library_end();
			return -1;
		}
		//printf("Init key: %s\n", init_key);
		applied_first_key(init_key);
		int counter=0;
		std::vector<int> xbee_error;
		while(1){
			usleep(100000);
			xbee_error.clear();
			bool all_response=true;
			for(int i=0; i< m_comm_data->get_xbee_size(); i++){
				int status=m_comm_data->get_xbee_status(i);
				if(status == AES_CHAN_NOW){
					all_response = false;
				} else if(status != AES_CHAN_OK){
					xbee_error.push_back(i);
				}
			}
			if(all_response){
				if(xbee_error.size() == 0){
					/// ZAPNI AES a potvrd kluc
					turn_on_aes(init_key);
				} else {
					std::string xbee_numbers;
					for(std::vector<int>::iterator it = xbee_error.begin(); it != xbee_error.end(); ++it){
						xbee_numbers.append(patch::to_string(*it)+" ");
					}
					m_log_class->ih_log_write(LOG_ERROR,4,"No response from xbee: %s, %s:%d\n",xbee_numbers.c_str(), __FILE__,__LINE__);
					/// niektore xbee nie su zapnute, chyba, neaktivuje sa AES
				}
				break;
			}
			counter++;
			if(counter > 40){
				m_log_class->ih_log_write(LOG_ERROR,4,"Chyba aktivacie AES, niektore xbee neodpovedali, %s:%d\n", __FILE__,__LINE__);
				//vyprsal casovy limit na odpoved od xbee
				break;
			}
		}
	}
	free(init_key);
	init_key = NULL;

	mysql_close(conn);
	mysql_library_end();

	if(status){}
	return 0;
}

int xbee::turn_on_aes(unsigned char *key)
{
	int i;
	unsigned char *mess;
	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		sprintf((char *)mess,"%d_%x_EE_%s_%s%c",m_comm_data->get_xbee_ident(i),22, "1","OPTIONAL",'\0');
		send_command(mess);
		sprintf((char *)mess,"%d_%x_AC_%s%c",m_comm_data->get_xbee_ident(i),23, "-1",'\0');
		send_command(mess);
		free(mess);
		mess=NULL;
		usleep(100000);
	}
	if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	free(mess);
	mess=NULL;
	this->ee_koordinator(true, key);
	/*sprintf((char*)mess,"KY_%s%c",key,'\0');
	send_command(mess);
	sprintf((char*)mess,"EE_%s%c","1",'\0');
	send_command(mess);
	sprintf((char*)mess,"AC_%s%c","-1",'\0');
	send_command(mess);*/
/*
 /////////////////// POSLANIE PRIKAZU WR, aby si zapisali vsetko do non-volatile pamate
	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		sprintf((char *)mess,"%d_%x_WR_%s%c",m_comm_data->get_xbee_ident(i),i+30, "-1",'\0');
		send_command(mess);
		free(mess);
		mess=NULL;
		usleep(100000);
	}
	if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	sprintf((char*)mess,"WR_%s%c","-1",'\0');
	send_command(mess);
	free(mess);
	mess=NULL;*/

	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		std::string query = "UPDATE xbees set id_key = 1, ee_enable = 1, status = 1 where mac_xbee = '"+m_comm_data->get_xbee_mac(i)+"'";
		mysql_update(query.c_str());
	}
	m_log_class->ih_log_write(LOG_ERROR,3,"First aes initialisation - done");

	return 0;
}

int xbee::applied_first_key(unsigned char *key)
{
	int i;
	unsigned char *mess;
	m_comm_data->set_aes_changing();
	for(i=0; i< m_comm_data->get_xbee_size(); i++){
		if((mess = (unsigned char *)malloc(PACKET_MAX*2)) == NULL){
			m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
			return -1;
		}
		sprintf((char *)mess,"%d_%x_KY_%s_%s%c",m_comm_data->get_xbee_ident(i),i+20, key,"OPTIONAL",'\0');
		send_command(mess);
		free(mess);
		mess=NULL;
	}
	return 0;
}

unsigned char *xbee::get_key(MYSQL *conn, int key_id)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned char *res=NULL;

	if((res = (unsigned char *)malloc(17)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error, %s:%d\n",__FILE__,__LINE__);
		return NULL;
	}

	int query_state = mysql_query(conn,"SELECT value from aes_keys where id_key = 1");
	if(query_state != 0){
	
	} else {
		result = mysql_store_result(conn);
		while ((row = mysql_fetch_row(result)) != NULL ) {
			strncpy((char*)res, row[0],16);
			res[16] = '\0';
		}
		if(result != NULL){
			mysql_free_result(result);
		}
	}

	return res;
}

int xbee::add_xbee2db()
{
	int i;
	std::string query = "INSERT INTO xbees (id_actual,id_key, mac_xbee, ee_enable, status) VALUES ";
	for(i=0; i<m_comm_data->get_xbee_size(); i++){
		if(i == m_comm_data->get_xbee_size() - 1){
			query += "("+patch::to_string(m_comm_data->get_xbee_ident(i))+",0,'"+m_comm_data->get_xbee_mac(i)+"', -1, -1)";
		} else {
			query += "("+patch::to_string(m_comm_data->get_xbee_ident(i))+",0,'"+m_comm_data->get_xbee_mac(i)+"', -1, -1),";
		}
	}
	if(mysql_update(query.c_str()) == -1){
		return -1;
	}
	return 0;
}

int xbee::mysql_update(const char *query)
{
	MYSQL *conn;

	int query_state;

	conn= mysql_init(NULL);

	if( ( mysql_real_connect(conn,m_mysql_server.c_str(),"root",m_mysql_pass.c_str(),"driver",0,0,0)) == NULL){
		char bla[200];
		strcpy(bla, mysql_error(conn));
		m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
		mysql_close(conn);
		mysql_library_end();
		return -1;
	} else {
		query_state = mysql_query(conn, query);
	
		if (query_state != 0) {
			char bla[200];
			strcpy(bla, mysql_error(conn));
			m_log_class->ih_log_write(LOG_ERROR, 3, "%s, %s:%d",bla, __FILE__, __LINE__);
			mysql_close(conn);
			mysql_library_end();
			return -1;
		}
	}
	mysql_close(conn);
	mysql_library_end();
	return 0;
}

inline bool xbee::exists(const std::string& name) {
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
}

void xbee::read_xml_startup()
{
	XMLNode xml_main_node;
	XMLNode xml_node; 
	m_mysql_server = "localhost";
	m_mysql_pass = "pwd";

	if(!this->exists("driver.xml")){
		cout << "Node is empty: driver.xml" << endl;
	}

	if(m_comm_data->get_source_install()){
		xml_main_node = XMLNode::openFileHelper("driver.xml","driver");
	} else {
		xml_main_node = XMLNode::openFileHelper("/usr/share/driver/driver.xml","driver");
	}

	xml_node = xml_main_node.getChildNode("xbee_socket");
	m_xbee_socket = xml_node.getAttribute("value");

	xml_node = xml_main_node.getChildNode("cls_socket");
	m_cls_socket = xml_node.getAttribute("value");

	xml_node = xml_main_node.getChildNode("log_verbose");
	m_log_verbose = atoi(xml_node.getAttribute("value"));

	xml_node = xml_main_node.getChildNode("ND_command");
	if((strncmp(xml_node.getAttribute("value"),"0",1)) == 0){
		m_nd_option = false;
	} else {
		m_nd_option = true;
	}

	xml_node = xml_main_node.getChildNode("AES_enabled");
	if((strncmp(xml_node.getAttribute("value"),"0",1)) == 0){
		m_aes_option = false;
	} else {
		m_aes_option = true;
	}

	xml_node = xml_main_node.getChildNode("AES_key_time");
	m_key_time = atoi(xml_node.getAttribute("value"));

	xml_node = xml_main_node.getChildNode("AES_init");
	if((strncmp(xml_node.getAttribute("value"),"0",1)) == 0){
		m_aes_init = false;
	} else {
		m_aes_init = true;
	}

	xml_node = xml_main_node.getChildNode("xbee_count");
	int xbee_count = atoi(xml_node.getAttribute("value"));

	for(int i=0; i< xbee_count; i++){
		xml_node = xml_main_node.getChildNode("xbee",i).getChildNode("id");
		int xbee_id=  atoi(xml_node.getAttribute("value"));
		xml_node = xml_main_node.getChildNode("xbee",i).getChildNode("mac");;
		m_comm_data->add_xbee_dev(xbee_dev(xml_node.getAttribute("value"), xbee_id));	
	}
	//m_comm_data->print_xbee_dev();
	
	/*	m_xbee_socket = "/home/user/xbee";
	m_cls_socket = "/home/user/control";
	m_log_verbose = 1;
	m_nd_option = false;
	m_aes_option = false;
	int xbee_count=4;
	m_comm_data->add_xbee_dev(xbee_dev("00 13 a2 00 40 ad ef 0c ff fe",1));	
	m_comm_data->add_xbee_dev(xbee_dev("00 13 a2 00 40 a6 bd de ff fe",37));	

	m_comm_data->print_xbee_dev();

	*/
	
}

void xbee::send_ND_command()
{
	unsigned char *socket_buf;
	if((socket_buf = (unsigned char*)malloc(2*PACKET_MAX)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error,%s:%d\n",__FILE__,__LINE__);
		return;
	}

	strcpy((char *)socket_buf, "ND_-1");

	struct sockaddr_un address_unix;	
	int len_add,buf_len,result,status1;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	address_unix.sun_family = AF_UNIX;
	strcpy(address_unix.sun_path,m_xbee_socket.c_str());
	len_add = sizeof(address_unix);

	if((result = connect(sockfd, (struct sockaddr *)&address_unix, len_add)) == -1){
		m_log_class->ih_log_write(LOG_ERROR,3,"Cannot open socket to my listener,%s:%d\n",__FILE__,__LINE__);
		return ;
	}
	if(result == 0){
		buf_len=strlen((char*)socket_buf);
		for(int i=0; i<2*PACKET_MAX-buf_len-1; i++){
			strcat((char*)socket_buf,"_");
		}
		status1 = write(sockfd,(char*)socket_buf,2*PACKET_MAX);
		if(status1){}
	}
	shutdown(sockfd,2);
	close(sockfd);
	free(socket_buf);
}
void xbee::send_quit()
{
	unsigned char *socket_buf;
	if((socket_buf = (unsigned char*)malloc(2*PACKET_MAX)) == NULL){
		m_log_class->ih_log_write(LOG_ERROR,4,"Allocation error,%s:%d\n",__FILE__,__LINE__);
		return;
	}

	strcpy((char *)socket_buf, "quit_-1");

	struct sockaddr_un address_unix;	
	int len_add,buf_len,result,status1;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	address_unix.sun_family = AF_UNIX;
	strcpy(address_unix.sun_path,m_xbee_socket.c_str());
	len_add = sizeof(address_unix);

	if((result = connect(sockfd, (struct sockaddr *)&address_unix, len_add)) == -1){
		m_log_class->ih_log_write(LOG_ERROR,3,"Cannot open socket to my listener,%s:%d\n",__FILE__,__LINE__);
		return ;
	}
	if(result == 0){
		buf_len=strlen((char*)socket_buf);
		for(int i=0; i<2*PACKET_MAX-buf_len-1; i++){
			strcat((char*)socket_buf,"_");
		}
		status1 = write(sockfd,(char*)socket_buf,2*PACKET_MAX);
		if(status1){}
	}
	shutdown(sockfd,2);
	close(sockfd);
	free(socket_buf);
}

int xbee::send_command(unsigned char *socket_buf)
{
	struct sockaddr_un address_unix;	
	int len_add,buf_len,result,status1;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	address_unix.sun_family = AF_UNIX;
	strcpy(address_unix.sun_path,m_xbee_socket.c_str());
	len_add = sizeof(address_unix);

	if((result = connect(sockfd, (struct sockaddr *)&address_unix, len_add)) == -1){
		m_log_class->ih_log_write(LOG_ERROR,3,"Cannot open socket to my listener,%s:%d\n",__FILE__,__LINE__);
		return -1;
	}
	if(result == 0){
		buf_len=strlen((char*)socket_buf);
		for(int i=0; i<2*PACKET_MAX-buf_len-1; i++){
			strcat((char*)socket_buf,"_");
		}
		status1 = write(sockfd,(char*)socket_buf,2*PACKET_MAX);
		if(status1){}
	}
	shutdown(sockfd,2);
	close(sockfd);
	return 0;
}

void xbee::delete_objects()
{
	delete m_log_class;
	delete m_api_uart;
	delete m_xbee_listen;
	delete m_control_listen;
	delete m_comm_data;
}
		

