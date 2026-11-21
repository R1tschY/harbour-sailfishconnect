/*
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <QDBusVariant>
#include <QObject>
#include <QString>

/// Announces the device again when ConnMan brings the network online.
///
/// Qt 5.6's bearer management does not report network changes on Sailfish OS,
/// so without this the phone never re-introduces itself after Wi-Fi comes up
/// and the desktop only sees it after a manual refresh.
class ConnmanWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ConnmanWatcher(QObject *parent = nullptr);

private slots:
    void onPropertyChanged(const QString &name, const QDBusVariant &value);
    void handleState(const QString &state);

private:
    QString m_state;
};
