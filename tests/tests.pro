include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11 thread testcase
CONFIG -= app_bundle

include(../lib/lib.pri)

HEADERS += \
    test.h \
    mock_devicelink.h \
    mock_linkprovider.h

SOURCES += main.cpp \
    test_filehelper.cpp \
    test_networkpacket.cpp \
    test_connectconfig.cpp \
    test_device.cpp \
    mock_devicelink.cpp \
    mock_linkprovider.cpp
