#include "TCPServer.h"
#include "Connections.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

TCPServer::TCPServer(const char* ip, const uint16_t port)
    : _listen_fd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
      _conns(FD_SETSIZE)
{
    // create a socket
    if (_listen_fd == -1) {
        std::perror("socket");
        std::exit(EXIT_FAILURE);
    }

    std::signal(SIGPIPE, SIG_IGN);

    // allow reuse of address
    const auto reuse = 1;
    auto res = setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (res == -1) {
        std::perror("setsockopt");
        std::exit(EXIT_FAILURE);
    }

    // prepare socket address
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(ip, &addr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP address.\n");
        std::exit(EXIT_FAILURE);
    }

    // name the socket
    res = bind(_listen_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    if (res == -1) {
        std::perror("bind");
        std::exit(EXIT_FAILURE);
    }

    // create a socket queue
    res = listen(_listen_fd, backlog);
    if (res == -1) {
        std::perror("listen");
        std::exit(EXIT_FAILURE);
    }

    // others
    _max_fd = _listen_fd;
    _num_fd = _max_fd + 1;
    FD_ZERO(&_client_fds);
}

TCPServer::~TCPServer()
{
    close(_listen_fd);
    for (auto i = _listen_fd + 1; i <= _max_fd; ++i) {
        if (FD_ISSET(i, &_client_fds)) {
            close(i);
        }
    }
}

void TCPServer::loop()
{
    while (true) {
        auto rfds = _client_fds, wfds = _conns.get_wfds();
        if (_num_fd < FD_SETSIZE) {
            // allow to accept
            FD_SET(_listen_fd, &rfds);
        }

        // select
        int res_sel;
        do {
            res_sel = select(_max_fd + 1, &rfds, &wfds, nullptr, nullptr);
        }
        while (res_sel == -1 && errno == EINTR);
        if (res_sel == -1) {
            std::perror("loop/select");
            break;
        }

        // ready to read or write
        for (auto i = _listen_fd; i <= _max_fd; ++i) {
            // ready to read
            if (FD_ISSET(i, &rfds)) {
                if (!this->read_sock(i)) {
                    continue;
                }
            }
            // ready to write
            if (FD_ISSET(i, &wfds)) {
                this->write_sock(i);
            }
        }
    }
}

int TCPServer::accept_sock()
{
    sockaddr_in client_addr = {};
    socklen_t client_addr_len = sizeof(client_addr);

    // accept
    int client_fd;
    do {
        client_fd = accept4(_listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len,
                            SOCK_NONBLOCK);
    }
    while (client_fd == -1 && errno == EINTR);
    if (client_fd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::perror("accept_sock/accept");
        return -1;
    }

    // print client's info
    const auto client_ip = inet_ntoa(client_addr.sin_addr);
    const auto client_port = ntohs(client_addr.sin_port);
    printf("Accepted from %s:%hu (%d)\n", client_ip, client_port, client_fd);

    return client_fd;
}

bool TCPServer::read_sock(const int fd)
{
    if (fd == _listen_fd) {
        // accept
        while (_num_fd < FD_SETSIZE) {
            const auto cfd = this->accept_sock();
            if (cfd != -1) {
                _max_fd = std::max(_max_fd, cfd);
                FD_SET(cfd, &_client_fds);
                _num_fd++;
            }
            else {
                break;
            }
        }
    }
    else {
        // work
        const auto res = _conns.recv_msg(fd);
        if (!res) {
            this->close_sock(fd);
            return false;
        }
    }
    return true;
}

bool TCPServer::write_sock(const int fd)
{
    const auto res = _conns.send_msg(fd);
    if (!res) {
        this->close_sock(fd);
        return false;
    }
    return true;
}

void TCPServer::close_sock(const int fd)
{
    // shutdown(fd, SHUT_RDWR);
    close(fd);
    FD_CLR(fd, &_client_fds);
    _num_fd--;

    _conns.close_conn(fd);
}
