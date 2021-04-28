#ifndef ST_CLIENT_PROTOCOL_H
#define ST_CLIENT_PROTOCOL_H


#include <qglobal.h>
#include <QString>
#include "st_message.h"
#include "st_packet_tool.h"
namespace BdUgdServer {
    //远程版本管理通讯协议相关函数
    class st_client_protocol:public st_packet_tool
    {
    public:
        explicit st_client_protocol();
    protected:
        uint16_t packing(uint16_t type, void *pPacket,uint8_t res = 0);

    protected:
        QString m_deviceIdStr;
        QByteArray m_txBuf; //GPRS数据发送缓存
        uint8_t m_MsgProperties;    //消息属性
        uint16_t m_DownlinkMSN; //服务器发送到终端数据Message serial number
        uint16_t m_UplinkMSN;   //终端发送到服务器数据Message serial number
        uint8_t m_DeviceID[8];  //设备ID,m_DeviceID[7]为xor校验位
        uint16_t m_DownlinkFID; //服务器发送到终端的Message function ID 缓存
        uint16_t m_UplinkFID;   //终端发送到服务器的Message function ID 缓存

    };
}


#endif // ST_CLIENT_PROTOCOL_H
