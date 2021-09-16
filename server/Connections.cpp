#include "Connections.h"
#include "common.h"
#include "Log.h"
#include <cassert>
#include <cstring>
#include <netinet/in.h>

Connections::Connections(const size_t size)
    : _conns(size)
{
    assert(size <= FD_SETSIZE);

    FD_ZERO(&_wfds);
    if (_db.get_connect() == -1) {
        std::exit(EXIT_FAILURE);
    }
}

Connections::~Connections()
{
    _db.dis_connect();
}

void Connections::close_conn(const fd_t fd)
{
    assert(fd >= 0 && (unsigned int)fd < _conns.size());

    if (_fd_to_uid.count(fd)) {
        const auto uid = _fd_to_uid[fd];
        assert(_uid_to_fd.at(uid) == fd);

        log_log_out(uid);

        _fd_to_uid.erase(fd);
        _uid_to_fd.erase(uid);
        this->push_online(uid, false);
    }

    _conns[fd].recving_header = true;
    _conns[fd].recv_len = 0;
    _conns[fd].sending_header = true;
    _conns[fd].send_len = 0;
    _conns[fd].send_que = {};

    FD_CLR(fd, &_wfds);
}

bool Connections::recv_msg(const fd_t fd)
{
    assert(fd >= 0 && (unsigned int)fd < _conns.size());

    if (_conns[fd].recving_header)
        return this->recv_header(fd);
    else
        return this->recv_data(fd);
}

bool Connections::send_msg(const fd_t fd)
{
    assert(fd >= 0 && (unsigned int)fd < _conns.size());
    assert(!_conns[fd].send_que.empty());
    assert(FD_ISSET(fd, &_wfds));

    bool res;
    if (_conns[fd].sending_header)
        res = this->send_header(fd);
    else
        res = this->send_data(fd);

    if (!res) {
        std::printf("Force to close %d\n", fd);
    }
    return res;
}

fd_set Connections::get_wfds() const
{
    return _wfds;
}

bool Connections::recv_header(const fd_t fd)
{
    auto& conn = _conns[fd];
    auto& header = conn.recv_msg.header;
    const int expect_len = sizeof(header);

    const auto res = read_from_fd(fd, reinterpret_cast<uint8_t *>(&header) + conn.recv_len,
                                  expect_len - conn.recv_len);
    if (res <= 0)
        return false;

    conn.recv_len += res;
    if (conn.recv_len < expect_len)
        return true;

    conn.recving_header = false;
    conn.recv_len = 0;

    header.length = ntohs(header.length);
    std::printf("Received [%0hhX-%0hhX-%hu] from %u\n", header.type, header.sub_type, header.length, fd);

    return this->recv_data(fd);
}

bool Connections::recv_data(const fd_t fd)
{
    auto& conn = _conns[fd];
    auto& data = conn.recv_msg.data;
    const int expect_len = conn.recv_msg.header.length - sizeof(Header);

    // TODO: limit on length
    if (expect_len < 0) {
        this->parse_unknown(fd);
        return true;
    }

    if (expect_len > 0) {
        data.resize(expect_len);
        const auto res = read_from_fd(fd, data.data() + conn.recv_len,
                                      expect_len - conn.recv_len);
        if (res <= 0)
            return false;

        conn.recv_len += res;
        if (conn.recv_len < expect_len)
            return true;
    }

    conn.recving_header = true;
    conn.recv_len = 0;

    this->parse(fd);

    return true;
}

bool Connections::send_header(const fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& msg = conn.send_que.front();
    const auto& header = msg.header;
    const int expect_len = sizeof(header);

    const auto res = write_to_fd(fd, reinterpret_cast<const uint8_t *>(&header) + conn.send_len,
                                 expect_len - conn.send_len);
    if (res <= 0)
        return false;

    conn.send_len += res;
    if (conn.send_len < expect_len)
        return true;

    conn.sending_header = false;
    conn.send_len = 0;

    std::printf("Sent [%0hhX-%0hhX-%hu] to %u\n", msg.header.type, msg.header.sub_type, ntohs(msg.header.length), fd);

    if (header.type == 0xA0 || header.sub_type == 0xFF)
        return false;
    else if (header.type == 0x90 && (header.sub_type & 0xF0))
        return false;
    else
        return this->send_data(fd);
}

