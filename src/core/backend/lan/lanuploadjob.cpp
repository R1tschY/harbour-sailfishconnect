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

namespace SailfishConnect {

LanUploadJob::LanUploadJob(
        const QSharedPointer<QIODevice>& source, const QString& deviceId)
    : CopyJob(source, QSharedPointer<QIODevice>())
    , m_input(source)
    , m_server(new Server(this))
    , m_socket(nullptr)
    , m_port(0)
    // We will use this info if link is on ssl, to send encrypted payload
    , m_deviceId(deviceId)
{
    connect(m_input.data(), &QIODevice::readyRead,
            this, &LanUploadJob::startUploading);
    connect(m_input.data(), &QIODevice::aboutToClose,
            this, &LanUploadJob::aboutToClose);
}

LanUploadJob::~LanUploadJob()
{
    qDebug(coreLogger) << "~LanUploadJob";
}

void LanUploadJob::doStart()
{
    m_port = MIN_PORT;
    while (!m_server->listen(QHostAddress::Any, m_port)) {
        m_port++;
        if (m_port > MAX_PORT) {
            // No ports available?
            qCWarning(coreLogger)
                    << "Error opening a port in range"
                    << MIN_PORT << "-" << MAX_PORT;
            m_port = 0;
            return abort(tr("Couldn't find an available port"));
        }
    }
    connect(m_server, &QTcpServer::newConnection,
            this, &LanUploadJob::newConnection);
}

void LanUploadJob::onFinished()
{
    CopyJob::onFinished();
    close();
}

void LanUploadJob::newConnection()
{
    qCDebug(coreLogger) << "connection for payload upload";
    if (!m_input->open(QIODevice::ReadOnly)) {
        qCWarning(coreLogger) << "error when opening the input to upload";
        abort(m_input->errorString());
        return;
    }

    Server* server = qobject_cast<Server*>(sender());
    // FIXME : It is called again when payload sending is finished. Unsolved mystery :(
    disconnect(m_server, &QTcpServer::newConnection,
               this, &LanUploadJob::newConnection);

    m_socket = QSharedPointer<QSslSocket>(server->nextPendingConnection());
    setDestination(m_socket);

    connect(m_socket.data(), &QSslSocket::disconnected,
            this, &LanUploadJob::tearDown);
    connect(m_socket.data(), SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketFailed(QAbstractSocket::SocketError)));
    connect(m_socket.data(), SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(sslErrors(QList<QSslError>)));
    connect(m_socket.data(), &QSslSocket::encrypted,
            this, &LanUploadJob::startUploading);
//     connect(mSocket, &QAbstractSocket::stateChanged,
//             this, [](QAbstractSocket::SocketState state){
//                  qDebug() << "statechange" << state; });

    LanLinkProvider::configureSslSocket(m_socket.data(), m_deviceId, true);
    m_socket->startServerEncryption();
}

void LanUploadJob::startUploading()
{
    qCDebug(coreLogger) << "start payload upload";
    CopyJob::doStart();
}

void LanUploadJob::aboutToClose()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

void LanUploadJob::tearDown()
{
    close();
    exit();
}

QVariantMap LanUploadJob::transferInfo()
{
    Q_ASSERT(m_port != 0);
    return {{"port", m_port}};
}

void LanUploadJob::socketFailed(QAbstractSocket::SocketError error)
{
    if (m_socket) {
        qWarning() << "error uploading" << error;
        abort(m_socket->errorString());
    }
}

void LanUploadJob::sslErrors(const QList<QSslError>& errors)
{
    qWarning() << "ssl errors" << errors;

    abort(errors.first().errorString());  // TODO
}

} // namespace SailfishConnect
