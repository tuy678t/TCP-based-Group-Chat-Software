#include "TcpClient.h"
#include <utility>
#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QString>
#include <QTimer>
#include <QFile>
#include <ctime>

constexpr bool IS_FILE = true;
constexpr bool IS_OFFLINE = true;
constexpr bool IS_ONLINE = !IS_OFFLINE;
constexpr bool IS_SUCCESS = true;
constexpr bool IS_FAIL = !IS_SUCCESS;

TcpClient::TcpClient(QObject* parent) :
	QObject(parent),
	// m_host("192.168.80.10"),
	// m_port(12345),
	m_timeout_timer(new QTimer()),
	m_socket(new QTcpSocket()),
	m_contactsModel(new ContactsModel(this)),
	m_recordModel(new RecordModel(this))
{
	this->getConnectionFromFile();
	qDebug() << "host :" << m_host;
	qDebug() << "port :" << m_port;

	m_timeout_timer->setSingleShot(true);
	connect(m_timeout_timer, &QTimer::timeout, this, &TcpClient::connectionTimeout);
	connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::closeConnection);
	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
	connect(m_socket, &QTcpSocket::connected, this, &TcpClient::connectionSuccess);
	connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::receiveHeader);
	this->connect2Host();
}

TcpClient::~TcpClient()
{
	if (m_socket->isOpen())
		m_socket->disconnectFromHost();
	delete m_socket;
	delete m_timeout_timer;
}

/*** public ***/

void TcpClient::connect2Host()
{
	// clean up
	m_recordModel->clear();
	m_contactsModel->clear();
	map_contacts.clear();
	m_queue_uid.clear();
	m_contactsModel->insertContact(UID_FOR_ALL, false, QString::fromLocal8Bit("所有人"));

	m_timeout_timer->start(5000); // 5 seconds
//	m_f_timeout_timer->start(5000); // 5 seconds

	m_socket->connectToHost(m_host, m_port, QTcpSocket::ReadWrite);
	//	m_fsocket->connectToHost(m_host, m_port, QTcpSocket::ReadWrite); // TODO same port?
}

void TcpClient::sendWholeFrame() const
{
	qint64 has_read = 0;
	const auto expected_length = m_send_message.size();
	const auto p_data = m_send_message.data();

	qDebug() << __func__ << expected_length;
	Q_ASSERT(expected_length == m_send_header.length);

	while (has_read < expected_length) {
		auto res = m_socket->write(p_data + has_read, expected_length - has_read);
		has_read += res;
	}
}

void TcpClient::login(const QString& name, const QString& password)
{
	setName(name);
	setPassword(password);

	qDebug() << __func__;

	this->composeLogin();
	this->sendWholeFrame();
}

void TcpClient::getUserName(const uid_t toUid)
{
	m_queue_uid.push_back(toUid);
	setToUid(toUid);
	this->composeGetUserName();
	this->sendWholeFrame();
}

void TcpClient::getChatHistory(const uid_t toUid)
{
	m_queue_uid.push_back(toUid);
	setToUid(toUid);
	this->composeGetChatHistory();
	this->sendWholeFrame();
}

void TcpClient::sendMessage(const QString& toMessage)
{
	//	setToUid(toUid);
	setToMessage(toMessage);
	this->composeSendMessage();
	this->sendWholeFrame();
	// saved
	m_recordModel->addRecord(m_to_uid, true, m_uname, toMessage, time(nullptr));
}

void TcpClient::changePassword(const QString& oldPassword, const QString& newPassword)
{
	setPassword(newPassword);
	setOldPassword(oldPassword);

	this->composeChangePassword();
	this->sendWholeFrame();
}

void TcpClient::sendFile(const QUrl &toFilename)
{
    const auto filepath = toFilename.toLocalFile();
    const auto filename = toFilename.fileName();
	// m_to_uid is initialized
	m_from_uid = m_uid; // me
    setToFilename(filename);	// resize
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
	Q_ASSERT(file.exists());
	m_to_filesize = file.size();

	this->composeFileRequest();
	this->sendWholeFrame();

	quint32 cnt = 0;
	while (cnt < m_to_filesize) {
		m_to_filepart = file.read(FILEPART_MAX_SIZE);
        qDebug() << __func__ << m_to_filepart.size();
		if (m_to_filepart.size() == 0)
            break;

		cnt += m_to_filepart.size();

		this->composeFilePartRequest();
		this->sendWholeFrame();
	}
    Q_ASSERT(cnt == m_to_filesize);
}

