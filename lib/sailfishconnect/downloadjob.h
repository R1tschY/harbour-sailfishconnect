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

#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include <QString>
#include <sailfishconnect/io/copyjob.h>


namespace SailfishConnect {

class DownloadJob : public CopyJob
{
    Q_OBJECT
public:
    DownloadJob(
            const QString &deviceId,
            const QSharedPointer<QIODevice>& origin,
            const QString &destination,
            qint64 size,
            QObject* parent = nullptr);

    QString destination() const;

protected:
    void doStart() override;
    bool doCancelling() override;
    void onFinished() override;
    void onError() override;

private:
    QString m_destination;
};

} // namespace SailfishConnect

#endif // DOWNLOADJOB_H
