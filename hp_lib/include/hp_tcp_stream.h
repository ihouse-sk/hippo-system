#ifndef HP_TCP_STREAM_H
#define HP_TCP_STREAM_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;

class hp_tcp_stream
{
    int     m_sd;
    string  m_peerIP;
    int     m_peerPort;
    unsigned int m_buffer_size;

  public:
    friend class hp_tcp_acceptor;
    friend class hp_tcp_connector;

    ~hp_tcp_stream();

    ssize_t send(const char* buffer, size_t len);
    ssize_t receive(char* buffer, size_t len, int timeout=0);

    string getPeerIP();
    int    getPeerPort();

    enum {
        connectionClosed = 0,
        connectionReset = -1,
        connectionTimedOut = -2
    };

  private:
    bool waitForReadEvent(int timeout);
    
    hp_tcp_stream(int sd, struct sockaddr_in* address);
    hp_tcp_stream();
    hp_tcp_stream(const hp_tcp_stream& stream);
};

#endif
