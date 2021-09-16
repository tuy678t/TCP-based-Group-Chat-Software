#include "Database.h"
#include "MD5.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <cstring>
#include "Log.h"
#include <netinet/in.h>
using namespace std;

int Database::get_connect()
{
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        return -1;
    }

    if (mysql_real_connect(mysql, HOST, USER, PASSWD, DB, PORT, UNIX_SOCKET, CLIENT_FLAG) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }

    if (mysql_set_character_set(mysql, "gbk"))
    {
        cout << "mysql_set_character_set failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }
    return 0;
}

int Database::dis_connect()
{
    mysql_close(mysql);
    return 0;
}

int Database::insert_login(string username, string passwd)
{
    string secu_passwd = MD5(passwd).hexdigest();
    string sqlstr = "insert into login(username,password,first) values('" + username + "','" + secu_passwd + "',1)";
    if (mysql_query(mysql, sqlstr.data()) == 0)
    {
        printf("insert success");
        return 0;
    }
    else
    {
        printf("insert failed");
        return -1;
    }
}

int Database::select_login_all()
{
    string sqlstr = "select * from login";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result)
    {
        int row_num, col_num;
        row_num = mysql_num_rows(result);
        col_num = mysql_num_fields(result);
        std::cout << "共有" << row_num << "条数据，以下为其详细内容：" << std::endl;
        while ((fd = mysql_fetch_field(result)))
        {
            std::cout << fd->name << "\t";
        }
        std::cout << std::endl;
        while ((sql_row = mysql_fetch_row(result)))
        {
            for (int i = 0; i < col_num; i++)
            {
                if (sql_row[i] == NULL)
                    std::cout << "NULL\t";
                else
                    std::cout << sql_row[i] << "\t";
            }
            std::cout << std::endl;
        }
    }
    if (result != NULL)
    {
        mysql_free_result(result);
    }
    return 0;
}

int Database::login_check(string username, string passwd, bool &exist, bool &legal, bool &first, uint32_t &uid)
{
    string sqlstr = "select * from login where username='" + username + "'";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        exist = true;
        string secu_passwd = MD5(passwd).hexdigest();
        string sqlstr = "select count(*) from login where username='" + username + "' and password='" + secu_passwd + "'";
        cout << sqlstr << endl;
        if (mysql_query(mysql, sqlstr.data()))
        {
            cout << mysql_error(mysql) << endl;
            return -1;
        }
        result = mysql_store_result(mysql);
        if (result && mysql_num_rows(result))
        {
            sql_row = mysql_fetch_row(result);
            string tmp = sql_row[0];
            int count = atoi(tmp.c_str());
            if (count == 1)
            {
                legal = true;
            }
            else
            {
                log_err_passwd(username,PASSWD);
                legal = false;
                return 2;
            }
        }
        sqlstr = "select uid,first from login where username='" + username + "' and password='" + secu_passwd + "'";
        if (mysql_query(mysql, sqlstr.data()))
        {
            cout << mysql_error(mysql) << endl;
            return -1;
        }
        result = mysql_store_result(mysql);
        if (result)
        {
            sql_row = mysql_fetch_row(result);
            string tmp = sql_row[0];
            uid = atoi(tmp.c_str());
            tmp = sql_row[1];
            first = atoi(tmp.c_str());
        }
        if (result != NULL)
        {
            mysql_free_result(result);
        }
        log_log_in(username,passwd);
        return 0;
    }
    else
    {
        log_no_preson(username);
        exist = false;
        legal = false;
        return 1;
    }
}

int Database::login_check(string username, string passwd, bool &legal, bool &first, uint32_t &uid)
{
    string secu_passwd = MD5(passwd).hexdigest();
    string sqlstr = "select count(*) from login where username='" + username + "' and password='" + secu_passwd + "'";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        sql_row = mysql_fetch_row(result);
        string tmp = sql_row[0];
        int count = atoi(tmp.c_str());
        if (count == 1)
        {
            legal = true;
        }
        else
        {
            legal = false;
            log_err(username,passwd);
            return 0;
        }
    }
    sqlstr = "select uid,first from login where username='" + username + "' and password='" + secu_passwd + "'";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result)
    {
        sql_row = mysql_fetch_row(result);
        string tmp = sql_row[0];
        uid = atoi(tmp.c_str());
        tmp = sql_row[1];
        first = atoi(tmp.c_str());
    }
    if (result != NULL)
    {
        mysql_free_result(result);
    }
    log_log_in(username,passwd);
    return 0;
}

