#include <QApplication>
#include "st_logger.h"
#include "zpmainframe.h"

STMsgLogger::st_logger g_logger;

void stMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    g_logger.MessageOutput(type,context,msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("MdvrUgdServer");
//    Install message handler
    qInstallMessageHandler(stMessageOutput);
    ZPMainFrame zpServer;
    zpServer.setWindowTitle(a.applicationName());
    g_logger.setLogLevel(3);
    zpServer.setLogger(&g_logger);
    zpServer.startStop();
    zpServer.show();

    return a.exec();
}
