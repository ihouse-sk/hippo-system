#ifndef HP_TCP_ACCEPTOR_H
#define HP_TCP_ACCEPTOR_H

#include <string>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <memory>
#include "hp_tcp_stream.h"

using namespace std;

class hp_tcp_acceptor
{
    int    m_lsd;
    int    m_port;
    string m_address;
    bool   m_listening;
    
  public:
    hp_tcp_acceptor(int port, const char* address="");
    ~hp_tcp_acceptor();

    int        start();
    hp_tcp_stream* accept();
   // shared_ptr<hp_tcp_stream> accept();

  private:
    hp_tcp_acceptor() {}
};

#endif
