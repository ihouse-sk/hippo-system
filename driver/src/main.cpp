#include <iostream>
#include "../include/mesh_xbee.h"


using namespace std;

int main()
{
	xbee *m_xbee = new xbee();

	if((m_xbee->init()) == -1){
		delete m_xbee;
		return 0;
	}
	m_xbee->run();

	delete m_xbee;

	return 0;
}