bool Connections::send_data(const fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& msg = conn.send_que.front();
    const auto& data = msg.data;
    const int expect_len = data.size();

    assert(sizeof(Header) + expect_len == ntohs(msg.header.length));

    if (expect_len > 0) {
        const auto res = write_to_fd(fd, data.data() + conn.send_len, expect_len - conn.send_len);
        if (res <= 0)
            return false;

        conn.send_len += res;
        if (conn.send_len < expect_len)
            return true;
    }

    conn.sending_header = true;
    conn.send_len = 0;

    conn.send_que.pop();
    if (conn.send_que.empty()) {
        FD_CLR(fd, &_wfds);
    }

    return true;
}

void Connections::parse(const fd_t fd)
{
    const auto& header = _conns[fd].recv_msg.header;

    switch (header.type) {
    case 0x10:
        this->parse_login(fd);
        break;
    case 0x11:
        this->parse_username(fd);
        break;
    case 0x12:
        this->parse_chatrecord(fd);
        break;
    case 0x13:
        this->parse_textmsg(fd);
        break;
    case 0x15:
        this->parse_change_pwd(fd);
        break;
    case 0x17: //发送方请求发送报文
        this->parse_transfile_ask_rc(fd);
        break;
    case 0x18: //接受方同意接受报文
        this->parse_transfile_rep_sc(fd);
        break;
    case 0x19: //发送方发送数据报文
        this->parse_transfile_data(fd);
        break;
    case 0x1A: //接受方确认接受报文
        this->parse_transfile_ack(fd);
        break;
    default:
        this->parse_unknown(fd);
        break;
    }
}

void Connections::parse_login(const fd_t fd)
{
    struct Data
    {
        char username[28];
        char password[28];
    };
    struct RespondData
    {
        uid_t uid;
        uint32_t primary_color;
    };

    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.sub_type != 0x00 || header.length != sizeof(Header) + sizeof(Data)) {
        this->parse_unknown(fd);
        return;
    }

    const auto& data = conn.recv_msg.data;
    assert(data.size() == sizeof(Data));

    // check username and password
    const auto p_data = reinterpret_cast<const Data *>(data.data());
    const auto username = p_data->username[27] ? std::string(p_data->username, 28) : std::string(p_data->username);
    const auto password = p_data->password[27] ? std::string(p_data->password, 28) : std::string(p_data->password);
    auto legal = false;
    auto first = true;
    uid_t uid = 0;
    if (_db.login_check(std::move(username), std::move(password), legal, first, uid) < 0) {
        this->parse_unknown(fd);
        return;
    }
    const auto primary_color = _db.get_login_color(uid);

    // build respond
    Message msg;
    msg.header.type = 0x90;
    msg.header.sub_type = 0x00;
    if (!legal) {
        msg.header.sub_type = 0xF0;
        msg.header.length = htons(sizeof(Header));
    }
    else {
        msg.header.length = htons(sizeof(Header) + sizeof(RespondData));
        msg.data.resize(sizeof(RespondData));
        const auto p_resdata = reinterpret_cast<RespondData *>(msg.data.data());
        p_resdata->uid = htonl(uid);
        p_resdata->primary_color = htonl(primary_color);

        if (first) {
            msg.header.sub_type = 0x01;
        }
        else if (_uid_to_fd.count(uid)) {
            const auto other_fd = _uid_to_fd[uid];
            assert(_fd_to_uid.at(other_fd) == uid);

            msg.header.sub_type = 0x02;
            this->push_shutdown(other_fd);
            log_log_ag(uid);
            _fd_to_uid.erase(other_fd);
            _uid_to_fd.erase(uid);
        }
    }

    // send respond
    conn.send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);

    assert(!_fd_to_uid.count(fd));

    // update online list
    if (legal) {
        this->push_online(uid, true);
        _uid_to_fd[uid] = fd;
        _fd_to_uid[fd] = uid;
        this->push_userlist(fd);
    }
}

