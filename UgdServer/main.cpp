#include <QApplication>
#include "st_logger.h"
#include "zpmainframe.h"
#include <QSharedMemory>
#include <QMessageBox>



STMsgLogger::st_logger g_logger;

void stMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    g_logger.MessageOutput(type,context,msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSharedMemory shared("ugd_server_share");				//�󶨹����ڴ�
    if (shared.attach())						//����󶨳ɹ������ʾ�����ڴ��Ѵ���
    {
        QMessageBox::warning(NULL, QObject::tr("Warning"), QObject::tr("Program is running!"));
        return 0;
    }
    shared.create(1);
    a.setApplicationName("MdvrUgdServer");

//    Install message handler
    //qInstallMessageHandler(stMessageOutput);
    ZPMainFrame zpServer;
    zpServer.setWindowTitle(a.applicationName());
    g_logger.setLogLevel(3);
    zpServer.setLogger(&g_logger);
    zpServer.startStop();
    zpServer.show();

    return a.exec();
}