int Database::check_uid(int uid, bool &exist, string &username)
{
    string sqlstr = "select username from login where uid='" + int2string(uid) + "'";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        exist = true;
        sql_row = mysql_fetch_row(result);
        username = sql_row[0];
    }
    else
    {
        exist = false;
    }
    if (result != NULL)
    {
        mysql_free_result(result);
    }
    return 0;
}

int Database::update_passwd(int uid, string newpasswd, string oldpasswd)
{
    string usrname;
    bool legal;
    bool tmp1;
    unsigned int tmp2;
    check_uid(uid, legal, usrname);
    login_check(usrname, oldpasswd, legal, tmp1, tmp2);
    if (legal && newpasswd != oldpasswd)
    {
        string secu_passwd = MD5(newpasswd).hexdigest();
        string sqlstr = "update login set password='" + secu_passwd + "',first=0 where uid='" + int2string(uid) + "'";
        if (mysql_query(mysql, sqlstr.data()))
        {
            cout << mysql_error(mysql) << endl;
            printf("insert failed");
            return -1;
        }
        return 0;
    }
    else
        return 1;
}

string Database::int2string(int integer)
{
    stringstream ss;
    ss << integer;
    return ss.str();
}

string Database::ll2string(long long lli)
{
    stringstream ss;
    ss << lli;
    return ss.str();
}

int Database::string2int(string str)
{
    return atoi(str.c_str());
}

long long Database::string2ll(string str)
{
    return atoll(str.c_str());
}

string Database::get_curtime(time_t cur)
{
    char time_str[20];
    tm *t = localtime(&cur);
    snprintf(time_str, 20, "%d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    return (string)time_str;
}

int Database::save_message(int sendid, int recvid, string text)
{
    string sqlstr = "insert into record(sendid,recvid,time,message) values('" + int2string(sendid) + "','" + int2string(recvid) + "','" + ll2string(time(NULL)) + "','" + text + "')";
    log_msg(sendid,recvid,text);
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("insert failed");
        return -1;
    }
    return 0;
}

int Database::serh_message(int myid, int srid, vector<uint8_t> &records)
{
    records.clear();
    int count = get_record_cnt(myid);
    string sqlstr;
    if(srid == 0xFFFF)
        sqlstr = "select * from record where recvid=0xFFFF order by rid asc";
    else
        sqlstr = "select * from record where sendid=" + int2string(myid) + " and recvid=" + int2string(srid) + " or sendid=" + int2string(srid) + " and recvid=" + int2string(myid) + " order by rid asc";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        int rownum = mysql_num_rows(result);
        int tcnt = 0;
        int psiz = 0;
        while ((sql_row = mysql_fetch_row(result)))
        {
            if (tcnt + count < rownum)
            {
                tcnt++;
                continue;
            }
            else
            {
                tcnt++;
                const auto len = strlen(sql_row[4]);
                records.resize(psiz + 12 + len + 1);
                *(uint32_t *)(records.data() + psiz) = htonl(string2int(sql_row[1]));
                *(uint64_t *)(records.data() + psiz + 4) = htobe64(string2ll(sql_row[3]));
                memcpy((char *)records.data() + psiz + 12, sql_row[4], len + 1);
                psiz = records.size();
            }
        }
    }
    return 0;
}

