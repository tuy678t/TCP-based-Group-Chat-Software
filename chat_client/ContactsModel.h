#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QAbstractListModel>

class ContactsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ContactsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

	friend class TcpClient;

    void insertContact(quint32 uid, bool is_offline, QString name);
    bool updateContact(quint32 uid, bool is_offline, QString name);
    void clear();

private:
    struct Contact {
        quint32 uid;
        bool is_offline;
        QString name;
    };
    enum RoleName {
        role_uid = Qt::UserRole,
        role_is_offline,
        role_name
    };

    QList<Contact> _contacts;
    QHash<int, QByteArray> _roleNames;
};

#endif // CONTACTSMODEL_H
