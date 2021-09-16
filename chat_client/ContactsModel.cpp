#include "ContactsModel.h"

ContactsModel::ContactsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    _roleNames[role_uid] = "uid";
    _roleNames[role_is_offline] = "is_offline";
    _roleNames[role_name] = "name";
}

int ContactsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _contacts.count();
}

QVariant ContactsModel::data(const QModelIndex& index, const int role) const
{
    const auto row = index.row();
    if (row < 0 || row >= _contacts.count()) {
        return QVariant();
    }

    const auto& contact = _contacts[row];
    switch (role) {
    case role_uid:
        return contact.uid;
    case role_is_offline:
        return contact.is_offline;
    case role_name:
        return contact.name;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> ContactsModel::roleNames() const
{
    return _roleNames;
}

void ContactsModel::insertContact(quint32 uid, bool is_offline, QString name)
{
    emit beginInsertRows({}, _contacts.size(), _contacts.size());
    _contacts.append({uid, is_offline, name});
    emit endInsertRows();
}

bool ContactsModel::updateContact(quint32 uid, bool is_offline, QString name)
{
    const auto it = std::find_if(_contacts.begin(), _contacts.end(),
        [uid](const Contact& v) {
        return v.uid == uid;
    });
    if (it == _contacts.end())
        return false;

    const Contact newContact = {uid, is_offline, name};

    const auto index = it - _contacts.begin();

    emit beginRemoveRows({}, index, index);
    _contacts.erase(it);
    emit endRemoveRows();


    if (is_offline) {
        emit beginInsertRows({}, _contacts.size(), _contacts.size());
        _contacts.append(newContact);
    }
    else {
        emit beginInsertRows({}, 1, 1);
        _contacts.insert(1, newContact);
    }
    emit endInsertRows();

    return true;
}

void ContactsModel::clear()
{
    _contacts.clear();
}

