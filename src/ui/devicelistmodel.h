#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class Daemon;
class Device;

namespace SailfishConnect {

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ExtraRoles {
        NameRole = Qt::UserRole,
        IdRole,
        IconUrlRole
    };

    explicit DeviceListModel(QObject *parent = 0);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    Daemon* m_daemon;
    QList<Device*> m_devices;

    void deviceIdAdded(const QString& id);
    void deviceIdRemoved(const QString& id);
    void deviceRemoved(Device *device);
    void deviceDataChanged(Device* device);

    void connectDevice(Device* device);
    void disconnectDevice(Device* device);

    int indexOfDevice(const QString& id);
    Device* getDevice(const QString& id);
};

} // namespace SailfishConnect

#endif // DEVICELISTMODEL_H
