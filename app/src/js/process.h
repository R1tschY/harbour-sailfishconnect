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

#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>
#include <QProcess>

namespace QmlJs {

/// a QML wrapper for QProcess
class Process : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString program READ program WRITE setProgram)
    Q_PROPERTY(QString workingDirectory
               READ workingDirectory WRITE setWorkingDirectory)
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments)

    Q_PROPERTY(qint64 processId READ processId)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int exitCode READ exitCode)
    Q_PROPERTY(QString exitStatus READ exitStatus)
    Q_PROPERTY(bool normalExit READ normalExit)
    Q_PROPERTY(QString error READ error NOTIFY errorOccurred)

public:
    explicit Process(QObject *parent = nullptr);

    // properties

    QStringList arguments() const { return m_inner.arguments(); }
    void setArguments(const QStringList &arguments)
    { return m_inner.setArguments(arguments); }

    QString program() { return m_inner.program(); }
    void setProgram(const QString& program) { m_inner.setProgram(program); }

    QString state() const;
    qint64 processId() const { return m_inner.processId(); }
    int exitCode() const { return m_inner.exitCode(); }
    QString exitStatus() const;
    bool normalExit() const
    { return m_inner.exitStatus() == QProcess::NormalExit; }
    QString error() const;

    QString workingDirectory() const { return m_inner.workingDirectory(); }
    void setWorkingDirectory(const QString &workingDirectory)
    { return m_inner.setWorkingDirectory(workingDirectory); }

    // actions
    Q_SCRIPTABLE void start() { m_inner.start(); }
    Q_SCRIPTABLE void start(const QString &program, const QStringList &arguments)
    { m_inner.start(program, arguments); }
    Q_SCRIPTABLE bool waitForStarted(int msecs = 30000)
    { return m_inner.waitForStarted(msecs); }
    Q_SCRIPTABLE bool waitForFinished(int msecs = 30000)
    { return m_inner.waitForFinished(msecs); }
    Q_SCRIPTABLE void terminate() { m_inner.terminate(); }

    static void registerType();

signals:
    void errorOccurred();
    void finished(int exitCode);
    void started();
    void stateChanged();

private:
    QProcess m_inner;
};

} // namespace QmlJs

#endif // PROCESS_H
