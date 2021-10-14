INCLUDEPATH += $$PWD/src/include_asio/
INCLUDEPATH += $$PWD/src/

win32:win32-g++:LIBS += -lMswsock
win32:LIBS += -lws2_32

DEFINES += ASIO_STANDALONE

CONFIG += c++11

HEADERS += \
    $$PWD/src/ioserviceclass.h \
    $$PWD/src/linstenclass.hpp \
    $$PWD/src/qasiosocket.h \
    $$PWD/src/qasiosslserver.h \
    $$PWD/src/qasiosslsocket.h \
    $$PWD/src/qasiotcpserver.h \
    $$PWD/src/qasiotcpsocket.h \
    $$PWD/src/qsocketconnection.h \ \
    $$PWD/src/tcpabstractsocket.h

SOURCES += \
    $$PWD/src/qasiotcpsocket.cpp \
    $$PWD/src/qasiotcpserver.cpp \
    $$PWD/src/qsocketconnection.cpp \
    $$PWD/src/qasiosslsocket.cpp \
    $$PWD/src/qasiosslserver.cpp
