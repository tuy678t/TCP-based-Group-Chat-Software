#ifndef LOG_H
#define LOG_H
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/un.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string>
using namespace std;

void make_log();

void log_no_preson(string username);

void log_err_passwd(string username, string passwd);

void log_err(string username, string passwd);

void log_log_in(string username, string passwd);

void log_wifi_disconn(int uid);

void log_wifi_disconn(string username);

void log_log_out(int uid);

void log_log_out(string username);

void log_log_ag(int uid);

void log_log_ag(string username);

void log_msg(string sendname, string recvname, string message);

void log_file(string sendname, string recvname, string filename, int filesize);

void log_msg(int sendid, int recvid, string message);

void log_file(int sendid, int recvid, string filename, int filesize);

#endif