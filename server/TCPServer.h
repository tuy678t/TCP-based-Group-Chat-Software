#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "Connections.h"
#include <cstdint>
#include <sys/socket.h>
#include <sys/select.h>

class TCPServer
{
public:
    // max number of queued requests
    static constexpr int backlog = SOMAXCONN;

    TCPServer(const char* ip, uint16_t port);
    ~TCPServer();

    void loop();

private:
    // listening file descriptor of this server
    const int _listen_fd;
    // max file descriptor
    int _max_fd;
    // number of opened files
    int _num_fd;
    // set of sockets of clients
    fd_set _client_fds;
    // connections
    Connections _conns;

    // accept from a client
    int accept_sock();
    // read from a client
    bool read_sock(int fd);
    // write to a client
    bool write_sock(int fd);
    // close a client
    void close_sock(int fd);
};

#endif // TCPSERVER_H
