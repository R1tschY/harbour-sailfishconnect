/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dbusinterfaces.h>
#include <core/kdeconnectpluginconfig.h>

class KdeConnectPluginConfig;

namespace SailfishConnect {

void checkForDbusError(const QDBusPendingCall& async);

// template<typename T>
// void checkForDbusError(const QDBusPendingReply<T>& reply) {
//     reply.waitForFinished();
//     if (!reply.isValid()) {
//         qCWarning(api_logger) 
//             << "Getting announcedName failed" 
//             << reply.error();
//         return QString();
//     } else {
//         return reply.value();
//     }
// }

class RemoteControlApi : public RemoteControlDbusInterface {
    Q_OBJECT
public:
    using RemoteControlDbusInterface::RemoteControlDbusInterface;

    Q_SCRIPTABLE void moveCursor(const QPoint &p) {
        checkForDbusError(RemoteControlDbusInterface::moveCursor(p));
    }

    Q_SCRIPTABLE void sendCommand(const QString &name, bool val) {
        checkForDbusError(RemoteControlDbusInterface::sendCommand(name, val));
    }

    Q_SCRIPTABLE void scroll(int x, int y) {
        checkForDbusError(RemoteControlDbusInterface::scroll(x, y));
    }
};

class RemoteKeyboardApi : public RemoteKeyboardDbusInterface {
    Q_OBJECT
public:
    using RemoteKeyboardDbusInterface::RemoteKeyboardDbusInterface;

    Q_SCRIPTABLE void sendKeyPress(
            const QString& key, int specialKey = 0, bool shift = false, bool ctrl = false,
            bool alt = false, bool sendAck = true)
    {
        return checkForDbusError(
            RemoteKeyboardDbusInterface::sendKeyPress(key, specialKey, shift, ctrl, alt, sendAck));
    }
};

class RemoteCommandsApi : public RemoteCommandsDbusInterface {
    Q_OBJECT
public:
    using RemoteCommandsDbusInterface::RemoteCommandsDbusInterface;

    Q_SCRIPTABLE void triggerCommand(const QString& key) {
        checkForDbusError(RemoteCommandsDbusInterface::triggerCommand(key));
    }
};

class RemoteSystemVolumeApi : public RemoteSystemVolumeDbusInterface {
    Q_OBJECT
public:
    using RemoteSystemVolumeDbusInterface::RemoteSystemVolumeDbusInterface;

    Q_SCRIPTABLE void sendMuted(const QString &name, bool muted) {
        checkForDbusError(RemoteSystemVolumeDbusInterface::sendMuted(name, muted));
    }

    Q_SCRIPTABLE void sendVolume(const QString &name, int volume) {
        checkForDbusError(RemoteSystemVolumeDbusInterface::sendVolume(name, volume));
    }
};

class ShareApi : public ShareDbusInterface {
    Q_OBJECT
public:
    using ShareDbusInterface::ShareDbusInterface;

    Q_SCRIPTABLE void openFile(const QString& file)
    {
        checkForDbusError(ShareDbusInterface::openFile(file));
    }

    Q_SCRIPTABLE void shareText(const QString& text)
    {
        checkForDbusError(ShareDbusInterface::shareText(text));
    }

    Q_SCRIPTABLE void shareUrl(const QString& url)
    {
        checkForDbusError(ShareDbusInterface::shareUrl(url));
    }

    Q_SCRIPTABLE void shareUrls(const QStringList& urls) {
        checkForDbusError(ShareDbusInterface::shareUrls(urls));
    }
};

class PluginConfigApi : public KdeConnectPluginConfig {
    Q_OBJECT
public:
    using KdeConnectPluginConfig::KdeConnectPluginConfig;
};

class DeviceApi : public DeviceDbusInterface {
    Q_OBJECT
    Q_PROPERTY(QStringList loadedPlugins READ loadedPlugins NOTIFY pluginsChangedProxy)
public:
    explicit DeviceApi(const QString& deviceId, QObject* parent = nullptr);

    Q_SCRIPTABLE void unpair() {
        checkForDbusError(DeviceDbusInterface::unpair());
    }

    Q_SCRIPTABLE void requestPair() {
        checkForDbusError(DeviceDbusInterface::requestPair());
    }

    Q_SCRIPTABLE void acceptPairing() {
        checkForDbusError(DeviceDbusInterface::acceptPairing());
    }

    Q_SCRIPTABLE void rejectPairing() {
        checkForDbusError(DeviceDbusInterface::rejectPairing());
    }

    Q_SCRIPTABLE QString encryptionInfo() {
        return DeviceDbusInterface::encryptionInfo();
    }

    QStringList loadedPlugins();

    Q_SCRIPTABLE bool isPluginLoaded(const QString& pluginId);

    Q_SCRIPTABLE bool isPluginEnabled(const QString& pluginId) {
        return DeviceDbusInterface::isPluginEnabled(pluginId);
    }

    // plugins

    Q_SCRIPTABLE RemoteControlApi* getRemoteControlApi() {
        return new RemoteControlApi(id());
    }

    Q_SCRIPTABLE RemoteCommandsApi* getRemoteCommandsApi() {
        return new RemoteCommandsApi(id());
    }

    Q_SCRIPTABLE RemoteSystemVolumeApi* getRemoteSystemVolumeApi() {
        return new RemoteSystemVolumeApi(id());
    }

    Q_SCRIPTABLE RemoteKeyboardApi* getRemoteKeyboardApi() {
        return new RemoteKeyboardApi(id());
    }

    Q_SCRIPTABLE ShareApi* getShareApi() {
        return new ShareApi(id());
    }

    Q_SCRIPTABLE PluginConfigApi* getPluginConfig(const QString& pluginId) {
        return new PluginConfigApi(id(), pluginId);
    }

    Q_SCRIPTABLE void pluginCall(const QString& plugin, const QString& method);

Q_SIGNALS:
    void pluginsChangedProxy();

private:
    QStringList m_loadedPlugins;
    bool m_loadedPluginsLoaded = false;
};

class DaemonApi : public DaemonDbusInterface {
    Q_OBJECT
public:
    using DaemonDbusInterface::DaemonDbusInterface;

    Q_SCRIPTABLE QString announcedName() {
        return DaemonDbusInterface::announcedName();
    }

    Q_SCRIPTABLE void setAnnouncedName(const QString& value) {
        checkForDbusError(DaemonDbusInterface::setAnnouncedName(value));
    }

    Q_SCRIPTABLE DeviceApi* getDevice(const QString& id)
    {
        return new DeviceApi(id);
    }

    Q_SCRIPTABLE void forceOnNetworkChange() {
        checkForDbusError(DaemonDbusInterface::forceOnNetworkChange());
    }
};

} // namespace SailfishConnect
