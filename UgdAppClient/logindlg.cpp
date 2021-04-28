#include "logindlg.h"
#include "ui_logindlg.h"
#include "ccommon.h"
#include "ctcpclient.h"
#include <QElapsedTimer>
#include <QThread>
#include <assert.h>

LoginDlg::LoginDlg(CTcpClient *pTcpClient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg),
    m_tcpClient(pTcpClient),
    m_bImmediatelyCancel(false)
{
    assert(m_tcpClient != nullptr);
    ui->setupUi(this);
    ui->ServerIpLineEdit->setText(Settings.value("ServerSettings/sServerIp","127.0.0.1").toString());
    ui->ServerPortLineEdit->setText(Settings.value("ServerSettings/nServerPort","25600").toString());
    ui->UserNameLineEdit->setText(Settings.value("ServerSettings/sUserName","").toString());
    ui->PasswdLineEdit->setText(Settings.value("ServerSettings/sPasswd","").toString());
    this->setWindowTitle(tr("MdvrUgdPcClient"));
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

void LoginDlg::closeEvent(QCloseEvent *)
{
    m_bImmediatelyCancel = true;
}

void LoginDlg::on_CancelBtn_clicked()
{
    m_tcpClient->stopClient();
    emit reject();
    close();
}

void LoginDlg::on_LoginBtn_clicked()
{
    QString serverIp = ui->ServerIpLineEdit->text();
    qint16 serverPort = ui->ServerPortLineEdit->text().toShort();
    QString userName = ui->UserNameLineEdit->text();
    QString password = ui->PasswdLineEdit->text();
    m_tcpClient->setServerInfo(serverIp,serverPort,userName,password);
    m_tcpClient->startClient();
    QElapsedTimer timer;
    timer.start();
    int dotCount = 1;
    QString dotStr;
    qint64 lastElapsed = timer.elapsed();
    qint64 currElapsed = timer.elapsed();
    ui->LoginBtn->setEnabled(false);
    while (1) {
        if(m_bImmediatelyCancel)
            return;
        currElapsed = timer.elapsed();
        if(currElapsed % 5 == 0)
            qApp->processEvents();
        if(currElapsed > 10000)
        {
            if(!m_tcpClient->isConnected())
                ui->TipsLabel->setText(tr("Connection failure!"));
            else if(m_tcpClient->authLevel() < 0)
                ui->TipsLabel->setText(tr("Failed to login!"));
            break;
        }
        if(currElapsed % 1000 == 0 && lastElapsed != currElapsed)
        {
            lastElapsed = currElapsed;
            dotStr.clear();
            for(int i=0; i<dotCount; i++)
                dotStr += ".";
            if(!m_tcpClient->isConnected())
                ui->TipsLabel->setText(tr("Connecting to server") + dotStr);
            else if(m_tcpClient->authLevel() < 0)
                ui->TipsLabel->setText(tr("Logging in to server") + dotStr);

            dotCount++;
            if(dotCount > 6)
                dotCount = 1;
        }
        if(m_tcpClient->authLevel() >= 0)
            break;

    }
    ui->LoginBtn->setEnabled(true);
    if(m_tcpClient->authLevel() >= 0)
    {
        Settings.setValue("ServerSettings/sServerIp",serverIp);
        Settings.setValue("ServerSettings/nServerPort",serverPort);
        Settings.setValue("ServerSettings/sUserName",userName);
        Settings.setValue("ServerSettings/sPasswd",password);
        emit accept();
        close();
    }
}
