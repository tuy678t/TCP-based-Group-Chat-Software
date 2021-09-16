#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QQueue>
#include <QFile>
#include <QUrl>
#include <QColor>
#include "ContactsModel.h"
#include "RecordModel.h"

using uid_t = quint32;

struct UserInfo
{
	bool is_offline;
	QString name;
};

class TcpClient : public QObject
{
	Q_OBJECT
		Q_PROPERTY(ContactsModel* contactsModel MEMBER m_contactsModel CONSTANT)
		Q_PROPERTY(RecordModel* recordModel MEMBER m_recordModel CONSTANT)
        Q_PROPERTY(QColor theme1 MEMBER m_theme1 CONSTANT)
        Q_PROPERTY(QColor theme2 MEMBER m_theme2 CONSTANT)
        Q_PROPERTY(QColor theme3 MEMBER m_theme3 CONSTANT)

public:
	//	QMap<uid_t, QList<ChatElem>> map_records; //saved
	QMap<uid_t, UserInfo> map_contacts; //saved

	//TODO type
	constexpr static auto HEADER_BYTE_LENGTH = 4;
	constexpr static auto UNAME_BYTE_LENGTH = 28;
	constexpr static auto PASSWORD_BYTE_LENGTH = 28;
	constexpr static auto UID_BYTE_LENGTH = 4;
	constexpr static auto UID_FOR_ALL = 0xFFFF;
    constexpr static auto FILEPART_MAX_SIZE = 0xFFFF - 12;

	constexpr static auto FILENAME_BYTE_LENGTH = 64;
	constexpr static auto FILESIZE_BYTE_LENGTH = 4;
	constexpr static auto CONNECTION_FILE_NAME = "connection.conf";

	TcpClient(QObject* parent = nullptr);
	~TcpClient();
	TcpClient(const TcpClient&) = delete;
	TcpClient(TcpClient&&) = delete;
	TcpClient& operator=(const TcpClient&) = delete;
	TcpClient& operator=(TcpClient&&) = delete;

public slots:
	// send messages interface
	void connect2Host();
	void login(const QString& name, const QString& password);
	void sendMessage(const QString& toMessage);
	void changePassword(const QString& oldPassword, const QString& newPassword);
    void sendFile(const QUrl& toFilename);

	void closeConnection();
	//	void closeFileConnection() const;
	void receiveHeader();
	//	void receiveFileHeader();

	void setUid(quint32 uid);

signals:
	void connected(); // TODO
	void connectFailed();
	void disconnected();

	void loginSuccess();	//登陆成功
	void needChangePassword();	//登陆成功且需要改密
	void logoutOthers();	//登录成功且强制下线其他会话
	void loginFailed();		//登陆失败，用户名或密码错

	void logoutByOthers();	//用户在其他地方登陆

//	void userToOnline();	//用户上线
//	void userToOffline();	//用户下线

//	void getUserNameSuccess();	//获取成功
//	void userNotExist();	//获取失败，对方不存在

//	void getChatHistorySuccess();

//	void sendMessageSuccess();	//发送成功
	void userOffline();	//发送失败，对方不在线

//	void recvUserMessage();		//私聊
//	void recvGroupMessage();	//群聊

	void passwordChangedSucc(); //更改密码成功
	void passwordChangedFail(); //更改密码失败

	void fileSendSuccess();		//发送成功
	void fileUserNotExist();	//对方不存在
	void fileUserOffline();		//对方不在线

private slots:
	void connectionSuccess();
	void connectionTimeout() const;
	void connectionError(QAbstractSocket::SocketError socketError);
	//	void connectionFileTimeout() const;

private:
	struct Header {
		quint16 length = 0;
		quint8 type = 0;
		quint8 sub_type = 0;
	};

	uid_t m_uid{}; // initialize in 0x90
	QString m_uname{}; //mine
	QString m_password{}; //mine
	QString m_old_password{}; //mine

	// configs 
    QColor m_theme1;
    QColor m_theme2;
    QColor m_theme3;

	Header m_send_header{}; // only header
//	Header m_send_f_header{}; // only header
	QByteArray m_send_message; // whole message
//	QByteArray m_send_f_message; // whole message
//	QQueue<QPair<ChatElem, quint32>> m_send_queue;

	uid_t m_to_uid{ UID_FOR_ALL }; //对方id
	uid_t m_from_uid{};	//文件对方id
//	uid_t m_to_file_uid{ UID_FOR_ALL }; //对方id 文件
	QString m_to_message; //给对方消息
	quint32 m_to_filesize; //文件大小
//	QString m_to_f_message; //给对方消息

	// TODO
	QString m_to_filename; //给对方文件名
	QByteArray m_to_filepart;
	QFile m_file;
	quint32 m_file_size;
	quint32 m_file_recv_cnt;

	QString m_host;
	quint16 m_port;
	Header m_received_header;
	//	Header m_f_received_header;
	QByteArray m_received_current_data;
	//	QByteArray m_f_received_current_data;

	QTimer *m_timeout_timer = Q_NULLPTR;
	//	QTimer *m_f_timeout_timer = Q_NULLPTR;
	QTcpSocket *m_socket = Q_NULLPTR;
	//	QTcpSocket *m_fsocket = Q_NULLPTR;

	QQueue<uid_t> m_queue_uid;

	ContactsModel* const m_contactsModel;
	RecordModel* const m_recordModel;

	QByteArray blockRead(qint64 readLength, int msecs = 30000) const;

	void sendWholeFrame() const;

	// parse stages
	void parseLoginInfos();
	void parseLogoutInfos();
	void parseUserListState();
	void parseGetUserName();
	void parseGetChatHistory();
	void parseSendMessageInfos(); // TODO
	void parseReceiveMessageInfos();
	void parseChangePasswordInfos();
	//	void parseSendFileInfos(); // TODO
	//	void parseReceiveFileInfos(); // TODO

	void fetchFileRequest();
	void parseFileRequest();
	void fetchFileAccessRespond();

	void parseFileAccessRespond();
	void parseFilePartRequest();
	void fetchFilePartRequest();
	void parseFileSuccessRespond();
	void fetchFileSuccessRespond();
	void composeFileRequest();
	void composeFileAccessRespond();
	void composeFilePartRequest();
	void composeFileSuccessRespond();
	void parseHeader();
	void receiveData();

	void getConnectionFromFile();
	void getUserName(uid_t toUid);
	void getChatHistory(uid_t toUid);

	void fetchConfigs();
	void fetchUserStates(bool isOffline = false);
	void fetchUserChatHistory(uid_t other_uid);
	void fetchRecvMessageHistory(bool group);

	void composeLogin();
	void composeGetUserName();
	void composeGetChatHistory();
	void composeSendMessage();
	void composeChangePassword();
	//	void composeSendFile();

	// set
	void setName(const QString& name);
	void setPassword(const QString& password);
	void setOldPassword(const QString& oldPassword);
	void setToUid(uid_t toUid);
	//	void setToFileUid(uid_t toFileUid);
	void setToMessage(const QString& toMessage);
	void setToFilename(const QString& toFilename);
};

#endif // TCPCLIENT_H
