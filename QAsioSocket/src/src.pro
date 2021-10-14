QT       += core

TEMPLATE = lib
DEFINES += QASIOSOCKET_LIBRARY
#DEFINES += QASIO_SSL
DESTDIR   = $$PWD/../lib
DEFINES += _WIN32_WINNT=0x0501

TARGET  = $$qtLibraryTarget(QAsioSocket)

include($$PWD/../QAsioSocket.pri)



win32: LIBS += -L$$PWD/openssl/lib/ -llibssl

INCLUDEPATH += $$PWD/openssl
DEPENDPATH += $$PWD/openssl
