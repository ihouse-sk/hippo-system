#ifndef HP_TCP_CONNECTOR_H
#define HP_TCP_COONECTOR_H

#include <netinet/in.h>
#include "hp_tcp_stream.h"

class hp_tcp_connector
{
  public:
    hp_tcp_stream* connect(const char* server, int port);
    hp_tcp_stream* connect(const char* server, int port, int timeout);
    
  private:
    int resolveHostName(const char* host, struct in_addr* addr);
};

#endif
