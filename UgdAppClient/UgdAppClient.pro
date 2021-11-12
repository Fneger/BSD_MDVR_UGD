QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MdvrUgdClient
CONFIG += c++11
RC_ICONS = UgdAppClient.ico


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TRANSLATIONS += $$PWD/translations/enl2chinese_sim.ts

SOURCES += \
    ../UgdServer/ZoomPipeline/smartlink/st_message.cpp \
    ../UgdServer/ZoomPipeline/smartlink/st_packet_tool.cpp \
    base-ui/CHintWidget.cpp \
    base-ui/ccalendardlg.cpp \
    base/myautomutex.cpp \
    base/mythread.cpp \
    ccommon.cpp \
    ceventsdlg.cpp \
    cloginfodlg.cpp \
    info/caddeventdlg.cpp \
    info/caddgroupfiledlg.cpp \
    info/caddproductdlg.cpp \
    info/cadduserdlg.cpp \
    info/caddversindlg.cpp \
    info/cadministratordlg.cpp \
    info/celidelabel.cpp \
    info/ceventitem.cpp \
    info/cgeneraluseraddproductdlg.cpp \
    info/cgroupfileitem.cpp \
    info/cmcuinfodlg.cpp \
    info/cproductitem.cpp \
    info/cproductversioninfo.cpp \
    info/cuseritem.cpp \
    info/cversioninfo.cpp \
    logger/st_logger.cpp \
    logindlg.cpp \
    main.cpp \
    mainwindow.cpp \
    network/cbdugdclientprotocol.cpp \
    network/ctcpclient.cpp \
    network/ctcpclienttest.cpp \
    network/qghtcpclient.cpp

HEADERS += \
    ../UgdServer/ZoomPipeline/smartlink/st_packet_tool.h \
    base-ui/CHintWidget.h \
    base-ui/ccalendardlg.h \
    base/myautomutex.h \
    base/mythread.h \
    ccommon.h \
    ceventsdlg.h \
    cloginfodlg.h \
    info/caddeventdlg.h \
    info/caddgroupfiledlg.h \
    info/caddproductdlg.h \
    info/cadduserdlg.h \
    info/caddversindlg.h \
    info/cadministratordlg.h \
    info/celidelabel.h \
    info/ceventitem.h \
    info/cgeneraluseraddproductdlg.h \
    info/cgroupfileitem.h \
    info/cmcuinfodlg.h \
    info/cproductitem.h \
    info/cproductversioninfo.h \
    info/cuseritem.h \
    info/cversioninfo.h \
    logger/st_logger.h \
    logindlg.h \
    mainwindow.h \
    network/cbdugdclientprotocol.h \
    network/ctcpclient.h \
    network/ctcpclienttest.h \
    network/qghtcpclient.h

FORMS += \
    base-ui/ccalendardlg.ui \
    ceventsdlg.ui \
    cloginfodlg.ui \
    info/caddeventdlg.ui \
    info/caddgroupfiledlg.ui \
    info/caddproductdlg.ui \
    info/cadduserdlg.ui \
    info/caddversindlg.ui \
    info/cadministratordlg.ui \
    info/ceventitem.ui \
    info/cgeneraluseraddproductdlg.ui \
    info/cgroupfileitem.ui \
    info/cmcuinfodlg.ui \
    info/cproductitem.ui \
    info/cproductversioninfo.ui \
    info/cuseritem.ui \
    info/cversioninfo.ui \
    logindlg.ui \
    mainwindow.ui

INCLUDEPATH += \
    $$PWD/../UgdServer/ZoomPipeline/network \
    $$PWD/../UgdServer/ZoomPipeline/smartlink \
    $$PWD/network \
    $$PWD/info \
    $$PWD/logger \
    $$PWD/base \
    $$PWD/base-ui

INCLUDEPATH += E:\boost_1_77_0
INCLUDEPATH += E:\boost_1_77_0\boost

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

DISTFILES +=
