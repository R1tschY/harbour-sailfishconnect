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

PKGCONFIG += sailfishapp contextkit-statefs nemonotifications-qt5
DEFINES += \
    QT_STATICPLUGIN \
    QT_DEPRECATED_WARNINGS \
    QT_DISABLE_DEPRECATED_BEFORE=0x050600 \
    QT_USE_QSTRINGBUILDER
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_DEBUG
include(../lib/lib.pri)
INCLUDEPATH += $$PWD/src

SOURCES += \
    src/appdaemon.cpp \
    src/ui/devicelistmodel.cpp \
    src/sailfishconnect.cpp \
    src/plugins/battery/batteryplugin.cpp \
    src/plugins/ping/pingplugin.cpp \
    src/plugins/clipboard/clipboardplugin.cpp \
    src/ui/devicepluginsmodel.cpp \
    src/ui.cpp \
    src/plugins/telepathy/telepathyplugin.cpp \
    src/plugins/mprisremote/mprisremoteplugin.cpp \
    src/ui/mprisplayersmodel.cpp \
    src/plugins/sendnotifications/notificationslistener.cpp \
    src/plugins/sendnotifications/notifyingapplication.cpp \
    src/plugins/sendnotifications/sendnotificationsplugin.cpp \
    src/plugins/touchpad/touchpadplugin.cpp \
    src/plugins/share/shareplugin.cpp \
    src/ui/jobsmodel.cpp \
    src/js/url.cpp \
    src/js/qmlregister.cpp \
    src/js/humanize.cpp \
    src/plugins/telephony/telephonyplugin.cpp \
    src/dbus/ofono.cpp \
    src/ui/sortfiltermodel.cpp

DISTFILES += \
    qml/cover/CoverPage.qml \
    qml/pages/DevicePage.qml \
    qml/pages/DeviceListPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/EncryptionInfoPage.qml \
    qml/components/MprisUi.qml \
    rpm/harbour-sailfishconnect.yaml \
    rpm/harbour-sailfishconnect.spec \
    qml/harbour-sailfishconnect.qml \
    harbour-sailfishconnect.desktop \
    src/plugins/battery/metadata.json \
    src/plugins/ping/metadata.json \
    qml/components/ClipboardUi.qml \
    qml/pages/SettingsPage.qml \
    rpm/harbour-sailfishconnect.changes \
    src/plugins/sendnotifications/metadata.json \
    src/plugins/touchpad/metadata.json \
    qml/components/Touchpad.qml \
    qml/components/TouchpadPage.qml \
    src/plugins/share/metadata.json \
    qml/components/ShareUi.qml \
    qml/pages/TransfersPage.qml \
    src/plugins/telephony/metadata.json \
    src/dbus/ofono/org.ofono.Manager.xml

SAILFISHAPP_ICONS = 86x86 108x108 128x128

HEADERS += \
    src/appdaemon.h \
    src/ui/devicelistmodel.h \
    src/plugins/battery/batteryplugin.h \
    src/plugins/ping/pingplugin.h \
    src/plugins/clipboard/clipboardplugin.h \
    src/ui/devicepluginsmodel.h \
    src/ui.h \
    src/sailfishconnect.h \
    src/plugins/telepathy/telepathyplugin.h \
    src/plugins/mprisremote/mprisremoteplugin.h \
    src/ui/mprisplayersmodel.h \
    src/plugins/sendnotifications/notificationslistener.h \
    src/plugins/sendnotifications/notifyingapplication.h \
    src/plugins/sendnotifications/sendnotificationsplugin.h \
    src/plugins/touchpad/touchpadplugin.h \
    src/plugins/share/shareplugin.h \
    src/ui/jobsmodel.h \
    src/js/url.h \
    src/js/qmlregister.h \
    src/js/humanize.h \
    src/plugins/telephony/telephonyplugin.h \
    src/dbus/tuple.h \
    src/dbus/ofono.h \
    src/ui/sortfiltermodel.h
