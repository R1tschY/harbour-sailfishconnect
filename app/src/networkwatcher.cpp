/*
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "networkwatcher.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

#include <daemon.h>

Q_LOGGING_CATEGORY(loggerNetwork, "sailfishconnect.network")

namespace {
const char connmanService[] = "net.connman";
const char connmanPath[] = "/";
const char connmanManager[] = "net.connman.Manager";
} // namespace

ConnmanWatcher::ConnmanWatcher(QObject *parent)
    : QObject(parent)
{
    QDBusConnection bus = QDBusConnection::systemBus();

    const bool connected = bus.connect(
        QString::fromLatin1(connmanService), QString::fromLatin1(connmanPath),
        QString::fromLatin1(connmanManager), QStringLiteral("PropertyChanged"),
        this, SLOT(onPropertyChanged(QString, QDBusVariant)));
    if (!connected) {
        qCWarning(loggerNetwork) << "Cannot watch ConnMan state";
    }

    QDBusInterface manager(QString::fromLatin1(connmanService),
                           QString::fromLatin1(connmanPath),
                           QString::fromLatin1(connmanManager), bus);
    const QDBusReply<QVariantMap> reply = manager.call(QStringLiteral("GetProperties"));
    if (reply.isValid()) {
        handleState(reply.value().value(QStringLiteral("State")).toString());
    }
}

void ConnmanWatcher::onPropertyChanged(const QString &name, const QDBusVariant &value)
{
    if (name == QLatin1String("State")) {
        handleState(value.variant().toString());
    }
}

void ConnmanWatcher::handleState(const QString &state)
{
    if (state == m_state) {
        return;
    }
    m_state = state;

    qCInfo(loggerNetwork) << "ConnMan state changed to" << state;
    if (state == QLatin1String("online")) {
        // Introduce ourselves in the (possibly new) network.
        Daemon::instance()->forceOnNetworkChange();
    }
}

#include "networkwatcher.moc"
