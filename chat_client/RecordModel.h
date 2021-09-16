#ifndef RECORDMODEL_H
#define RECORDMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

class RecordModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit RecordModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setUid(quint32 uid);
    void addRecord(quint32 uid, bool sentByMe, QString name, QString txt, qint64 time);
    void sendMsg(QString txt);
    void clear();

private:
    struct Record {
        bool sentByMe;
        QString name;
        QString txt;
        QString time;
    };
    enum RoleName {
        role_sentByMe = Qt::UserRole,
        role_name,
        role_txt,
        role_time
    };

    quint32 _cur_uid;
    QMap<quint32, QList<Record>> _records;
    QHash<int, QByteArray> _roleNames;
};

#endif // RECORDMODEL_H
