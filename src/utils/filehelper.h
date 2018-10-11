#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QString>
#include <QFileInfo>

namespace SailfishConnect {

QFileInfo nonexistingFile(const QString& path);
QString escapeForFilePath(const QString& name);

} // namespace SailfishConnect

#endif // FILEHELPER_H
