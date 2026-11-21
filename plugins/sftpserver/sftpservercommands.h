/*
 * Copyright 2021 Richard Liebscher <richard.liebscher@gmail.com>.
 * Copyright 2026 Manuel FLURY <zx97@users.noreply.github.com>.
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

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QString>

#include <libssh/sftp.h>

#include <memory>
#include <unordered_map>

#include "ssh.h"

/// Serves SFTP requests of a single client session.
///
/// Takes ownership of the libssh sftp_session. All paths are confined to
/// rootPath.
class SftpServerSession
{
public:
    SftpServerSession(sftp_session sftp, const QString &rootPath);

    int init();
    bool readMessage();

private:
    QString getFilename(const char *rawFilename);
    QString getFilename(sftp_client_message msg);
    QString getDataFilename(sftp_client_message msg);
    QString resolvePath(const QString &rawFilename) const;
    bool isInsideRoot(const QString &path) const;

    sftp_attributes_struct getFileAttrs(const QFileInfo &fileInfo);

    bool replyReadLink(sftp_client_message msg);
    bool replyRealPath(sftp_client_message msg);
    bool replyStat(sftp_client_message msg, bool link);
    bool replySetStat(sftp_client_message msg, bool follow);
    bool replyFStat(sftp_client_message msg);
    bool replyFSetStat(sftp_client_message msg);
    bool replyExtended(sftp_client_message msg);

    bool replyOpenDir(sftp_client_message msg);
    bool replyReadDir(sftp_client_message msg);
    bool replyClose(sftp_client_message msg);

    bool replyOpenFile(sftp_client_message msg);
    bool replyReadFile(sftp_client_message msg);
    bool replyWriteFile(sftp_client_message msg);

    bool replyRemove(sftp_client_message msg);
    bool replyMakeDir(sftp_client_message msg);
    bool replyRemoveDir(sftp_client_message msg);
    bool replyRename(sftp_client_message msg);
    bool replySymlink(sftp_client_message msg);

    SftpSessionPtr m_sftpSession;
    QString m_rootPath;
    QString m_rootCanonicalPath;

    std::unordered_map<void *, std::unique_ptr<QDirIterator>> m_openDirHandles;
    std::unordered_map<void *, std::unique_ptr<QFile>> m_openFileHandles;
};
