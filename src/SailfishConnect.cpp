#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <memory>
#include <QLoggingCategory>
#include <sailfishapp.h>

#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include "core/daemon.h"

namespace SailfishConnect {

static Q_LOGGING_CATEGORY(logger, "sailfishconnect.ui")

class SailfishConnectDaemon : public Daemon
{
public:
    SailfishConnectDaemon(QObject* parent = Q_NULLPTR) : Daemon(parent) {}

    void askPairingConfirmation(Device* device) override
    {
       qCWarning(logger) << "askPairingConfirmation";
//        KNotification* notification = new KNotification(QStringLiteral("pairingRequest"));
//        notification->setIconName(QStringLiteral("dialog-information"));
//        notification->setComponentName(QStringLiteral("kdeconnect"));
//        notification->setText(i18n("Pairing request from %1", device->name().toHtmlEscaped()));
//        notification->setActions(QStringList() << i18n("Accept") << i18n("Reject"));
////         notification->setTimeout(PairingHandler::pairingTimeoutMsec());
//        connect(notification, &KNotification::action1Activated, device, &Device::acceptPairing);
//        connect(notification, &KNotification::action2Activated, device, &Device::rejectPairing);
//        notification->sendEvent();
    }

    void reportError(const QString & title, const QString & description) override
    {
       qCCritical(logger) << title << description;
    }
};

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

} // SailfishConnect

int main(int argc, char *argv[])
{  
    using namespace SailfishConnect;

    qInstallMessageHandler(myMessageOutput);

    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    std::unique_ptr<QQuickView> view(SailfishApp::createView());

    view->setSource(SailfishApp::pathToMainQml());
    view->showFullScreen();

    SailfishConnectDaemon daemon;
    return app->exec();
}
