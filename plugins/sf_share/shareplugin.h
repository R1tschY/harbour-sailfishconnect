/*
 * Copyright 2018 Richard Liebscher <richard.liebscher@gmail.com>.
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

#pragma once

#include <core/kdeconnectplugin.h>

class KJob;

class SharePlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.share")

public:
    using KdeConnectPlugin::KdeConnectPlugin;

    QString incomingPath() const;

    void connected() override { }
    void receivePacket(const NetworkPacket &np) override;
    
    Q_SCRIPTABLE void shareUrl(const QString& url) { shareUrl(QUrl(url), false); }
    Q_SCRIPTABLE void shareUrls(const QStringList& urls);
    Q_SCRIPTABLE void shareText(const QString& text);
    Q_SCRIPTABLE void openFile(const QString& file) { shareUrl(QUrl(file), true); }

    QString dbusPath() const override;

signals:
    void shareReceived(const QString& url);

private:
    void finished(KJob* job, qint64 modified, bool open);
    void shareUrl(const QUrl& url, bool open);
};
