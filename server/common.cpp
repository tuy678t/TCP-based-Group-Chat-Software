#include "common.h"
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <unistd.h>

int read_from_fd(const int fd, uint8_t* data, const int len)
{
    int cnt = 0;
    while (cnt < len) {
        long res_read;
        do {
            res_read = read(fd, data, len - cnt);
        }
        while (res_read == -1 && errno == EINTR);

        if (res_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            std::perror("read_from_fd/read");
            return -1;
        }
        if (res_read == 0) {
            break;
        }
        data += res_read;
        cnt += res_read;
    }

    return cnt;
}

int write_to_fd(const int fd, const uint8_t* data, const int len)
{
    int cnt = 0;
    while (cnt < len) {
        long res_write;
        do {
            res_write = write(fd, data, len - cnt);
        }
        while (res_write == -1 && errno == EINTR);

        if (res_write == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            std::perror("write_to_fd/write");
            return -1;
        }
        if (res_write == 0) {
            break;
        }
        data += res_write;
        cnt += res_write;
    }

    return cnt;
}
