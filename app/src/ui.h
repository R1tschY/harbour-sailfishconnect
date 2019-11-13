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

#ifndef UI_H
#define UI_H

#include "plugins/remotekeyboard/keyboardlayoutprovider.h"
#include <QObject>
#include <QSettings>
#include <QString>
#include <memory>

class QQuickView;
class QQmlImageProviderBase;
class QEventLoopLocker;

namespace SailfishConnect {

class AppDaemon;

/**
 * @brief manages ui
 *
 * - dbus interface to ui
 * - creates the QQuickView with the main window
 */
class UI : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.harbour.SailfishConnect.UI")

    Q_PROPERTY(
        bool runInBackground
            READ runInBackground
                WRITE setRunInBackground
                    NOTIFY runInBackgroundChanged)

public:
    static const QString DBUS_INTERFACE_NAME;
    static const QString DBUS_PATH;

    explicit UI(
        KeyboardLayoutProvider* keyboardLayoutProvider,
        bool daemonMode,
        QObject* parent = nullptr);
    ~UI();

    /**
     * @brief notify other main daemon to show app window
     */
    static void raise();

    bool runInBackground();
    void setRunInBackground(bool value);

    static QVariant openDevicePageDbusAction(const QString& deviceId);

signals:
    void runInBackgroundChanged();
    Q_SCRIPTABLE void openingDevicePage(const QString& deviceId);

public slots:
    /**
     * @brief show the main window
     */
    Q_SCRIPTABLE void showMainWindow();

    /**
     * @brief quit application
     */
    Q_SCRIPTABLE void quit();

    /**
     * @brief open page of device
     * @param deviceId id of device
     */
    Q_SCRIPTABLE void openDevicePage(const QString& deviceId);

private slots:
    void onRegisteredService();
    void onUnregisteredService();

private:
    QQuickView* m_view = nullptr;
    std::unique_ptr<AppDaemon> m_daemon;
    KeyboardLayoutProvider* m_keyboardLayoutProvider;

    QSettings m_settings;
    bool m_runInBackground = false;
    bool m_daemonMode;

    bool eventFilter(QObject* watched, QEvent* event) override;
};

} // namespace SailfishConnect

#endif // UI_H
