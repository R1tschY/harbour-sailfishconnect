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

#include "process.h"

#include <QtQml>

#include <sailfishconnect/helper/cpphelper.h>

namespace QmlJs {

using namespace SailfishConnect;

Process::Process(QObject *parent) : QObject(parent)
{
    connect(&m_inner, &QProcess::errorOccurred, this, &Process::errorOccurred);
    connect(&m_inner, Overload<int>::of(&QProcess::finished),
            this, &Process::finished);
    connect(&m_inner, &QProcess::started, this, &Process::started);
    connect(&m_inner, &QProcess::stateChanged, this, &Process::stateChanged);
}

QString Process::state() const
{
    switch (m_inner.state()) {
    case QProcess::NotRunning:
        return QStringLiteral("NotRunning");
    case QProcess::Starting:
        return QStringLiteral("Starting");
    case QProcess::Running:
        return QStringLiteral("Running");
    default:
        return QString();
    }
}

QString Process::exitStatus() const
{
    switch (m_inner.exitStatus()) {
    case QProcess::NormalExit:
        return QStringLiteral("NormalExit");
    case QProcess::CrashExit:
        return QStringLiteral("CrashExit");
    default:
        return QString();
    }
}

QString Process::error() const
{
    switch (m_inner.error()) {
    case QProcess::FailedToStart:
        return QStringLiteral("FailedToStart");
    case QProcess::Crashed:
        return QStringLiteral("Crashed");
    case QProcess::Timedout:
        return QStringLiteral("Timedout");
    case QProcess::WriteError:
        return QStringLiteral("WriteError");
    case QProcess::ReadError:
        return QStringLiteral("ReadError");
    case QProcess::UnknownError:
        return QStringLiteral("UnknownError");
    default:
        return QString();
    }
}

void Process::registerType()
{
    qmlRegisterType<Process>("SailfishConnect.Qml", 0, 4, "Process");
}

} // namespace QmlJs
