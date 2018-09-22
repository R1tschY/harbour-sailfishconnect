#include "filehelper.h"

#include <QDir>

namespace SailfishConnect {

QFileInfo nonexistingFile(const QString& path) {
    QFileInfo result(path);
    const QString prefix = result.baseName();
    const QString suffix = result.completeSuffix();

    if (!prefix.isEmpty() && !suffix.isEmpty()) {
        int i = 0;
        while (result.exists()) {
            i += 1;
            result.setFile(
                result.dir(),
                QStringLiteral("%1 (%2).%3").arg(
                    prefix, QString::number(i), suffix));
        }
        return result.filePath();
    } else {
        QString name = prefix.isEmpty() ? suffix : prefix;
        int i = 0;
        while (result.exists()) {
            i += 1;
            result.setFile(
                result.dir(),
                QStringLiteral(".%1 (%2)").arg(name, QString::number(i)));
        }
        return result.filePath();
    }
}

} // namespace SailfishConnect