void Connections::parse_username(const fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.sub_type != 0x00 || header.length != sizeof(Header) + sizeof(uid_t)) {
        this->parse_unknown(fd);
        return;
    }

    const auto& data = conn.recv_msg.data;
    assert(data.size() == sizeof(uid_t));

    const auto p_data = reinterpret_cast<const uid_t *>(data.data());
    const uid_t uid = ntohl(*p_data);
    printf("%s uid: %d\n", __func__, uid);


    // check uid, get username
    auto exist = false;
    std::string username;
    if (_db.check_uid(uid, exist, username) < 0) {
        this->parse_unknown(fd);
        return;
    }
    const auto len_username = username.length();

    // build respond
    Message msg;
    msg.header.type = 0x91;
    if (!exist) {
        msg.header.sub_type = 0xF0;
        msg.header.length = htons(sizeof(Header));
    }
    else {
        msg.header.sub_type = 0x00;
        msg.header.length = htons(sizeof(Header) + len_username);
        msg.data.resize(len_username);
        memcpy(msg.data.data(), username.data(), len_username);
    }

    // send respond
    conn.send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);
}

void Connections::parse_chatrecord(const fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;

    const auto is_private = header.sub_type == 0x00;
    const auto is_public = header.sub_type == 0x01;
    if ((!is_private && !is_public) || header.length != sizeof(Header) + sizeof(uid_t)) {
        this->parse_unknown(fd);
        return;
    }

    const auto& data = conn.recv_msg.data;
    assert(data.size() == sizeof(uid_t));

    const auto p_data = reinterpret_cast<const uid_t *>(data.data());
    const auto uid = ntohl(*p_data);

    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];

    // check uid
    auto exist = true;
    if (is_private) {
        std::string username;
        if (_db.check_uid(uid, exist, username) < 0) {
            this->parse_unknown(fd);
            return;
        }
    }

    // get records
    std::vector<uint8_t> records;
    if (exist) {
        if (_db.serh_message(my_uid, uid, records) < 0) {
            this->parse_unknown(fd);
            return;
        }
    }

    // build respond
    Message msg;
    msg.header.type = 0x92;
    if (!exist) {
        msg.header.sub_type = 0xF0;
        msg.header.length = htons(sizeof(Header));
    }
    else {
        msg.header.sub_type = 0x00;
        msg.header.length = htons(sizeof(Header) + records.size());
        msg.data = std::move(records);
    }

    // send respond
    conn.send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);
}

void Connections::parse_textmsg(const fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;

    const auto is_private = header.sub_type == 0x00;
    const auto is_public = header.sub_type == 0x01;
    if ((!is_private && !is_public) || header.length > sizeof(Header) + sizeof(uid_t) + 1024) {
        this->parse_unknown(fd);
        return;
    }

    const auto& data = conn.recv_msg.data;
    assert(sizeof(Header) + data.size() == header.length);

    const auto dst_uid = ntohl(*reinterpret_cast<const uid_t *>(data.data()));

    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];

    // check uid
    auto exist = false;
    if (is_private) {
        std::string username;
        if (_db.check_uid(dst_uid, exist, username) < 0) {
            this->parse_unknown(fd);
            return;
        }
    }
    else {
        exist = dst_uid == 0xFFFF;
    }
    const auto online = is_private ? _uid_to_fd.count(dst_uid) : true;

    // save records
    const auto p_txt = reinterpret_cast<const char*>(data.data()) + sizeof(uid_t);
    const auto len_txt = data.size() - sizeof(uid_t);
    const std::string txt(p_txt, len_txt);
    if (_db.save_message(my_uid, dst_uid, std::move(txt)) < 0) {
        this->parse_unknown(fd);
        return;
    }

    // build respond
    Message msg;
    msg.header.type = 0x93;
    msg.header.length = htons(sizeof(Header));
    if (!exist) {
        msg.header.sub_type = 0xF0;
    }
    else if (!online) {
        msg.header.sub_type = 0xF1;
    }
    else {
        msg.header.sub_type = 0x00;
    }

    // send respond
    conn.send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);

    // forward txt message
    if (online)
        this->push_textmsg(my_uid, dst_uid, conn.recv_msg);
}

void Connections::parse_change_pwd(fd_t fd)
{
    struct Data
    {
        char old_pwd[28];
        char new_pwd[28];
    };

    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.sub_type != 0x00 || header.length != sizeof(Header) + sizeof(Data)) {
        this->parse_unknown(fd);
        return;
    }

    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];

    const auto& data = conn.recv_msg.data;
    assert(data.size() == sizeof(Data));

    const auto p_data = reinterpret_cast<const Data *>(data.data());
    const auto old_pwd = p_data->old_pwd[27] ? std::string(p_data->old_pwd, 28) : std::string(p_data->old_pwd);
    const auto new_pwd = p_data->new_pwd[27] ? std::string(p_data->new_pwd, 28) : std::string(p_data->new_pwd);

    // change passwords
    const bool succeed = _db.update_passwd(my_uid, new_pwd, old_pwd) == 0;

    // build respond
    Message msg;
    msg.header.type = 0x95;
    msg.header.sub_type = succeed ? 0x00 : 0xF0;
    msg.header.length = htons(sizeof(Header));

    // send respond
    conn.send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);
}

