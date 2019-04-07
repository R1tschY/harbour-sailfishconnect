include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11 thread testcase no_testcase_installs
CONFIG -= app_bundle
QT += testlib

CONFIG += conan_basic_setup
include(../conanbuildinfo.pri)

include(../lib/lib.pri)

HEADERS += \
    test.h \
    mock_devicelink.h \
    mock_linkprovider.h \
    mock_pairinghandler.h \
    mock_job.h

SOURCES += main.cpp \
    test_filehelper.cpp \
    test_networkpacket.cpp \
    test_connectconfig.cpp \
    test_device.cpp \
    mock_devicelink.cpp \
    mock_linkprovider.cpp \
    mock_pairinghandler.cpp \
    test.cpp \
    test_job.cpp \
    mock_job.cpp \
    test_humanize.cpp \
    test_functools.cpp
