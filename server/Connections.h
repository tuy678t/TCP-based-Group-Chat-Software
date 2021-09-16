#ifndef CONNECTIONS_H
#define CONNECTIONS_H
#include "Database.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <sys/select.h>

#define BAGSIZ 4096

class Connections
{
  public:
    using fd_t = int;
    using uid_t = uint32_t;

    explicit Connections(size_t size);
    ~Connections();

    void close_conn(fd_t fd);
    bool recv_msg(fd_t fd);
    bool send_msg(fd_t fd);
    fd_set get_wfds() const;

  private:
    struct Header
    {
        uint8_t type = 0;
        uint8_t sub_type = 0;
        uint16_t length = 0;
    };

    using Buffer = std::vector<uint8_t>;

    struct Message
    {
        Header header;
        Buffer data;
    };

    struct Conn
    {
        bool recving_header = true;
        int recv_len = 0;
        Message recv_msg;

        bool sending_header = true;
        int send_len = 0;
        std::queue<Message> send_que;
    };

    Database _db;
    std::vector<Conn> _conns;
    fd_set _wfds;

    std::unordered_map<uid_t, fd_t> _uid_to_fd;
    std::unordered_map<fd_t, uid_t> _fd_to_uid;

    bool recv_header(fd_t fd);
    bool recv_data(fd_t fd);
    bool send_header(fd_t fd);
    bool send_data(fd_t fd);

    void parse(fd_t fd);
    void parse_login(fd_t fd);
    void parse_username(fd_t fd);
    void parse_chatrecord(fd_t fd);
    void parse_textmsg(fd_t fd);
    void parse_change_pwd(fd_t fd);
    void parse_unknown(fd_t fd);
    /**
     * 2018.12.21
     * ADD:
     */
    void parse_transfile_ask_rc(fd_t fd);
    void parse_transfile_rep_sc(fd_t fd);
    void parse_transfile_data(fd_t fd);
    void parse_transfile_ack(fd_t fd);

    void push_shutdown(fd_t fd);
    void push_userlist(fd_t fd);
    void push_online(uid_t uid, bool online);
    void push_textmsg(uid_t src_uid, uid_t dst_uid, Message msg);
    /**
     * 2018.12.21
     * ADD:
     */
    void push_transfile_ask_rc(uid_t src_uid, uid_t dst_uid, Message msg);
    void push_transfile_rep_sc(uid_t src_uid, uid_t dst_uid, Message msg);
    void push_transfile_data(uid_t src_uid, uid_t dst_uid, Message msg);
    void push_transfile_ack(uid_t src_uid, uid_t dst_uid, Message msg);
};

#endif // CONNECTIONS_H