void Connections::parse_unknown(const fd_t fd)
{
    // build respond
    Message msg;
    msg.header.type = _conns[fd].recv_msg.header.type | 0x80;
    msg.header.sub_type = 0xFF;
    msg.header.length = htons(sizeof(Header));

    // send respond
    _conns[fd].send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);
}

void Connections::push_shutdown(const fd_t fd)
{
    // build message
    Message msg;
    msg.header.type = 0xA0;
    msg.header.sub_type = 0x00;
    msg.header.length = htons(sizeof(Header));

    // send message
    _conns[fd].send_que.push(std::move(msg));
    FD_SET(fd, &_wfds);
}

void Connections::push_userlist(const fd_t fd)
{
    // get user list
    std::vector<uid_t> user_list;
    _db.get_all_uid(user_list);
    const auto len_online = _uid_to_fd.size() - 1;
    const auto len_offline = user_list.size() - _uid_to_fd.size();

    assert(_fd_to_uid.count(fd));
    const auto my_uid = _fd_to_uid[fd];

    // build message header
    Message msg_online, msg_offline;
    msg_online.header.type = 0xA1;
    msg_offline.header.type = 0xA1;
    msg_online.header.sub_type = 0x00;
    msg_offline.header.sub_type = 0x01;
    msg_online.header.length = htons(sizeof(Header) + sizeof(uid_t) * len_online);
    msg_offline.header.length = htons(sizeof(Header) + sizeof(uid_t) * len_offline);

    // build message data
    msg_online.data.resize(sizeof(uid_t) * len_online);
    msg_offline.data.resize(sizeof(uid_t) * len_offline);
    auto p_online = reinterpret_cast<uid_t *>(msg_online.data.data());
    auto p_offline = reinterpret_cast<uid_t *>(msg_offline.data.data());
    for (auto uid : user_list) {
        if (uid == my_uid)
            continue;
        if (_uid_to_fd.count(uid)) {
            *p_online++ = htonl(uid);
        }
        else {
            *p_offline++ = htonl(uid);
        }
    }

    // send message
    _conns[fd].send_que.push(std::move(msg_online));
    _conns[fd].send_que.push(std::move(msg_offline));
    FD_SET(fd, &_wfds);
}

void Connections::push_online(const uid_t uid, const bool online)
{
    // build message
    Message msg;
    msg.header.type = 0xA1;
    msg.header.sub_type = online ? 0x00 : 0x01;
    msg.header.length = htons(sizeof(Header) + sizeof(uid_t));

    msg.data.resize(sizeof(uid_t));
    auto p_data = reinterpret_cast<uid_t *>(msg.data.data());
    *p_data = htonl(uid);

    // send message
    for (const auto& p : _fd_to_uid) {
        assert(p.second != uid);

        _conns[p.first].send_que.push(msg);
        FD_SET(p.first, &_wfds);
    }
}

void Connections::push_textmsg(const uid_t src_uid, const uid_t dst_uid, Message msg)
{
    msg.header.length = htons(msg.header.length);
    msg.header.type = 0xA3;
    *reinterpret_cast<uid_t *>(msg.data.data()) = htonl(src_uid);
    if (dst_uid == 0xFFFF) {
        for (const auto& p : _fd_to_uid) {
            if (p.second == src_uid) {
                continue;
            }

            _conns[p.first].send_que.push(msg);
            FD_SET(p.first, &_wfds);
        }
    }
    else {
        assert(_uid_to_fd.count(dst_uid));
        const auto dst_fd = _uid_to_fd[dst_uid];
        _conns[dst_fd].send_que.push(std::move(msg));
        FD_SET(dst_fd, &_wfds);
    }
}

/**
 * 2018.12.21
 * ADD
 */
