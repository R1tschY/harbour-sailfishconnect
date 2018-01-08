#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <memory>
#include <QLoggingCategory>
#include <QQmlEngine>
#include <sailfishapp.h>

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include "appdaemon.h"
#include "ui/devicelistmodel.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

void logBacktrace()
{
    void* array[32];
    size_t size = backtrace (array, 32);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = qFormatLogMessage(type, context, msg).toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        fprintf(stderr, "%s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s\n", localMsg.constData());
        logBacktrace();
        abort();
    }
}

void registerQmlTypes() {
    qmlRegisterType<DeviceListModel>("SailfishConnect", 0, 1, "DeviceListModel");
}

} // SailfishConnect

int main(int argc, char *argv[])
{  
    using namespace SailfishConnect;

    qInstallMessageHandler(myMessageOutput);

    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    std::unique_ptr<QQuickView> view(SailfishApp::createView());

    registerQmlTypes();
    AppDaemon daemon;

    view->setSource(SailfishApp::pathToMainQml());
    view->showFullScreen();

    return app->exec();
}