/*** public slots ***/

void TcpClient::closeConnection()
{
	m_timeout_timer->stop();
	qDebug() << m_socket->state();

	switch (m_socket->state())
	{
	case QAbstractSocket::UnconnectedState:
		m_socket->disconnectFromHost();
		break;
	case QAbstractSocket::ConnectingState:
		m_socket->abort();
		break;
	default:
		m_socket->abort();
	}
	emit disconnected();
}

void TcpClient::receiveHeader()
{
	while (m_socket->bytesAvailable() > 0) {
		const auto header = this->blockRead(TcpClient::HEADER_BYTE_LENGTH);

		Q_ASSERT(header.size() == TcpClient::HEADER_BYTE_LENGTH); // has some to read
		QDataStream data_stream(header);
		data_stream >> m_received_header.type;
		data_stream >> m_received_header.sub_type;
		data_stream >> m_received_header.length;

		this->parseHeader();
	}
}

void TcpClient::setUid(uid_t uid)
{
	m_recordModel->setUid(uid);
	m_to_uid = uid;
}

//void TcpClient::receiveFileHeader()
//{
//	const auto header = this->blockRead(TcpClient::HEADER_BYTE_LENGTH, IS_FILE);

//	Q_ASSERT(header.size() == TcpClient::HEADER_BYTE_LENGTH); // has some to read
//	QDataStream data_stream(header);
//	data_stream >> m_f_received_header.type;
//	data_stream >> m_f_received_header.sub_type;
//	data_stream >> m_f_received_header.length;

//    this->parseHeader(IS_FILE);
//}

/*** private slots ***/

void TcpClient::connectionSuccess()
{
	qDebug() << "connected";
	m_timeout_timer->stop();
	emit connected();
}

void TcpClient::connectionTimeout() const
{
	// TODO details
	m_timeout_timer->stop();
	if (m_socket->state() == QAbstractSocket::ConnectingState) {
		m_socket->abort();
		emit m_socket->error(QAbstractSocket::SocketTimeoutError);
	}
}

void TcpClient::connectionError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "connection error" << socketError;
	emit connectFailed();
}

//void TcpClient::connectionFileTimeout() const
//{
//	// TODO details
//	m_f_timeout_timer->stop();
//	qDebug() << m_fsocket->state();
//	if (m_fsocket->state() == QAbstractSocket::ConnectingState) {
//		m_fsocket->abort();
//		emit m_fsocket->error(QAbstractSocket::SocketTimeoutError);
//    }
//}

/*** private ***/

/* receive */

QByteArray TcpClient::blockRead(const qint64 readLength,
	const int msecs) const
{
	const auto socket_temp = m_socket;
	while (socket_temp->bytesAvailable() < readLength) {
		if (!socket_temp->waitForReadyRead(msecs)) { // 30000ms default
			emit socket_temp->error(QAbstractSocket::SocketTimeoutError);  // TODO signal emit
		}
	}
	return socket_temp->read(readLength);
}

void TcpClient::parseLoginInfos()
{
	Q_ASSERT(m_received_header.type == 0x90);
	qDebug() << __func__ << m_received_header.sub_type;
	switch (m_received_header.sub_type) {
	case 0x00: //登陆成功
		fetchConfigs(); // TODO get uid
		emit loginSuccess();
		break;
	case 0x01: //登陆成功且需要改密
		fetchConfigs();
		emit needChangePassword(); // 还需要 #1 无需此时配置
		break;
	case 0x02: //登录成功且强制下线其他会话
		fetchConfigs();
		emit logoutOthers();
		break;
	case 0xF0: //登陆失败，用户名或密码错
		//fetchConfigs();
		emit loginFailed();
	case 0xFF: //报文格式错
	default:
		closeConnection();
	}
}

void TcpClient::parseLogoutInfos()
{
	Q_ASSERT(m_received_header.type == 0xA0);

	switch (m_received_header.sub_type) {
	case 0x00: //用户在其他地方登陆
		emit logoutByOthers();
	default:
		closeConnection();
	}
}