int Database::serh_message(int peo1, int peo2, vector<uint8_t> &records, int count)
{
    records.clear();
    string sqlstr = "select * from record where sendid=" + int2string(peo1) + " and recvid=" + int2string(peo2) + " or sendid=" + int2string(peo2) + " and recvid=" + int2string(peo1) + " order by rid asc";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        int rownum = mysql_num_rows(result);
        int tcnt = 0;
        int psiz = 0;
        while ((sql_row = mysql_fetch_row(result)))
        {
            if (tcnt + count < rownum)
            {
                tcnt++;
                continue;
            }
            else
            {
                tcnt++;
                records.resize(psiz + 16 + string(sql_row[4]).length());
                *(uint32_t *)(records.data() + psiz) = htonl(string2int(sql_row[1]));
                *(uint64_t *)(records.data() + psiz + 4) = htobe64(string2ll(sql_row[3]));
                *(uint64_t *)(records.data() + psiz + 12) = htonl(string(sql_row[4]).length());
                memcpy((char *)records.data() + psiz + 16, string(sql_row[4]).data(), string(sql_row[4]).length());
                psiz = records.size();
            }
        }
    }
    return 0;
}

int Database::update_record_cnt(int uid, int count)
{
    string sqlstr = "update login set rcnt=" + int2string(count) + " where uid='" + int2string(uid) + "'";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    return 0;
}

int Database::get_record_cnt(int uid)
{
    string sqlstr = "select rcnt from login where uid=" + int2string(uid) + ";";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        sql_row = mysql_fetch_row(result);
        return string2int(string(sql_row[0]));
    }
    return -1;
}

int Database::update_login_color(int uid, int color)
{
    string sqlstr = "update login set color=" + int2string(color) + " where uid='" + int2string(uid) + "'";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    return 0;
}

int Database::get_login_color(int uid)
{
    string sqlstr = "select color from login where uid=" + int2string(uid) + ";";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        sql_row = mysql_fetch_row(result);
        return string2int(string(sql_row[0]));
    }
    return -1;
}

int Database::insert_file_record(int sendid, int recvid, int allsiz, string filename)
{
    string sqlstr = "insert into filetrans(sendid,recvid,allsiz,filename) values('" + int2string(sendid) + "','" + int2string(recvid) + "','" + int2string(allsiz) + "','" + filename + "')";
    log_file(sendid,recvid,filename,allsiz);
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("insert failed");
        return -1;
    }
    return 0;
}

int Database::permit_file_record(int sendid, int recvid)
{
    string sqlstr = "update filetrans set statue=1 where sendid = " + int2string(sendid) + " and recvid = " + int2string(recvid) + " and statue = 0";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("update failed");
        return -1;
    }
    return 0;
}

int Database::delete_file_record(int sendid, int recvid)
{
    string sqlstr = "delete from filetrans where sendid = " + int2string(sendid) + " and recvid = " + int2string(recvid) + " and statue = 0";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("update failed");
        return -1;
    }
    return 0;
}

int Database::update_file_record(int sendid, int recvid, int siz)
{
    string sqlstr = "update filetrans set succsiz=" + int2string(siz) + " where sendid = " + int2string(sendid) + " and recvid = " + int2string(recvid) + " and statue = 1";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("update failed");
        return -1;
    }
    return 0;
}

int Database::get_file_record(int sendid, int &recvid, string &filename, int &siz)
{
    string sqlstr = "select recvid,filename,succsiz from filetrans where sendid = " + int2string(sendid) + " and succsiz < allsiz";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("get failed");
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        sql_row = mysql_fetch_row(result);
        recvid = string2int(sql_row[0]);
        filename = string(sql_row[1]);
        siz = string2int(sql_row[2]);
        return 1;
    }
    else if (mysql_num_rows(result) == 0)
    {
        return 0;
    }
    return -1;
}

int Database::get_all_uid(vector<uint32_t> &uids)
{
    uids.clear();
    string sqlstr = "select uid from login";
    if (mysql_query(mysql, sqlstr.data()))
    {
        cout << mysql_error(mysql) << endl;
        printf("get failed");
        return -1;
    }
    result = mysql_store_result(mysql);
    if (result && mysql_num_rows(result))
    {
        while ((sql_row = mysql_fetch_row(result)))
        {
            uids.push_back(string2int(sql_row[0]));
        }
    }
    return 0;
}