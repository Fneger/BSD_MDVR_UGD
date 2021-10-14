#-------------------------------------------------
#
# Project created by QtCreator 2015-07-30T15:30:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = msserver
TEMPLATE = app
CONFIG   += console

CONFIG += C++11

SOURCES += main.cpp\
        mainwindow.cpp \
    testserver.cpp

HEADERS  += mainwindow.h \
    testserver.h

FORMS    += mainwindow.ui


INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

win32: LIBS += -L$$PWD/../../lib/ -lQAsioSocket

INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../lib/QAsioSocket.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../lib/libQAsioSocket.a