void TcpClient::parseUserListState()
{
	Q_ASSERT(m_received_header.type == 0xA1);

	switch (m_received_header.sub_type) {
	case 0x00: //用户上线
		this->fetchUserStates(IS_ONLINE);
		// this->getAllNames();
		break;
	case 0x01: //用户下线
		this->fetchUserStates(IS_OFFLINE);
		// this->getAllNames();
		break;
	default:
		closeConnection();
	}
}

void TcpClient::parseGetUserName()
{
	Q_ASSERT(m_received_header.type == 0x91);

	const auto sub_type = m_received_header.sub_type;
	if (sub_type == 0x00) {	//获取成功
		const auto uid = m_queue_uid.front();
		m_queue_uid.pop_front();
		const auto name = QString::fromLocal8Bit(m_received_current_data);
		map_contacts[uid].name = name;
		m_contactsModel->insertContact(uid, map_contacts[uid].is_offline, name);
	}
	else {
		closeConnection();
	}
}

void TcpClient::parseGetChatHistory()
{
	Q_ASSERT(m_received_header.type == 0x92);

	const auto sub_type = m_received_header.sub_type;
	if (sub_type == 0x00) {	//获取成功
		const auto uid = m_queue_uid.front();
		m_queue_uid.pop_front();
		this->fetchUserChatHistory(uid);
	}
	else {
		closeConnection();
	}
}

void TcpClient::parseSendMessageInfos()
{
	Q_ASSERT(m_received_header.type == 0x93);

	switch (m_received_header.sub_type) {
	case 0x00: //发送成功
//		emit sendMessageSuccess();
		break;
	case 0xF0: //发送失败，对方不存在
//		emit userNotExist();
		break;
	case 0xF1: //发送失败，对方不在线
		emit userOffline();
		break;
	case 0xFF: //报文格式错
	default:
		closeConnection();
	}
}

void TcpClient::parseReceiveMessageInfos()
{
	Q_ASSERT(m_received_header.type == 0xA3);

	switch (m_received_header.sub_type) {
	case 0x00: //私聊
		fetchRecvMessageHistory(false);
		//		emit recvUserMessage();
		break;
	case 0x01: //群聊
		fetchRecvMessageHistory(true);
		//		emit recvGroupMessage();
		break;
	default:
		closeConnection();
	}
}

void TcpClient::parseChangePasswordInfos()
{
	Q_ASSERT(m_received_header.type == 0x95);

	switch (m_received_header.sub_type) {
	case 0x00: // 更改成功
		emit passwordChangedSucc();
		break;
	case 0xF0: // 原密码错误
		emit passwordChangedFail();
		break;
	case 0xFF: //格式错误
	default:
		closeConnection();
	}
}

//void TcpClient::parseSendFileInfos()
//{
//	Q_ASSERT(m_f_received_header.type == 0x94);

//	switch (m_f_received_header.sub_type)
//	{
//	case 0x00: //发送成功
//		emit fileSendSuccess();
//		break;
//	case 0xF0: //发送失败，对方不存在
//		emit fileUserNotExist(); // TODO
//		break;
//	case 0xF1: //发送失败，对方不在线
//		emit fileUserOffline();
//		break;
//	case 0xFF: //报文格式错
//	default:
//		closeConnection();
//	}
//}

//void TcpClient::parseReceiveFileInfos()
//{
//	Q_ASSERT(m_f_received_header.type == 0x94);
//	Q_ASSERT(m_f_received_header.sub_type == 0x00);
//	// TODO emit file write done
//}

