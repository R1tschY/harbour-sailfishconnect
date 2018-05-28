# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-sailfishconnect

CONFIG += sailfishapp c++14
QT += network dbus
PKGCONFIG += openssl contextkit-statefs nemonotifications-qt5
DEFINES += QT_STATICPLUGIN
INCLUDEPATH += src

SOURCES += \
    src/core/backend/lan/landevicelink.cpp \
    src/core/backend/lan/lanlinkprovider.cpp \
    src/core/backend/lan/lanpairinghandler.cpp \
    src/core/backend/lan/socketlinereader.cpp \
    src/core/backend/devicelinereader.cpp \
    src/core/backend/devicelink.cpp \
    src/core/backend/linkprovider.cpp \
    src/core/backend/pairinghandler.cpp \
    src/core/device.cpp \
    src/core/kdeconnectconfig.cpp \
    src/core/kdeconnectplugin.cpp \
    src/core/kdeconnectpluginconfig.cpp \
    src/core/networkpackage.cpp \
    src/core/corelogging.cpp \
    src/core/backend/lan/server.cpp \
    src/core/daemon.cpp \
    src/utils/sslhelper.cpp \
    src/appdaemon.cpp \
    src/ui/devicelistmodel.cpp \
    src/core/backend/lan/downloadjob.cpp \
    src/core/backend/lan/uploadjob.cpp \
    src/sailfishconnect.cpp \
    src/core/pluginloader.cpp \
    src/plugins/battery/batteryplugin.cpp \
    src/plugins/ping/pingplugin.cpp \
    src/core/systeminfo.cpp \
    src/plugins/clipboard/clipboardplugin.cpp \
    src/ui/filtervalueproxymodel.cpp \
    src/ui/devicepluginsmodel.cpp \
    src/ui.cpp \
    src/plugins/telepathy/telepathyplugin.cpp \
    src/plugins/mprisremote/mprisremoteplugin.cpp \
    src/ui/mprisplayersmodel.cpp

DISTFILES += \
    qml/cover/CoverPage.qml \
    translations/*.ts \
    qml/pages/DevicePage.qml \
    qml/pages/DeviceListPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/EncryptionInfoPage.qml \
    rpm/harbour-sailfishconnect.yaml \
    rpm/harbour-sailfishconnect.spec \
    rpm/harbour-sailfishconnect.changes.in \
    rpm/harbour-sailfishconnect.changes.run.in \
    qml/harbour-sailfishconnect.qml \
    harbour-sailfishconnect.desktop \
    src/plugins/battery/metadata.json \
    src/plugins/ping/metadata.json \
    *.md

SAILFISHAPP_ICONS = 86x86 108x108 128x128

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-sailfishconnect-de.ts
TRANSLATIONS += translations/harbour-sailfishconnect-ru.ts

HEADERS += \
    src/core/backend/lan/landevicelink.h \
    src/core/backend/lan/lanlinkprovider.h \
    src/core/backend/lan/lanpairinghandler.h \
    src/core/backend/lan/socketlinereader.h \
    src/core/backend/devicelinereader.h \
    src/core/backend/devicelink.h \
    src/core/backend/linkprovider.h \
    src/core/backend/pairinghandler.h \
    src/core/device.h \
    src/core/kdeconnectconfig.h \
    src/core/kdeconnectplugin.h \
    src/core/kdeconnectpluginconfig.h \
    src/core/networkpackage.h \
    src/core/networkpackagetypes.h \
    src/core/corelogging.h \
    src/core/backend/lan/server.h \
    src/core/daemon.h \
    src/utils/sslhelper.h \
    src/appdaemon.h \
    src/ui/devicelistmodel.h \
    src/core/backend/lan/downloadjob.h \
    src/core/backend/lan/uploadjob.h \
    src/core/pluginloader.h \
    src/plugins/battery/batteryplugin.h \
    src/plugins/ping/pingplugin.h \
    src/core/systeminfo.h \
    src/utils/cpphelper.h \
    src/plugins/clipboard/clipboardplugin.h \
    src/ui/filtervalueproxymodel.h \
    src/ui/devicepluginsmodel.h \
    src/ui.h \
    src/sailfishconnect.h \
    src/plugins/telepathy/telepathyplugin.h \
    src/plugins/mprisremote/mprisremoteplugin.h \
    src/ui/mprisplayersmodel.h

SUBDIRS += \
    src/plugins/battery/SailfishConnectBatteryPlugin.pro