void Connections::parse_transfile_ask_rc(fd_t fd)
{
    struct Qust
    {
        uint32_t recvid;
        uint32_t sendid;
        uint32_t filesize;
        char filename[64];
    };
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.length != sizeof(Header) + sizeof(Qust)) {
        this->parse_unknown(fd);
        return;
    }
    const auto& data = conn.recv_msg.data;
    assert(data.size() == sizeof(Qust));

    const auto q_data = reinterpret_cast<const Qust *>(data.data());
    const auto dst_uid = ntohl(q_data->recvid);
    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];

    const auto file_name_str = q_data->filename[63] ? std::string(q_data->filename, 64) : std::string(q_data->filename);
    _db.insert_file_record(my_uid, dst_uid, ntohl(q_data->filesize), file_name_str);
    push_transfile_ask_rc(my_uid, dst_uid, conn.recv_msg);
}

void Connections::parse_transfile_rep_sc(fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.length != sizeof(Header) + sizeof(uint32_t)+sizeof(uint32_t)) {
        this->parse_unknown(fd);
        return;
    }
    const auto& data = conn.recv_msg.data;
    assert(data.size() == 2*sizeof(uint32_t));
    const auto dst_uid = ntohl(*reinterpret_cast<const uid_t *>(data.data()));
    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];
    if (header.sub_type == 0x01) {
        _db.permit_file_record(dst_uid, my_uid);
    }
    else {
        _db.delete_file_record(dst_uid, my_uid);
    }
    push_transfile_rep_sc(my_uid, dst_uid, conn.recv_msg);
}

void Connections::parse_transfile_data(fd_t fd)
{
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.length != sizeof(Header) + conn.recv_msg.data.size()) {
        this->parse_unknown(fd);
        return;
    }
    const auto& data = conn.recv_msg.data;
    const auto dst_uid = ntohl(*reinterpret_cast<const uid_t *>(data.data()));
    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];
    push_transfile_data(my_uid, dst_uid, conn.recv_msg);
}

void Connections::parse_transfile_ack(fd_t fd)
{
    struct Ackg
    {
        uint32_t recvid;
        uint32_t sendid;
        uint32_t ui8cnt;
    };
    auto& conn = _conns[fd];
    const auto& header = conn.recv_msg.header;
    if (header.length != sizeof(Header) + sizeof(Ackg)) {
        this->parse_unknown(fd);
        return;
    }
    const auto& data = conn.recv_msg.data;
    assert(data.size() == sizeof(Ackg));
    const auto dst_uid = ntohl(*reinterpret_cast<const uid_t *>(data.data()));
    assert(_uid_to_fd.at(_fd_to_uid.at(fd)) == fd);
    const auto my_uid = _fd_to_uid[fd];
    const auto a_data = reinterpret_cast<const Ackg *>(data.data());
    _db.update_file_record(dst_uid, my_uid, ntohl(a_data->ui8cnt));
    push_transfile_data(my_uid, dst_uid, conn.recv_msg);
}

void Connections::push_transfile_ask_rc(const uid_t src_uid, const uid_t dst_uid, Message msg)
{
    msg.header.type = 0xA7;
    assert(_uid_to_fd.count(dst_uid));
    const auto dst_fd = _uid_to_fd[dst_uid];
    msg.header.length = htons(msg.header.length);
    _conns[dst_fd].send_que.push(std::move(msg));
    FD_SET(dst_fd, &_wfds);
}

void Connections::push_transfile_rep_sc(const uid_t src_uid, const uid_t dst_uid, Message msg)
{
    msg.header.type = 0xA8;
    assert(_uid_to_fd.count(dst_uid));
    const auto dst_fd = _uid_to_fd[dst_uid];
    msg.header.length = htons(msg.header.length);
    _conns[dst_fd].send_que.push(std::move(msg));
    FD_SET(dst_fd, &_wfds);
}

void Connections::push_transfile_data(const uid_t src_uid, const uid_t dst_uid, Message msg)
{
    msg.header.type = 0xA9;
    assert(_uid_to_fd.count(dst_uid));
    const auto dst_fd = _uid_to_fd[dst_uid];
    msg.header.length = htons(msg.header.length);
    _conns[dst_fd].send_que.push(std::move(msg));
    FD_SET(dst_fd, &_wfds);
}

void Connections::push_transfile_ack(const uid_t src_uid, const uid_t dst_uid, Message msg)
{
    msg.header.type = 0xAA;
    assert(_uid_to_fd.count(dst_uid));
    const auto dst_fd = _uid_to_fd[dst_uid];
    msg.header.length = htons(msg.header.length);
    _conns[dst_fd].send_que.push(std::move(msg));
    FD_SET(dst_fd, &_wfds);
}
