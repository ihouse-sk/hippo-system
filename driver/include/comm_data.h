#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <boost/thread.hpp>
		
#ifndef XBEE_DEV_C
#define XBEE_DEV_C
#include "xbee_dev.h"
#endif

#define CHECK() fprintf(stderr, "%s:%d:check\n", __FILE__, __LINE__)

class comm_data{
	public:
		comm_data(int, int,bool);
		int get_xbee_size() { return m_xbee_dev.size(); }
		int get_xbee_ident(int position){
			if(position < (int)m_xbee_dev.size()){
				return m_xbee_dev[position].get_ident();
			} else {
				return -1;
			}
		}
		std::string get_xbee_mac(int position){
			if(position < (int)m_xbee_dev.size()){
				return m_xbee_dev[position].get_mac();
			} else {
				return "";
			}
		}
		inline int get_xbee_status(int position){
			if(position < (int) m_xbee_dev.size()){
				return m_xbee_dev[position].get_key_status();
			} else {
				return -1;
			}
		}
		inline void set_x_status(int status) { 
			mtx.lock();
			m_status_xthread = status; 
			mtx.unlock();
		} 
		int get_x_status() { return m_status_xthread;}
		int get_source_install() { return m_source_install;}
		inline void set_c_status(int status) { 
			mtx.lock();
			m_status_cthread = status;  
			mtx.unlock();
		}
		int get_c_status() { return m_status_cthread;}
		unsigned int get_send_time() { return m_last_send_time;}
		void set_send_time(unsigned int itime) { m_last_send_time = itime;}
		void set_recieve_time(unsigned int itime) { m_last_recieve_time = itime;}
		unsigned int get_recieve_time() { return m_last_recieve_time;}
		void set_reset_xbee() { m_reset_xbee = !m_reset_xbee;}
		bool get_reset_xbee() { return m_reset_xbee;}
		void add_xbee_dev(xbee_dev);
		void print_xbee_dev();
		void add_aes_mess(unsigned char *);
		void set_aes_changing(int xbee_number = 0);
		bool get_aes_changing_status(){return m_changing_aes;}
		void set_aes_changing_status(bool status){ m_changing_aes = status;}
		bool get_add_new() {return m_add_new;}
		void set_add_new(bool add_new) {m_add_new = add_new;}
		int get_aes_actual() {return m_actual_aes_key_num;}
		void set_aes_actual(int aes_number) {m_actual_aes_key_num = aes_number;}
	private:
		bool m_source_install;
		int m_status_cthread;
		int m_status_xthread;
		int m_actual_aes_key_num;
		unsigned int m_last_send_time;
		unsigned int m_last_recieve_time;
		bool m_reset_xbee;
		bool m_changing_aes;
		bool m_add_new;
		boost::mutex mtx;
		boost::mutex mtx_aes;
		std::vector<xbee_dev> m_xbee_dev;
		std::vector<std::string> m_aes_mess;

};
