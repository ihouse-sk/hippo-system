#include <iostream>
#include <vector>
#include <string>

#define AES_CHAN_DEFAULT	0
#define AES_CHAN_NOW		1
#define AES_CHAN_OK 		2
#define AES_CHAN_ERROR		3

class xbee_dev{
	public:
		xbee_dev(std::string, int);
		std::string get_mac();
		unsigned char *get_hex_mac;
		int get_ident();
		void set_key_status(int status){ m_key_changing = status;}
		int get_key_status() { return m_key_changing;}
	private:
		std::string m_mac;
		int m_ident;
		int m_last_aes;
		int m_key_changing;
};