void TcpClient::parseHeader()
{
	const auto type = m_received_header.type;

	receiveData(); // recv whole frame

	qDebug() << __func__ << type;
	Q_ASSERT(m_received_header.length == m_received_current_data.size() + HEADER_BYTE_LENGTH);
	switch (type) {
	case 0x90: // 1 客户端登陆
		return this->parseLoginInfos();
	case 0xA0: // 2 强制下线
		return this->parseLogoutInfos();
	case 0xA1: // 3 更新用户列表
		return this->parseUserListState();
	case 0x91: // 4 获取用户名
		return this->parseGetUserName();
	case 0x92: // 5 获取聊天记录
		return this->parseGetChatHistory();
    case 0x93: // 6 发送文本信息
        return this->parseSendMessageInfos();
    case 0xA3: // 7 接收文本信息
        return this->parseReceiveMessageInfos();
	case 0x95: // 8 更改密码
		return this->parseChangePasswordInfos();
	case 0xA7: // 服务器向文件接收者请求发送
		return this->parseFileRequest();
	case 0xA8: // 服务器向文件发送者回复
		return this->parseFileAccessRespond();
	case 0xA9: // 服务器向文件接收者发送文件
		return this->parseFilePartRequest();
	case 0xAA: // 服务器向文件发送者回馈成功
		return this->parseFileSuccessRespond();
	default:;
	}
}

void TcpClient::receiveData()
{
	const auto total_length = m_received_header.length;
	if (total_length == HEADER_BYTE_LENGTH) {
		m_received_current_data.clear();
		return;
	}
	m_received_current_data = this->blockRead(total_length - HEADER_BYTE_LENGTH);
}

void TcpClient::getConnectionFromFile()
{
	QFile fp(CONNECTION_FILE_NAME);
	Q_ASSERT(fp.exists());
	if (!fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "file not exist!";
	}
	QTextStream in(&fp);
	auto line1 = in.readLine();
	m_host = line1.toLocal8Bit().data();
	auto line2 = in.readLine();
	m_port = line2.toLocal8Bit().toUShort();
	in.flush();
	fp.close();
}

void TcpClient::fetchConfigs()
{
    QRgb theme1, theme2, theme3;
	QDataStream data_stream(m_received_current_data);
    data_stream >> m_uid >> theme1;
    m_theme1 = QColor::fromRgb(theme1);
    m_theme2 = QColor::fromRgb(theme1);
	this->getChatHistory(UID_FOR_ALL);
}

void TcpClient::fetchUserStates(const bool isOffline)
{
	QDataStream data_stream(m_received_current_data);

	while (!data_stream.atEnd()) {
		uid_t temp_uid;
		data_stream >> temp_uid;
		qDebug() << __func__ << temp_uid;
		if (map_contacts.contains(temp_uid)) {
			m_contactsModel->updateContact(temp_uid, isOffline, map_contacts[temp_uid].name);
		}
		else {
			map_contacts[temp_uid].is_offline = isOffline;
			this->getUserName(temp_uid);
			this->getChatHistory(temp_uid);
			// m_contactsModel->insertContact(temp_uid, isOffline, {});
		}
	}
}

void TcpClient::fetchUserChatHistory(const uid_t other_uid)
{
	QDataStream data_stream(m_received_current_data);
	while (!data_stream.atEnd()) {
		//uid
		uid_t temp_uid;
		data_stream >> temp_uid;
		//name
		Q_ASSERT(m_uname.size() != 0);
		Q_ASSERT(!map_contacts.empty());
		//send_by_me
		const auto is_me = temp_uid == m_uid;
		const auto temp_name = is_me ? m_uname : map_contacts[temp_uid].name;
		//time
		time_t temp_time;
		data_stream >> temp_time;
		//message
		QByteArray msg;
		while (true) {
			qint8 c;
			data_stream >> c;
			if (c == 0)
				break;
			msg.append(c);
		}
		m_recordModel->addRecord(other_uid, is_me, temp_name, QString::fromLocal8Bit(msg), temp_time);
	}
}

void TcpClient::fetchRecvMessageHistory(bool group)
{
	QDataStream data_stream(m_received_current_data);
	uid_t temp_uid;
	data_stream >> temp_uid;

	const auto msg = m_received_current_data.right(m_received_current_data.size() - UID_BYTE_LENGTH);

	qDebug() << __func__ << temp_uid << QString::fromLocal8Bit(msg);

	m_recordModel->addRecord(group ? UID_FOR_ALL : temp_uid, false, map_contacts[temp_uid].name, QString::fromLocal8Bit(msg), time(nullptr));
}

/* send */

