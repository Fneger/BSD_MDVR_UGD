#include "ctcpclienttest.h"
#include "ccommon.h"
#include "st_message.h"
#include "qghtcpclient.h"
#include "ctcpclient.h"
#include <QMessageBox>
#include <QTimer>
#include "st_global_def.h"

using namespace GsGlobalDef;
CTcpClientTest::CTcpClientTest(quint64 nStartUUID, quint64 nEndUUID, QObject *parent) : CMyThread(parent)
{
    m_minUUID = nStartUUID;
    m_maxUUID = nEndUUID;
    m_currNewUUID = nStartUUID;
}

CTcpClientTest::~CTcpClientTest()
{
    startStopTest(false);
}

void CTcpClientTest::startStopTest(bool bIsStart)
{
    if (bIsStart==true)
    {
        this->startRun();
    }
    else
    {
        this->stopRun();
    }
}

void CTcpClientTest::run()
{
    int nTotalClients = 640;
    while (!getQuitFlag()) {
        QList<CTcpClient*> listObj = m_clients.values();

        foreach(CTcpClient * client,listObj)
        {
            if (rand()%100<50)
            {
                if(!client->isStartTest())
                    client->startTest();
                if(client->isStartTest())
                {
                    if(!client->sendTestPacket(rand()%2048))
                    {
                        client->stopTest();
                        qWarning() << QString("client %1 test error!").arg(client->uuid(),16,16,QChar('0'));
                    }
                    if(CCommon::Instance()->AddLog2Db(client,"ZH510", QString::number(rand() % 1024), "000000", "DVR-T21110194-ZH510.6550-ENG-BD.crc",
                                                      LOG_DEV_MODULE_E, DEV_MODULE_4G_E, DEV_MODULE_ABNORMAL_E,
                                                      "Dialing failed!"))
                    {
                        client->stopTest();
                        qWarning() << QString("client %1 test add log!").arg(client->uuid(),16,16,QChar('0'));
                    }
                }
            }
        }
        //
        if (rand()%500 <100)
            //if (m_clients.size() < 1)
        {
            //1/10 chance to make new connections.
            if (m_clients.size()>nTotalClients)
            {
                int nDel = m_clients.size()-nTotalClients;
                QList<quint64> listObj = m_clients.keys();
                for (int i=0;i<nDel;i++)
                {
                    CTcpClient * client = m_clients[listObj.at(i)];
                    m_clients.remove(listObj.at(i));
                    //client->stopClient();
                    client->deleteLater();
                }
            }
            QString serverIp = Settings.value("ServerSettings/sServerIp","127.0.0.1").toString();
            uint16_t serverPort = Settings.value("ServerSettings/nServerPort","25600").toString().toUInt();
            CTcpClient * client = new CTcpClient();
            m_currNewUUID++;
            if(m_currNewUUID > m_maxUUID)
                m_currNewUUID = m_minUUID;
            client->setUuid(m_currNewUUID);
            client->setServerInfo(serverIp,serverPort);
            m_clients[m_currNewUUID] = client;
            client->startClient();
        }
        msleep(500);
    }
    QList<CTcpClient*> listObj = m_clients.values();
    foreach(CTcpClient * client,listObj)
    {
        client->stopClient();
        client->deleteLater();
    }
    m_clients.clear();
}
