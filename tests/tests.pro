include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread

INCLUDEPATH += ../lib
LIBS += -L../lib -lsailfishconnect
PRE_TARGETDEPS += $$OUT_PWD/../lib/libsailfishconnect.a

HEADERS += \
    test.h

SOURCES += main.cpp \
    test_filehelper.cpp
