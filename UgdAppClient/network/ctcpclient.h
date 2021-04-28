#ifndef CTCPCLIENT_H
#define CTCPCLIENT_H

#include <QObject>
#include "qghtcpclient.h"
#include "cbdugdclientprotocol.h"
#include <QJsonDocument>
#include <QMutex>

class QTcpSocket;
class QTimer;
class CTcpClient : public QObject, public CBdUgdClientProtocol
{
    Q_OBJECT
public:
    //等待Ack状态
    typedef enum WaitAckStatus{
        AckWaitting_E,  //等待中
        AckReceived_E,  //成功接收ACK
        AckParseError_E, //Ack解析错误
        AckWaittingTimeout_E, //等待Ack超时
        AckSendDataError_E,    //数据发送发生错误
    }WAIT_ACK_STATUS_E;
    //拥塞控制数据结构体
    struct congestion_control_para{
        qint64 min_rtt_ms; //最小传输延迟
        qint64 curr_rtt_ms;    //当前传输延迟
        qint64 last_rtt_ms;    //上一次传输延迟
        quint32 curr_cwnd_bytes; //当前拥塞窗口
        float cwnd_gain;  //快速启动增益
    }m_ccp;
    explicit CTcpClient(QObject *parent = nullptr);
    ~CTcpClient();
    quint64 uuid();
    void setUuid(quint64 uuid);
    void setServerInfo(const QString &sIp, const quint16 nPort,const QString &userName = "admin",const QString &passwd = "456123");
    void startClient();
    void stopClient();
    bool isConnected()const { return m_bIsConnected; }
    int authLevel()const { return m_nAuthLevel; }
    void login();
    bool proxyCall(BD_JSON_REQUEST_CODE_E nCode,void *data, QJsonDocument &ackDoc);
    bool uploadFileData(BD_TRANSFER_FILE_DATA_S &body);
    bool downloadFileData(BD_TMN_DOWNLOAD_FILE_DATA_S &body, BD_TRANSFER_FILE_DATA &tBody);
    bool startTest();
    void stopTest();
    bool isStartTest()const { return m_bIsStartTest; }
    bool sendTestPacket(uint16_t nLen);
    quint32 getCcpCwnd()const { return m_ccp.curr_cwnd_bytes; }
    void resetCcp();

    //统计数据传输速率
    static void updateSentBytes(quint64 nBytes);
    static quint64 getSentBytes();
    static void updateRecBytes(quint64 nBytes);
    static quint64 getRecBytes();

signals:
    void evt_ShowTips(QString tips);

private:
    void displayMessage(QString str);
    quint32 getRadomUUIDDestin();
    virtual void SendPacket(BD_PACKET_FUNC_ID_E fid,void *data,uint8_t res = 0);
    //处理有效数据包
    virtual int OnParseValidPacket(quint8 *ptr, int extdatalen);
    uint16_t sendData(uint8_t *ptr,uint16_t len);
    void delayMs(int ms);
    // 重新设置/获取发送时间(ms)
    void resetSendTime();
    qint64 getSendIntervalTime(void);
    WAIT_ACK_STATUS_E waittingAck(int ms = 10000);
    //重连
    void reConnect();
    void setWaitStatus(WAIT_ACK_STATUS_E nStatus);
    WAIT_ACK_STATUS_E waitStatus();
    //拥塞控制，避免频繁请求发送大数据包造成网络拥塞
    void updateCcp();
    //发送心跳包并等待返回消息
    void sendHeartbeat();


private slots:
    void on_timerTimeout();
    void new_data_recieved();
    void on_client_trasferred(qint64);
    void on_client_connected();
    void on_client_disconnected();
    void displayError(QAbstractSocket::SocketError);

private:
    quint64 m_uuid;
    quint64 m_maxUUID;
    quint64 m_minUUID;
    QTcpSocket *m_client;
    QByteArray m_receivedBlock;
    QTimer *m_timer;
    bool m_bSsl;
    QString m_serverIp;
    quint16 m_serverPort;
    QString m_userName;
    QString m_password;
    int m_nPayload;
    bool m_bIsConnected;
    int m_nAuthLevel; // < 0登录失败
    WAIT_ACK_STATUS_E m_bWaittingAckStatus;
    QMutex m_waitMutex;
    QJsonDocument m_ackJson;
    BD_TRANSFER_FILE_DATA m_downlodaData;
    qint64 m_lastSendTime;
    quint64  m_totalSendTestByte;
    bool m_bIsStartTest;
    uint8_t m_ackCode; //消息执行结果
    bool m_heartbeatAckReturned; //心跳包Ack返回标志
};

#endif // CTCPCLIENT_H
