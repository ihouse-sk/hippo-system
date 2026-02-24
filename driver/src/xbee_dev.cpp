#include "../include/xbee_dev.h"

xbee_dev::xbee_dev(std::string mac, int ident)
{
	m_mac = mac;
	m_ident = ident;
	m_key_changing = AES_CHAN_DEFAULT;
}

std::string xbee_dev::get_mac()
{
	return m_mac;
}
int xbee_dev::get_ident()
{
	return m_ident;
}
