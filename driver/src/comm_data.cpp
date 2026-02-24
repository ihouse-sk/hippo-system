#include "../include/comm_data.h"

comm_data::comm_data(int status_c, int status_x,bool source)
{
	m_status_xthread = status_x;
	m_status_cthread = status_c;
	m_source_install = source;
	m_last_send_time = time(NULL);
	m_last_recieve_time = time(NULL);
	m_reset_xbee = false;
	m_changing_aes = false;
	m_add_new = false;
	m_actual_aes_key_num = -1;
}

void comm_data::add_xbee_dev(xbee_dev dev)
{
	m_xbee_dev.push_back(dev);
}

void comm_data::print_xbee_dev()
{
	for(std::vector<xbee_dev>::iterator it = m_xbee_dev.begin(); it != m_xbee_dev.end(); ++it){
		printf("xbee id: %d\t mac: %s\n",it->get_ident(), it->get_mac().c_str());
	}
}

void comm_data::add_aes_mess(unsigned char * mess)
{
	mtx_aes.lock();
	std::string str_mess,mac, status;
	str_mess = (char *)mess;
	if(str_mess.length() > 35){
		for(int i=0; i<10; i++){
			if(i != 9){
				mac.append(str_mess.substr(i*2+14,2)+" ");
			} else {
				mac.append(str_mess.substr(i*2+14,2));
			}
		}
		status = str_mess.substr(35);
		for(std::vector<xbee_dev>::iterator it = m_xbee_dev.begin(); it != m_xbee_dev.end(); ++it){
			if(mac.find(it->get_mac()) != std::string::npos){
				if(status == "ok"){
					it->set_key_status(AES_CHAN_OK);
				} else {
					it->set_key_status(AES_CHAN_ERROR);
				}
			}
		}
	} 
	mtx_aes.unlock();
}

void comm_data::set_aes_changing(int xbee_number)
{
	if(xbee_number != 0){
		if(xbee_number > 0 && xbee_number < (int)m_xbee_dev.size()){
			m_xbee_dev[xbee_number].set_key_status(AES_CHAN_NOW);
		}
	} else {
		for(std::vector<xbee_dev>::iterator it = m_xbee_dev.begin(); it != m_xbee_dev.end(); ++it){
			it->set_key_status(AES_CHAN_NOW);
		}
	}
}


