#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "logindlg.h"
#include <ccommon.h>
#include <QDebug>
#include <QSharedMemory>
#include <QMessageBox>
#include <QLocale>
#include <QTranslator>
#include "st_logger.h"


STMsgLogger::st_logger g_logger;

void stMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    g_logger.MessageOutput(type,context,msg);
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /*获取当前语种*/
    QLocale local = QLocale::system();
    QLocale::Language lang = local.language();
    QTranslator translator;
    if(lang == QLocale::Chinese)
    {
        bool res = false;

        res = translator.load(qApp->applicationDirPath() + "/translations/enl2chinese_sim.qm");
        if(res)
            qApp->installTranslator(&translator);
    }

    QSharedMemory shared("ugd_client_share");				//绑定共享内存
    if (shared.attach())						//如果绑定成功，则表示共享内存已存在
    {
        QMessageBox::warning(NULL, QObject::tr("Warning"), QObject::tr("Program is running!"));
        return 0;
    }
    shared.create(1);

    a.setApplicationName("MdvrUgdClient");
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();

    qInstallMessageHandler(stMessageOutput);
    g_logger.setLogLevel(3);

    MainWindow w;
    w.setWindowTitle(a.applicationName());
    LoginDlg loginDlg(w.tcpClient());
    loginDlg.setWindowTitle(a.applicationName());
    if(loginDlg.exec() == QDialog::Accepted)
    {
        w.refreshProductInfos();
        if(screenRect.height() <= 768)
            w.showMaximized();
        else
            w.show();
    }
    else
    {
        return 0;
    }
    return a.exec();
}
