#ifndef COMMON_H
#define COMMON_H
#include <cstdint>

// ��������fd��len�ֽ����ݣ�д��data�У����سɹ��ֽ�������-1����
int read_from_fd(int fd, uint8_t* data, int len);

// ����data�е�len�ֽ����ݵ�fd�У����سɹ��ֽ�������-1����
int write_to_fd(int fd, const uint8_t* data, int len);

#endif // COMMON_H
