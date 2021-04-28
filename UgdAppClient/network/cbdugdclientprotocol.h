#ifndef CBDUGDCLIENTPROTOCOL_H
#define CBDUGDCLIENTPROTOCOL_H

#include <qglobal.h>
#include <QString>
#include "st_message.h"
#include "st_packet_tool.h"

using namespace BdUgdServer;
class CBdUgdClientProtocol:public st_packet_tool
{
public:
    CBdUgdClientProtocol();
    ~CBdUgdClientProtocol(){}
    //数据封包
    uint16_t Packing(BD_PACKET_FUNC_ID_E type, void *pPacket,uint8_t res = 0);
    // 解析bufptr, 返回解析的长度.
    // 如果存在一个有效的bb包, 则调用虚函数处理.
    int ParsePacket(uint8_t *bufptr, uint16_t buflen);
    virtual void SendPacket(BD_PACKET_FUNC_ID_E fid,void *data,uint8_t res = 0) = 0;
    //构建数据包用于发送
    uint16_t BuildPacket(uint8_t *txdata, BD_PACKET_FUNC_ID_E type, void *pPacket, uint8_t res);
    //发送Tcp连接所需心跳包
    void SendHeartbeatPacket();
    //处理有效数据包
    virtual int OnParseValidPacket(quint8 *extdata, int extdatalen) = 0;
    //获取系统时间
    static time_t GetSysTime(uint8_t *datetime,uint8_t len);
    //获取时间
    static void GetTime(time_t nowTime,uint8_t *datetime,uint8_t len);

   //获取设备ID字符串
   inline QString GetDeviceId() const { return m_deviceId; }
   //设置设备ID 7Bytes BCD
   void SetDeviceId(char *id);

private:


protected:
    uint16_t m_downlinkFID; //服务器发送到终端的Message function ID 缓存
    uint16_t m_uplinkFID;   //终端发送到服务器的Message function ID 缓存
    uint16_t m_downUplinkFID; //应答消息中上行数据功能ID
    uint16_t m_downlinkMSN; //服务器发送到终端数据Message serial number
    uint16_t m_uplinkMSN;   //终端发送到服务器数据Message serial number

private:
    uint8_t m_deviceID[8];  //设备ID,m_DeviceID[7]为xor校验位
    uint8_t m_gprsDataRX[DATA_PAYLOAD]; //GPRS数据接收缓存
    uint8_t m_gprsDataTX[DATA_PAYLOAD]; //GPRS数据发送缓存
    uint16_t m_msgProperties;    //消息属性
    QString m_deviceId;
};

#endif // CBDUGDCLIENTPROTOCOL_H
