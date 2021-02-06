/*
 * Copyright 2020 Richard Liebscher <richard.liebscher@gmail.com>.
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

#ifndef CLIPBOARDPLUGIN_H
#define CLIPBOARDPLUGIN_H

#include <QObject>

#include <core/kdeconnectplugin.h>

class QClipboard;

class ClipboardPlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", 
        "org.kde.kdeconnect.device.sailfishconnect_clipboard")

public:
    ClipboardPlugin(QObject* parent, const QVariantList& args);

    QString dbusPath() const override;

    void connected() override {}
    bool receivePacket(const NetworkPacket &np) override;
    Q_SCRIPTABLE void pushClipboard();

private:
    QClipboard* m_clipboard;
    QString m_currentContent;
    qint64 m_updateTimestamp = 0;
};

#endif // CLIPBOARDPLUGIN_H
