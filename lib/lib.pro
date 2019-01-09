TARGET = sailfishconnect
TEMPLATE = lib

CONFIG += staticlib c++14
QT += network dbus

PKGCONFIG += openssl
DEFINES += \
    QT_DEPRECATED_WARNINGS \
    QT_DISABLE_DEPRECATED_BEFORE=0x050600 \
    QT_USE_QSTRINGBUILDER
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
INCLUDEPATH += $PWD

SOURCES += \
    sailfishconnect/backend/lan/landevicelink.cpp \
    sailfishconnect/backend/lan/lanlinkprovider.cpp \
    sailfishconnect/backend/lan/lanpairinghandler.cpp \
    sailfishconnect/backend/lan/socketlinereader.cpp \
    sailfishconnect/backend/devicelinereader.cpp \
    sailfishconnect/backend/devicelink.cpp \
    sailfishconnect/backend/linkprovider.cpp \
    sailfishconnect/backend/pairinghandler.cpp \
    sailfishconnect/device.cpp \
    sailfishconnect/kdeconnectconfig.cpp \
    sailfishconnect/kdeconnectplugin.cpp \
    sailfishconnect/kdeconnectpluginconfig.cpp \
    sailfishconnect/corelogging.cpp \
    sailfishconnect/backend/lan/server.cpp \
    sailfishconnect/daemon.cpp \
    sailfishconnect/helper/sslhelper.cpp \
    sailfishconnect/pluginloader.cpp \
    sailfishconnect/systeminfo.cpp \
    sailfishconnect/io/job.cpp \
    sailfishconnect/helper/filehelper.cpp \
    sailfishconnect/io/copyjob.cpp \
    sailfishconnect/downloadjob.cpp \
    sailfishconnect/backend/lan/landownloadjob.cpp \
    sailfishconnect/backend/lan/lanuploadjob.cpp \
    sailfishconnect/backend/lan/lannetworklistener.cpp \
    sailfishconnect/io/jobmanager.cpp \
    sailfishconnect/networkpacket.cpp \
    sailfishconnect/helper/humanize.cpp


# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
# DISTFILES += translations/*.ts
# TRANSLATIONS += translations/harbour-sailfishconnect-de.ts
# TRANSLATIONS += translations/harbour-sailfishconnect-ru.ts
# TRANSLATIONS += translations/harbour-sailfishconnect-es.ts

HEADERS += \
    sailfishconnect/backend/lan/landevicelink.h \
    sailfishconnect/backend/lan/lanlinkprovider.h \
    sailfishconnect/backend/lan/lanpairinghandler.h \
    sailfishconnect/backend/lan/socketlinereader.h \
    sailfishconnect/backend/devicelinereader.h \
    sailfishconnect/backend/devicelink.h \
    sailfishconnect/backend/linkprovider.h \
    sailfishconnect/backend/pairinghandler.h \
    sailfishconnect/device.h \
    sailfishconnect/kdeconnectconfig.h \
    sailfishconnect/kdeconnectplugin.h \
    sailfishconnect/kdeconnectpluginconfig.h \
    sailfishconnect/corelogging.h \
    sailfishconnect/backend/lan/server.h \
    sailfishconnect/daemon.h \
    sailfishconnect/helper/sslhelper.h \
    sailfishconnect/pluginloader.h \
    sailfishconnect/systeminfo.h \
    sailfishconnect/helper/cpphelper.h \
    sailfishconnect/io/job.h \
    sailfishconnect/helper/filehelper.h \
    sailfishconnect/io/copyjob.h \
    sailfishconnect/downloadjob.h \
    sailfishconnect/backend/lan/landownloadjob.h \
    sailfishconnect/backend/lan/lanuploadjob.h \
    sailfishconnect/backend/lan/lannetworklistener.h \
    sailfishconnect/io/jobmanager.h \
    sailfishconnect/networkpacket.h \
    sailfishconnect/networkpackettypes.h \
    sailfishconnect/helper/humanize.h

DISTFILES += \
    lib.pri
