#include "Log.h"

void make_log(){
    openlog("server_log", LOG_CONS | LOG_PID, LOG_LOCAL0);
}

void log_no_preson(string username){
    syslog(LOG_ERR, "%s is not a user, but try to log in\n",username.data());
}

void log_err_passwd(string username,string passwd){
    syslog(LOG_ERR, "%s try to log in with error password\n",username.data());
}

void log_err(string username,string passwd){
    syslog(LOG_ERR, "%s try to log error with password\n",username.data());
}

void log_log_in(string username,string passwd){
    syslog(LOG_INFO, "%s log in with password\n",username.data());
}

void log_wifi_disconn(int uid){
    syslog(LOG_ERR, "user-%d's wifi disconnected\n",uid);
}

void log_wifi_disconn(string username){
    syslog(LOG_ERR, "%s's wifi disconnected\n",username.data());
}

void log_log_out(int uid){
    syslog(LOG_INFO, "user-%d log out\n",uid);
}

void log_log_out(string username){
    syslog(LOG_INFO, "%s log out\n",username.data());
}

void log_log_ag(int uid){
    syslog(LOG_INFO, "user-%d log again\n",uid);
}

void log_log_ag(string username){
    syslog(LOG_INFO, "%s log again\n",username.data());
}

void log_msg(int sendid, int recvid, string message){
    syslog(LOG_INFO, "user-%d send to user-%d message : %s\n",sendid,recvid,message.data());
}

void log_msg(string sendname, string recvname, string message){
    syslog(LOG_INFO, "%s send to %s message : %s\n",sendname.data(),recvname.data(),message.data());
}

void log_file(int sendid, int recvid, string filename, int filesize){
    syslog(LOG_INFO, "user-%d send to user-%d file : %s %d bytes\n",sendid,recvid,filename.data(),filesize);
}

void log_file(string sendname, string recvname, string filename, int filesize){
    syslog(LOG_INFO, "%s send to %s file : %s %d bytes\n",sendname.data(),recvname.data(),filename.data(),filesize);
}