void TcpClient::composeLogin()
{
	// header
	m_send_header.type = 0x10;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UNAME_BYTE_LENGTH + UNAME_BYTE_LENGTH + HEADER_BYTE_LENGTH;

	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	m_send_message.append(m_uname.toLocal8Bit());
	m_send_message.append(m_password.toLocal8Bit());
}

void TcpClient::composeGetUserName()
{
	// header
	m_send_header.type = 0x11;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH + HEADER_BYTE_LENGTH;

	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid;
}

void TcpClient::composeGetChatHistory()
{
	// header
	m_send_header.type = 0x12;
	m_send_header.sub_type = m_to_uid == UID_FOR_ALL ? 0x01 : 0x00;
	m_send_header.length = UID_BYTE_LENGTH + HEADER_BYTE_LENGTH;

	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid;
}

void TcpClient::composeSendMessage()
{
	const auto data = m_to_message.toLocal8Bit();
	// header
	m_send_header.type = 0x13;
	m_send_header.sub_type = m_to_uid == UID_FOR_ALL ? 0x01 : 0x00;
	m_send_header.length = UID_BYTE_LENGTH + HEADER_BYTE_LENGTH + data.size(); // TODO

	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid;
	m_send_message.append(data);
}

void TcpClient::composeChangePassword()
{
	// header
	m_send_header.type = 0x15;
	m_send_header.sub_type = 0x00;
	m_send_header.length = PASSWORD_BYTE_LENGTH * 2 + HEADER_BYTE_LENGTH; // TODO

	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	m_send_message.append(m_old_password.toLocal8Bit());
	m_send_message.append(m_password.toLocal8Bit());
}

//void TcpClient::composeSendFile()
//{
//	// header
//	m_send_f_header.type = 0x14;
//	m_send_f_header.sub_type = m_to_uid == 0x00;
//	m_send_f_header.length = UID_BYTE_LENGTH +
//		HEADER_BYTE_LENGTH + FILENAME_BYTE_LENGTH/*+ filedetails */;

//	// whole message
//	m_send_f_message.clear();
//	QDataStream data_stream(&m_send_f_message, QIODevice::WriteOnly);
//	data_stream << m_send_f_header.type << m_send_f_header.sub_type << m_send_f_header.length;
//    data_stream << m_to_uid;
//    m_send_f_message.append(m_to_filename.toLocal8Bit());
//	// TODO file details
//}

/*** set value ***/

void TcpClient::setName(const QString& name)
{
	m_uname = name;
	m_uname.resize(UNAME_BYTE_LENGTH, 0);
}

void TcpClient::setPassword(const QString& password)
{
	m_password = password;
	m_password.resize(PASSWORD_BYTE_LENGTH, 0);
}

void TcpClient::setOldPassword(const QString& oldPassword)
{
	m_old_password = oldPassword;
	m_old_password.resize(PASSWORD_BYTE_LENGTH, 0);
}

void TcpClient::setToUid(const uid_t toUid)
{
	m_to_uid = toUid;
}

//void TcpClient::setToFileUid(const uid_t toFileUid)
//{
//	m_to_uid = toFileUid;
//}

void TcpClient::setToMessage(const QString& toMessage)
{
	m_to_message = toMessage;
}

void TcpClient::setToFilename(const QString& toFilename)
{
	m_to_filename = toFilename;
}

void TcpClient::parseFileRequest()
{
	Q_ASSERT(m_received_header.type == 0xA7);
	Q_ASSERT(m_received_header.sub_type == 0x00);
	this->fetchFileRequest();

	qDebug() << __func__ << m_to_uid << m_from_uid << m_to_filesize << m_to_filename;
	m_file.setFileName(m_to_filename);
	m_file.open(QIODevice::WriteOnly);
	m_file_recv_cnt = 0;
	m_file_size = m_to_filesize;
}

void TcpClient::fetchFileRequest()
{
    uid_t temp_uid;
	Q_ASSERT(m_received_header.type == 0xA7);
	QDataStream data_stream(m_received_current_data);
    data_stream >> temp_uid >> m_from_uid;
    Q_ASSERT(temp_uid == m_uid);
	data_stream >> m_to_filesize;
	m_to_filename = QString::fromLocal8Bit(m_received_current_data.right(FILENAME_BYTE_LENGTH));
}

