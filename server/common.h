#ifndef COMMON_H
#define COMMON_H
#include <cstdint>

// 接收来自fd的len字节数据，写入data中，返回成功字节数，或-1错误
int read_from_fd(int fd, uint8_t* data, int len);

// 发送data中的len字节数据到fd中，返回成功字节数，或-1错误
int write_to_fd(int fd, const uint8_t* data, int len);

#endif // COMMON_H
