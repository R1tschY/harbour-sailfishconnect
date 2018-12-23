#include "filehelper.h"

#include <QDir>
#include <QDebug>
#include <QRegularExpression>

namespace SailfishConnect {

QFileInfo nonexistingFile(const QString& path) {
    QFileInfo result(path);
    const QString prefix = result.baseName();
    QString suffix = result.completeSuffix();
    if (path.contains(QChar('.'))) {
        suffix.insert(0, QChar('.'));
    }

    if (!prefix.isEmpty()) {
        int i = 0;
        while (result.exists()) {
            i += 1;
            result.setFile(
                result.dir(),
                QStringLiteral("%1 (%2)%3").arg(
                    prefix, QString::number(i), suffix));
        }
        return result;
    } else {
        int i = 0;
        while (result.exists()) {
            i += 1;
            result.setFile(
                result.dir(),
                QStringLiteral("%1 (%2)").arg(suffix, QString::number(i)));
        }
        return result;
    }
}

QString escapeForFilePath(const QString& name)
{
    QString result = name.trimmed();
    result.replace(QChar('/'), QString());
    result.replace(QChar('\0'), QString());

    if (result.isEmpty()
            || result == QLatin1String(".")
            || result == QLatin1String("..")) {
        return QStringLiteral("_");
    } else {
        return result;
    }
}

} // namespace SailfishConnect
