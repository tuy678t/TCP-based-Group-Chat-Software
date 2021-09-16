#include "RecordModel.h"
#include <QDateTime>
#include <ctime>

RecordModel::RecordModel(QObject *parent)
    : QAbstractListModel(parent)
{
    _roleNames[role_sentByMe] = "sentByMe";
    _roleNames[role_name] = "name";
    _roleNames[role_txt] = "txt";
    _roleNames[role_time] = "time";
}

int RecordModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _records[_cur_uid].count();
}

QVariant RecordModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(_records.contains(_cur_uid));

    const auto& record_list = _records[_cur_uid];
    const auto row = index.row();
    if (row < 0 || row >= record_list.count()) {
        return QVariant();
    }

    const auto& record = record_list[row];
    switch (role) {
    case role_sentByMe:
        return record.sentByMe;
    case role_name:
        return record.name;
    case role_txt:
        return record.txt;
    case role_time:
        return record.time;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> RecordModel::roleNames() const
{
    return _roleNames;
}

void RecordModel::setUid(const quint32 uid)
{
//    Q_ASSERT(_records.contains(uid));

    emit layoutAboutToBeChanged();
    _cur_uid = uid;
    emit layoutChanged();
}

void RecordModel::addRecord(quint32 uid, bool sentByMe, QString name, QString txt, qint64 time)
{
    Record record;
    record.sentByMe = sentByMe;
    record.name = name;
    record.txt = txt;
    record.time = QDateTime::fromSecsSinceEpoch(time).toString("MM/dd/yyyy hh:mm:ss");

    if (uid == _cur_uid) {
        emit beginInsertRows({}, 0, 0);
    }

    _records[uid].prepend(record);

    if (uid == _cur_uid) {
        emit endInsertRows();
    }
}

void RecordModel::sendMsg(QString txt)
{
    this->addRecord(_cur_uid, true, "Me", txt, time(nullptr));
}

void RecordModel::clear()
{
    _records.clear();
}