void TcpClient::parseFileAccessRespond()
{
	Q_ASSERT(m_received_header.type == 0xA8);
	const auto sub_type = m_received_header.sub_type;
	Q_ASSERT(sub_type == 0x01 || sub_type == 0x02);
	if (sub_type == 0x01) { // 对方接受，可以发送文件
		this->fetchFileAccessRespond();
		return;
	}
	//对方拒绝，关闭此次文件传输 0x02
	//closeFileConnection();
}

void TcpClient::fetchFileAccessRespond()
{
    uid_t temp_uid;
	QDataStream data_stream(m_received_current_data);
    data_stream >> temp_uid >> m_from_uid;
    Q_ASSERT(temp_uid == m_uid);
}

void TcpClient::parseFilePartRequest()
{
	Q_ASSERT(m_received_header.type == 0xA9);
	Q_ASSERT(m_received_header.sub_type == 0x00);
	this->fetchFilePartRequest();

    qDebug() << __func__ << m_to_uid << m_from_uid << m_received_header.length;

	auto res = m_file.write(m_to_filepart);
	Q_ASSERT(res == m_to_filepart.size());

	m_file_recv_cnt += res;
	if (m_file_recv_cnt >= m_file_size) {
		Q_ASSERT(m_file_recv_cnt == m_file_size);
		m_file.close();
	}
}

void TcpClient::fetchFilePartRequest()
{
    uid_t temp_uid;
	Q_ASSERT(m_received_header.type == 0xA9);
	Q_ASSERT(m_received_header.sub_type == 0x00);
	QDataStream data_stream(m_received_current_data);
    data_stream >> temp_uid >> m_from_uid;
    Q_ASSERT(temp_uid == m_uid);
	m_to_filepart = m_received_current_data.right(m_received_current_data.size() - UID_BYTE_LENGTH * 2);
}

void TcpClient::parseFileSuccessRespond()
{
	Q_ASSERT(m_received_header.type == 0xAA);
	Q_ASSERT(m_received_header.sub_type == 0x00);
	this->fetchFileSuccessRespond();
	// TODO send left file_part (m_to_filesize - m_to_filesize)
}

void TcpClient::fetchFileSuccessRespond()
{
    uid_t temp_uid;
	Q_ASSERT(m_received_header.type == 0xAA);
	Q_ASSERT(m_received_header.sub_type == 0x00);
	QDataStream data_stream(m_received_current_data);
    data_stream >> temp_uid >> m_from_uid;
    Q_ASSERT(temp_uid == m_uid);
	data_stream >> m_to_filesize;
}

void TcpClient::composeFileRequest()
{
	Q_ASSERT(!m_to_filename.isEmpty());
	// header 
	m_send_header.type = 0x17;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH * 2
		+ FILENAME_BYTE_LENGTH + FILESIZE_BYTE_LENGTH + HEADER_BYTE_LENGTH;
	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	// toLittle length uid file_size
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid << m_uid << m_to_filesize;
    auto send_filename = m_to_filename.toLocal8Bit();
    m_send_message.append(send_filename);
    m_send_message.append(FILENAME_BYTE_LENGTH - send_filename.size(), 0);
}

void TcpClient::composeFileAccessRespond()
{
	// header 
	m_send_header.type = 0x18;
	m_send_header.sub_type = 0x01; // TODO
	m_send_header.length = UID_BYTE_LENGTH * 2 + HEADER_BYTE_LENGTH;
	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	// toLittle length uid
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid << m_uid;
}

void TcpClient::composeFilePartRequest()
{
	// header 
	m_send_header.type = 0x19;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH * 2 + m_to_filepart.size() + HEADER_BYTE_LENGTH;
	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	// toLittle length uid
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid << m_uid;
	m_send_message.append(m_to_filepart);
}

void TcpClient::composeFileSuccessRespond()
{
	// header 
	m_send_header.type = 0x1A;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH * 2 + FILESIZE_BYTE_LENGTH + HEADER_BYTE_LENGTH;
	// whole message
	m_send_message.clear();
	QDataStream data_stream(&m_send_message, QIODevice::WriteOnly);
	// toLittle length uid size
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
	data_stream << m_to_uid << m_uid << m_to_filesize;
	// size initial
}
