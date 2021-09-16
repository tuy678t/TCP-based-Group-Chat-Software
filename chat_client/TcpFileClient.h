#include <TcpClient.h>

class TcpFileClient : public QObject
{
public:
    constexpr static auto FILENAME_BYTE_LENGTH = 64;
    constexpr static auto FILESIZE_BYTE_LENGTH = 4;

private:
	struct Header {
		quint16 length = 0;
		quint8 type = 0;
		quint8 sub_type = 0;
	};
    Header m_file_header;
    QByteArray m_file_data;

    uid_t m_uid{};
    uid_t m_to_file_uid{}; 
    uid_t m_from_file_uid{};
    quint32 m_to_file_size{}; // 发送方全部发送字节大小 TODO
    quint32 m_from_file_size{}; // 接受方已接受字节数
    QString m_to_filename{};
    QString m_from_filename{};
    QString m_to_file_part{};
    QString m_from_file_part{};
}
