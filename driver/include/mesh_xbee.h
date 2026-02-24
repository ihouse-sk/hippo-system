#include <iostream>
#include "xbee_listen.h"
#include "control_listen.h"
#include <boost/thread.hpp>
#include "xmlParser.h"
#include <sstream>
#include <mysql/mysql.h>
#include <signal.h>

#define AES_CHECKING_TIME 10

//#define TESTING

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
	stm.setf( std::ios::fixed, std:: ios::floatfield );
        stm << std::setprecision(2) << n ;
        return stm.str() ;
    }
}

class xbee {

	public:
		xbee();
		int init();
		int run();
	private:
		api_uart *m_api_uart;
		xbee_listen *m_xbee_listen;
		control_listen *m_control_listen;
		log_class *m_log_class;
		comm_data *m_comm_data;
		int m_status_cthread;
		int m_status_xthread;
		bool m_nd_option;
		bool m_aes_option;
		bool m_aes_init;
		std::string m_xbee_socket;
		std::string m_cls_socket;
		std::string m_mysql_server;
		std::string m_mysql_pass;
		int m_log_verbose;
		int m_key_time;	
		boost::thread th1,th2;
		
		void delete_objects();
		void send_ND_command();
		void reset_driver();
		void send_quit();
		int send_command(unsigned char *);
		int applied_first_key(unsigned char *);
		int apply_changes(unsigned char *);
		int turn_on_aes(unsigned char *);
		void read_xml_startup();
		int mysql_update(const char *);
		int first_aes_init();
		int add_xbee2db();
		unsigned char *get_key(MYSQL *,int key_id);
		int check_key_validity();
		int change_aes_key();
		unsigned char* generate_new_key();
		int add_new_xbee(std::string);
		int ee_koordinator(bool, unsigned char *key=NULL);
		inline bool exists(const std::string &name);

};
