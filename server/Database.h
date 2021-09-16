#ifndef SSQL_H
#define SSQL_H
#include <mysql.h>
#include <cstdint>
#include <string>
#include <vector>
using namespace std;

//mysql_real_connect参数
#define HOST "localhost" //数据库服务器
#define USER "u1652196"      //数据库用户名
#define PASSWD "u1652196"   //数据库密码
#define DB "db1652196"      //数据库名
#define PORT 0           //数据库端口
#define UNIX_SOCKET NULL //UNIX_SOCKET
#define CLIENT_FLAG 0    //客户端设置

class Database
{
private:
  MYSQL *mysql;
  MYSQL_RES *result;
  MYSQL_ROW sql_row;
  MYSQL_FIELD *fd;
  string int2string(int integer);
  string ll2string(long long lli);
  int string2int(string str);
  long long string2ll(string str);

public:
  /**
   * 初始化MYSQL结构，并连接上相应数据库，错误返回-1
   */
  int get_connect();
  /**
   * 关闭连接，必返回0
   */
  int dis_connect();
  /**
   * 加入login表
   * username 为注册用户名
   * password 为密码
   */
  int insert_login(string username, string passwd);
  /**
   * 测试函数，查看login表全体
   */
  int select_login_all();
  /**
   * 登录测试,错误返回-1
   * username 为登录用户名
   * password 为密码
   * legal 为该用户否为合法用户
   * first 为是否为第一次登陆
   * uid 为用户的uid的引用
   */
  int login_check(string username, string passwd,bool &legal, bool &first, uint32_t &uid);




  int login_check(string username, string passwd,bool&exist,bool &legal, bool &first, uint32_t &uid);
  /**
   * 根据uid查找用户，错误返回-1
   * uid 为用户的uid
   * exist 为该用户是否存在
   * username 为登录用户名的引用
   */
  int check_uid(int uid, bool &exist, string &username);
  /**
   * 用户设置密码，错误返回-1
   * uid 为用户的uid
   * newpasswd 为用户新设密码
   */
  int update_passwd(int uid, string newpasswd,string oldpasswd);
  /**
   * 测试函数，根据时间戳转化成标准字符串
   * cur 为时间戳
   */
  string get_curtime(time_t cur);
  /**
   * 保留消息信息进入数据库，错误返回-1
   * sendid 发送者id
   * recvid 接受者id
   * text 为消息文本
   */
  int save_message(int sendid, int recvid, string text);
  /**
   * 生成消息记录报文存于records中，错误返回-1
   * myid 为记录数相关用户
   * srid 为其余用户
   * records 为消息记录包(其中消息记录总数最多为存于数据库中的消息限定数量，顺序按时间由先到后排序)
   * 每条消息记录组成：(已转网络序)
   * 发送方编号(4个字节)
   * 消息记录时间戳(8个字节)
   * 消息正文(<1024以0结尾)
   */
  int serh_message(int myid, int srid, vector<uint8_t> &records);
  /**
   * 生成消息记录报文存于records中，错误返回-1
   * peo1 为用户1
   * peo2 为用户2
   * records 为消息记录包(其中消息记录总数最多为count，顺序按时间由先到后排序)
   * count 为限定记录数
   * 每条消息记录组成：(已转网络序)
   * 发送方编号(4个字节)
   * 消息记录时间戳(8个字节)
   * 消息长度(4个字节)
   * 消息正文(消息长度个字节)
   */
  int serh_message(int peo1, int peo2, vector<uint8_t> &records, int count);
  /**
   * 更新消息限定数(数据库中默认为10条记录)，错误返回-1
   * uid 为用户的uid
   * count 为限定记录数
   */
  int update_record_cnt(int uid, int count);
  /**
   * 获得消息限定数，错误返回-1
   * uid 为用户的uid
   */
  int get_record_cnt(int uid);
  /**
   * 更新颜色（默认为0)，错误返回-1
   * uid 为用户的uid
   * count 为限定记录数
   */
  int update_login_color(int uid, int color);
  /**
   * 获得登录颜色
   * 返回值：
   * -1：查询错误
   * >=0：相关设定
   */
  int get_login_color(int uid);
  /**
   * 插入传输文件记录表，错误返回-1
   * sendid 为发送者的uid
   * recvid 为接收者的uid
   * siz 为文件大小
   * filename 为文件名
   */
  int insert_file_record(int sendid, int recvid, int siz, string filename);
  /**
   * 传输文件记录记录置为有效，错误返回-1
   * sendid 为发送者的uid
   * recvid 为接收者的uid
   */
  int permit_file_record(int sendid, int recvid);
  /**
   * 删除该条无效记录，错误返回-1
   * sendid 为发送者的uid
   * recvid 为接收者的uid
   */
  int delete_file_record(int sendid, int recvid);
  /**
   * 更新已发送字节数，错误返回-1
   * sendid 为发送者的uid
   * recvid 为接收者的uid
   * siz 此次成功发送字节数
   */
  int update_file_record(int sendid, int recvid, int siz);
  /**
   * 获得未发送完的已发送字节数，
   * 错误返回-1
   * 有记录返回1
   * 无记录返回0
   * sendid 为发送者的uid
   * recvid 为接受者的uid的引用
   * filename为文件名的引用
   * siz为接收方已接收字节大小
   */
  int get_file_record(int sendid, int &recvid, string &filename, int &siz);

  int get_all_uid(vector<uint32_t> &uids);
};

#endif