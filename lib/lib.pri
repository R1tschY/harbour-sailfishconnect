QT += network dbus
CONFIG += link_pkgconfig
PKGCONFIG += openssl

INCLUDEPATH += $$PWD
LIBS += -L$$OUT_PWD/../lib -lsailfishconnect
PRE_TARGETDEPS += $$OUT_PWD/../lib/libsailfishconnect.a
