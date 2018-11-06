#pragma once

#include <QAbstractListModel>
#include <QStringList>

class Device;

namespace SailfishConnect {

class DevicePluginsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString deviceId
               READ deviceId
               WRITE setDeviceId
               NOTIFY deviceIdChanged)

public:
    enum ExtraRoles {
        NameRole = Qt::DisplayRole,
        EnabledRole = Qt::CheckStateRole,

        DescriptionRole = Qt::UserRole,
        IconUrlRole
    };

    explicit DevicePluginsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& idx,
                 const QVariant& value,
                 int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QString deviceId();
    void setDeviceId(const QString& value);

    QHash<int, QByteArray> roleNames() const override;

signals:
    void deviceIdChanged();

private:
    Device* device_ = nullptr;

    QStringList pluginIds_;

    void setDevice(Device* value);
    void update();
};

} // namespace SailfishConnect
