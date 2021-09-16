#include "TcpFileClient.h"

void TcpFileClient::receiveData()
{
	const auto total_length = m_file_header.length;
	if (total_length == TcpClient::HEADER_BYTE_LENGTH)
		return;
    m_f_received_current_data = this->blockRead(total_length - TcpClient::HEADER_BYTE_LENGTH);
}

void TcpFileClient::parseFileHeader()
{
   	receiveData(); // recv whole frame
	const auto type = m_file_header.type;
    switch (type) {
    case 0xA7: // 服务器向文件接收者请求发送
        return this->parseFileRequest();
    case 0xA8: // 服务器向文件发送者回复
        return this->parseFileAccessRespond();
    case 0xA9: // 服务器向文件接收者发送文件
        return this->parseFilePartRequest();
    case 0xAA: // 服务器向文件发送者回馈成功
        return this->parseFileSuccessRespond();
    default: return;
    }
}

void TcpFileClient::parseFileRequest()
{
    Q_ASSERT(m_received_header.type == 0xA7);
    Q_ASSERT(m_received_header.sub_type == 0x00);
    this->fetchFileRequest();
    // TODO
}

void TcpFileClient::fetchFileRequest()
{   
    Q_ASSERT(m_received_header.type == 0xA7);
    QDataStream data_stream(m_file_data);
    data_stream >> m_from_file_uid;
    data_stream >> m_from_file_size;
    m_from_filename = QString::fromLocal8Bit(m_file_data.right(FILENAME_BYTE_LENGTH));
    //m_from_filename = filename.resize(FILENAME_BYTE_LENGTH, 0);
}

void TcpFileClient::parseFileAccessRespond()
{
    Q_ASSERT(m_received_header.type == 0xA8);
    const auto sub_type = m_received_header.sub_type;
    Q_ASSERT(sub_type == 0x01 || sub_type == 0x02);
    if (sub_type == 0x01) { // 对方接受，可以发送文件
        this->fetchFileAccessRespond();
        return;
    }
    //对方拒绝，关闭此次文件传输 0x02
	closeFileConnection();
}

void TcpFileClient::fetchFileAccessRespond()
{
    QDataStream data_stream(m_file_data);
    data_stream >> m_from_file_uid;
}

void TcpFileClient::parseFilePartRequest()
{
    Q_ASSERT(m_received_header.type == 0xA9);
    Q_ASSERT(m_received_header.sub_type == 0x00);
    this->fetchFilePartRequest();
    // TODO write to file or ..
}

void TcpFileClient::fetchFilePartRequest()
{   
    Q_ASSERT(m_received_header.type == 0xA9);
    Q_ASSERT(m_received_header.sub_type == 0x00);
    QDataStream data_stream(m_file_data);
    data_stream >> m_from_file_uid;
    const auto file_part = m_file_data.right(m_file_data.size() - UID_BYTE_LENGTH);
    m_from_file_part = QString::fromLocal8Bit(file_part);
}

void TcpFileClient::parseFileSuccessRespond()
{
    Q_ASSERT(m_received_header.type == 0xAA);
    Q_ASSERT(m_received_header.sub_type == 0x00);
    this->fetchFileSuccessRespond();
    // TODO send left file_part (m_to_file_size - m_from_file_size)
}

void TcpFileClient::fetchFileSuccessRespond()
{   
    Q_ASSERT(m_received_header.type == 0xAA);
    Q_ASSERT(m_received_header.sub_type == 0x00);
    QDataStream data_stream(m_file_data);
    data_stream >> m_from_file_uid;
    data_stream >> m_from_file_size; 
}

void TcpFileClient::composeFileRequest()
{
    Q_ASSERT(!m_to_filename.isEmpty());
    // header 
	m_send_header.type = 0x17;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH 
        + FILENAME_BYTE_LENGTH + FILESIZE_BYTE_LENGTH + HEADER_BYTE_LENGTH;
	// whole message
	m_file_data.clear();
	QDataStream data_stream(&m_file_data, QIODevice::WriteOnly);
    // toLittle length uid file_size
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
    data_stream << m_to_file_uid << m_to_file_size;
    m_file_data.append(m_to_filename.toLocal8Bit());
}

void TcpFileClient::composeFileAccessRespond()
{
    // header 
	m_send_header.type = 0x18;
	m_send_header.sub_type = 0x01; // TODO
	m_send_header.length = UID_BYTE_LENGTH + HEADER_BYTE_LENGTH;
	// whole message
	m_file_data.clear();
	QDataStream data_stream(&m_file_data, QIODevice::WriteOnly);
    // toLittle length uid
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
    data_stream << m_to_file_uid;
}

void TcpFileClient::composeFilePartRequest()
{
    // header 
	m_send_header.type = 0x19;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH + m_to_file_part.size() + HEADER_BYTE_LENGTH;
	// whole message
	m_file_data.clear();
	QDataStream data_stream(&m_file_data, QIODevice::WriteOnly);
    // toLittle length uid
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
    data_stream << m_to_file_uid;
    m_file_data.append(m_to_file_part.toLocal8Bit());
}

void TcpFileClient::composeFileSuccessRespond()
{
    // header 
	m_send_header.type = 0x1A;
	m_send_header.sub_type = 0x00;
	m_send_header.length = UID_BYTE_LENGTH + FILESIZE_BYTE_LENGTH + HEADER_BYTE_LENGTH;
	// whole message
	m_file_data.clear();
	QDataStream data_stream(&m_file_data, QIODevice::WriteOnly);
    // toLittle length uid size
	data_stream << m_send_header.type << m_send_header.sub_type << m_send_header.length;
    data_stream << m_to_file_uid << m_to_file_size;
    // size initial
}