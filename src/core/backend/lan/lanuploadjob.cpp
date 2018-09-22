/*
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lanuploadjob.h"

#include "lanlinkprovider.h"
#include "../../kdeconnectconfig.h"
#include "../../corelogging.h"

using namespace SailfishConnect;

LanUploadJob::LanUploadJob(const QSharedPointer<QIODevice>& source, const QString& deviceId)
    : Job()
    , m_input(source)
    , m_server(new Server(this))
    , m_socket(nullptr)
    , m_port(0)
    , m_deviceId(deviceId) // We will use this info if link is on ssl, to send encrypted payload
{
    connect(m_input.data(), &QIODevice::readyRead, this, &LanUploadJob::startUploading);
    connect(m_input.data(), &QIODevice::aboutToClose, this, &LanUploadJob::aboutToClose);
}

void LanUploadJob::doStart()
{
    m_port = MIN_PORT;
    while (!m_server->listen(QHostAddress::Any, m_port)) {
        m_port++;
        if (m_port > MAX_PORT) { //No ports available?
            qCWarning(coreLogger) << "Error opening a port in range" << MIN_PORT << "-" << MAX_PORT;
            m_port = 0;
            setErrorString(tr("Couldn't find an available port"));
            exit();
            return;
        }
    }
    connect(m_server, &QTcpServer::newConnection, this, &LanUploadJob::newConnection);
}

void LanUploadJob::newConnection()
{
    if (!m_input->open(QIODevice::ReadOnly)) {
        qCWarning(coreLogger) << "error when opening the input to upload";
        setErrorString(m_input->errorString());
        cleanup();
        return;
    }

    Server* server = qobject_cast<Server*>(sender());
    // FIXME : It is called again when payload sending is finished. Unsolved mystery :(
    disconnect(m_server, &QTcpServer::newConnection, this, &LanUploadJob::newConnection);

    m_socket = server->nextPendingConnection();
    m_socket->setParent(this);
    connect(m_socket, &QSslSocket::disconnected, this, &LanUploadJob::cleanup);
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketFailed(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    connect(m_socket, &QSslSocket::encrypted, this, &LanUploadJob::startUploading);
//     connect(mSocket, &QAbstractSocket::stateChanged, [](QAbstractSocket::SocketState state){ qDebug() << "statechange" << state; });

    LanLinkProvider::configureSslSocket(m_socket, m_deviceId, true);

    m_socket->startServerEncryption();
}

void LanUploadJob::startUploading()
{
    // TODO: make async
    while ( m_input->bytesAvailable() > 0 )
    {
        qint64 bytes = qMin(m_input->bytesAvailable(), (qint64)4096);
        int w = m_socket->write(m_input->read(bytes));
        if (w<0) {
            qCWarning(coreLogger) << "error when writing data to upload" << bytes << m_input->bytesAvailable();
            break;
        }
        else
        {
            while ( m_socket->flush() );
        }
    }
    m_input->close();
}

void LanUploadJob::aboutToClose()
{
    qDebug() << "closing...";
    m_socket->disconnectFromHost();
}

void LanUploadJob::cleanup()
{
    m_socket->close();
    qDebug() << "closed!";
    exit();
}

QVariantMap LanUploadJob::transferInfo()
{
    Q_ASSERT(m_port != 0);
    return {{"port", m_port}};
}

void LanUploadJob::socketFailed(QAbstractSocket::SocketError error)
{
    qWarning() << "error uploading" << error;
    setErrorString(m_socket->errorString());
    m_socket->close();
    exit();
}

void LanUploadJob::sslErrors(const QList<QSslError>& errors)
{
    qWarning() << "ssl errors" << errors;

    setErrorString(errors.first().errorString());  // TODO

    m_socket->close();
    exit();
}
