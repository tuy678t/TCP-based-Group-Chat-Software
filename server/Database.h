#ifndef SSQL_H
#define SSQL_H
#include <mysql.h>
#include <cstdint>
#include <string>
#include <vector>
using namespace std;

//mysql_real_connect����
#define HOST "localhost" //���ݿ������
#define USER "u1652196"      //���ݿ��û���
#define PASSWD "u1652196"   //���ݿ�����
#define DB "db1652196"      //���ݿ���
#define PORT 0           //���ݿ�˿�
#define UNIX_SOCKET NULL //UNIX_SOCKET
#define CLIENT_FLAG 0    //�ͻ�������

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
   * ��ʼ��MYSQL�ṹ������������Ӧ���ݿ⣬���󷵻�-1
   */
  int get_connect();
  /**
   * �ر����ӣ��ط���0
   */
  int dis_connect();
  /**
   * ����login��
   * username Ϊע���û���
   * password Ϊ����
   */
  int insert_login(string username, string passwd);
  /**
   * ���Ժ������鿴login��ȫ��
   */
  int select_login_all();
  /**
   * ��¼����,���󷵻�-1
   * username Ϊ��¼�û���
   * password Ϊ����
   * legal Ϊ���û���Ϊ�Ϸ��û�
   * first Ϊ�Ƿ�Ϊ��һ�ε�½
   * uid Ϊ�û���uid������
   */
  int login_check(string username, string passwd,bool &legal, bool &first, uint32_t &uid);




  int login_check(string username, string passwd,bool&exist,bool &legal, bool &first, uint32_t &uid);
  /**
   * ����uid�����û������󷵻�-1
   * uid Ϊ�û���uid
   * exist Ϊ���û��Ƿ����
   * username Ϊ��¼�û���������
   */
  int check_uid(int uid, bool &exist, string &username);
  /**
   * �û��������룬���󷵻�-1
   * uid Ϊ�û���uid
   * newpasswd Ϊ�û���������
   */
  int update_passwd(int uid, string newpasswd,string oldpasswd);
  /**
   * ���Ժ���������ʱ���ת���ɱ�׼�ַ���
   * cur Ϊʱ���
   */
  string get_curtime(time_t cur);
  /**
   * ������Ϣ��Ϣ�������ݿ⣬���󷵻�-1
   * sendid ������id
   * recvid ������id
   * text Ϊ��Ϣ�ı�
   */
  int save_message(int sendid, int recvid, string text);
  /**
   * ������Ϣ��¼���Ĵ���records�У����󷵻�-1
   * myid Ϊ��¼������û�
   * srid Ϊ�����û�
   * records Ϊ��Ϣ��¼��(������Ϣ��¼�������Ϊ�������ݿ��е���Ϣ�޶�������˳��ʱ�����ȵ�������)
   * ÿ����Ϣ��¼��ɣ�(��ת������)
   * ���ͷ����(4���ֽ�)
   * ��Ϣ��¼ʱ���(8���ֽ�)
   * ��Ϣ����(<1024��0��β)
   */
  int serh_message(int myid, int srid, vector<uint8_t> &records);
  /**
   * ������Ϣ��¼���Ĵ���records�У����󷵻�-1
   * peo1 Ϊ�û�1
   * peo2 Ϊ�û�2
   * records Ϊ��Ϣ��¼��(������Ϣ��¼�������Ϊcount��˳��ʱ�����ȵ�������)
   * count Ϊ�޶���¼��
   * ÿ����Ϣ��¼��ɣ�(��ת������)
   * ���ͷ����(4���ֽ�)
   * ��Ϣ��¼ʱ���(8���ֽ�)
   * ��Ϣ����(4���ֽ�)
   * ��Ϣ����(��Ϣ���ȸ��ֽ�)
   */
  int serh_message(int peo1, int peo2, vector<uint8_t> &records, int count);
  /**
   * ������Ϣ�޶���(���ݿ���Ĭ��Ϊ10����¼)�����󷵻�-1
   * uid Ϊ�û���uid
   * count Ϊ�޶���¼��
   */
  int update_record_cnt(int uid, int count);
  /**
   * �����Ϣ�޶��������󷵻�-1
   * uid Ϊ�û���uid
   */
  int get_record_cnt(int uid);
  /**
   * ������ɫ��Ĭ��Ϊ0)�����󷵻�-1
   * uid Ϊ�û���uid
   * count Ϊ�޶���¼��
   */
  int update_login_color(int uid, int color);
  /**
   * ��õ�¼��ɫ
   * ����ֵ��
   * -1����ѯ����
   * >=0������趨
   */
  int get_login_color(int uid);
  /**
   * ���봫���ļ���¼�����󷵻�-1
   * sendid Ϊ�����ߵ�uid
   * recvid Ϊ�����ߵ�uid
   * siz Ϊ�ļ���С
   * filename Ϊ�ļ���
   */
  int insert_file_record(int sendid, int recvid, int siz, string filename);
  /**
   * �����ļ���¼��¼��Ϊ��Ч�����󷵻�-1
   * sendid Ϊ�����ߵ�uid
   * recvid Ϊ�����ߵ�uid
   */
  int permit_file_record(int sendid, int recvid);
  /**
   * ɾ��������Ч��¼�����󷵻�-1
   * sendid Ϊ�����ߵ�uid
   * recvid Ϊ�����ߵ�uid
   */
  int delete_file_record(int sendid, int recvid);
  /**
   * �����ѷ����ֽ��������󷵻�-1
   * sendid Ϊ�����ߵ�uid
   * recvid Ϊ�����ߵ�uid
   * siz �˴γɹ������ֽ���
   */
  int update_file_record(int sendid, int recvid, int siz);
  /**
   * ���δ��������ѷ����ֽ�����
   * ���󷵻�-1
   * �м�¼����1
   * �޼�¼����0
   * sendid Ϊ�����ߵ�uid
   * recvid Ϊ�����ߵ�uid������
   * filenameΪ�ļ���������
   * sizΪ���շ��ѽ����ֽڴ�С
   */
  int get_file_record(int sendid, int &recvid, string &filename, int &siz);

  int get_all_uid(vector<uint32_t> &uids);
};

#endif