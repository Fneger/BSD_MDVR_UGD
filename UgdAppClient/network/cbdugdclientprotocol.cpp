#include "cbdugdclientprotocol.h"
#include <time.h>
#include <QDebug>
#include "ccommon.h"

CBdUgdClientProtocol::CBdUgdClientProtocol() : st_packet_tool()
{
    //Device No.U201508120029
    QString uuidStr = Settings.value("ClientSettings/uuid","U000000000001").toString();
    uuidStr = CCommon::Instance()->getCpuId();

    if(uuidStr.length() > 14)
        uuidStr.remove(14,uuidStr.length() - 13);
    m_deviceId = uuidStr;
    memset(m_deviceID,0,sizeof (m_deviceID));
    int bcdlen = 0;
    str2bcd(m_deviceID,uuidStr.toLocal8Bit().data(),&bcdlen);
    m_deviceID[7] = 0;
    char id[32];
    bcd2str(id,m_deviceID,7);
    m_deviceId = QString(id);
    Settings.setValue("ClientSettings/uuid",uuidStr);
}


//@type:数据包功能ID
//@res:服务器指令执行结果
uint16_t CBdUgdClientProtocol::Packing(BD_PACKET_FUNC_ID_E type, void *pPacket, uint8_t res)
{
    memset(m_gprsDataTX,0,DATA_PAYLOAD);
    m_uplinkFID = type;
    //Package header
    m_gprsDataTX[0] = 0xE7;
    //Message Function ID
    m_gprsDataTX[1] = 0xFF & (m_uplinkFID >> 8);
    m_gprsDataTX[2] = 0xFF & m_uplinkFID;
    //Message Properties

    //Device ID
    for(int i=0; i<7; i++)
        m_gprsDataTX[5+i] = m_deviceID[i];
    //Message serial number
    m_gprsDataTX[12] = 0xFF & (m_uplinkMSN >> 8);
    m_gprsDataTX[13] = 0xFF & m_uplinkMSN;
    uint16_t msglen = 0;
    uint8_t *vdata = &m_gprsDataTX[14];

    switch (m_uplinkFID) {
    case BdTerminalResponse_E:
        //@msn：Corresponding to the downlink message serial number
        //Initiated by the platform to set the downlink data of the message serial number
        put2bytes(&vdata[msglen],m_downlinkMSN,&msglen);
        //@dfId:Corresponding to the downlink setting function ID
        //Initiated by the platform set up a commondownlink data response 2 bytes
        //is the function ID number of theplatform downlink data
        put2bytes(&vdata[msglen],m_downlinkFID,&msglen);
        //@result:Corresponds to the downlink setting returns result
        //0x00 setting successful,0x01 Setting failed,0XFF wrong format
        vdata[msglen++] = res;
        break;
    case BdTerminalStartClientTest_E:
    case BdTerminalRptsHeartbeat_E:
        //no
        break;
    case BdTerminalAnswerDFI_E:
        break;
    case BdTerminalLogin_E:
    {
        BD_TMN_LOGIN_INFO_S *loginInfo = static_cast<BD_TMN_LOGIN_INFO_S*>(pPacket);
        memcpy(vdata,loginInfo->tsft_version,14);
        msglen += 14;
        memcpy(&vdata[msglen],loginInfo->stf_date,10);
        msglen += 10;
        memcpy(&vdata[msglen],loginInfo->cpu_id,12);
        msglen += 12;
        vdata[msglen++] = loginInfo->terminal_type;
        memcpy(&vdata[msglen],loginInfo->user_name,33);
        msglen += 33;
        memcpy(&vdata[msglen],loginInfo->password,33);
        msglen += 33;
    }
        break;
    case BdTerminalDownloadFileData_E:
    {
        BD_TMN_DOWNLOAD_FILE_DATA_S *body = static_cast<BD_TMN_DOWNLOAD_FILE_DATA_S*>(pPacket);
        put4bytes(&vdata[msglen],body->request_receive_bytes,&msglen);
        put4bytes(&vdata[msglen],body->received_bytes,&msglen);
        put4bytes(&vdata[msglen],body->not_received_bytes,&msglen);
        put4bytes(&vdata[msglen],body->file_size,&msglen);
    }
        break;
    case BdPlarformSendFileData_E:
    case BdTerminalUploadFileData_E:
    {
        BD_TRANSFER_FILE_DATA_S *body = static_cast<BD_TRANSFER_FILE_DATA_S*>(pPacket);
        put4bytes(&vdata[msglen],body->currently_sent_bytes,&msglen);
        put4bytes(&vdata[msglen],body->sent_bytes,&msglen);
        put4bytes(&vdata[msglen],body->not_sent_bytes,&msglen);
        put4bytes(&vdata[msglen],body->file_size,&msglen);
        memcpy(&vdata[msglen],body->file_data,body->currently_sent_bytes);
        msglen += body->currently_sent_bytes;
    }
        break;
    case BdTerminalClientTest_E:
    {
        BD_TMN_CLIENT_TEST_S *body = static_cast<BD_TMN_CLIENT_TEST_S*>(pPacket);
        put4bytes(&vdata[msglen],body->currently_sent_bytes,&msglen);
        put8bytes(&vdata[msglen],body->total_sent_bytes,&msglen);
        memcpy(&vdata[msglen],body->test_data,body->currently_sent_bytes);
        msglen += body->currently_sent_bytes;
    }
        break;
    case BdTerminalJson_E:
    {
        if(pPacket != NULL)
        {
            char *json = (char*)pPacket;
            strcpy((char*)vdata,json);
            msglen += strlen(json) + 1;
        }
    }
        break;
    default:
        return 0;
    }
    //Message Properties
    msglen += 14 + 2; //帧头+帧尾
    m_msgProperties = 0;
    m_msgProperties |= msglen;
    m_gprsDataTX[3] = 0xFF & (m_msgProperties >> 8);
    m_gprsDataTX[4] = 0xFF & m_msgProperties;
    m_gprsDataTX[msglen-2] = frame_xor(&m_gprsDataTX[1],msglen-3); //check

    m_gprsDataTX[msglen-1] = 0xE7;
    m_uplinkMSN++;
    return msglen;
}

// 解析bufptr, 返回解析的长度.
// 如果存在一个有效的bb包, 则调用虚函数处理.
int CBdUgdClientProtocol::ParsePacket(uint8_t *bufptr, uint16_t buflen)
{
    int rc;
    int len = 0, retlen, valid_len;
    uint8_t *data = NULL;
    while ( 1 )
    {
        retlen = 0;
        bool finded = false;
        int pos = 0;
        for(int i=0; i<buflen; i++)
        {
            if(bufptr[i] == 0xE7 && finded == false)//找到数据包开始标志
            {
                pos = i;
                finded = true;
            }
            else if(bufptr[i] == 0xE7 && finded == true) //找到数据包结束标志
            {
                retlen = i-pos+1;
                if(retlen > DATA_PAYLOAD || retlen < 16)//服务器下行数据长度不超过257或数据包太短，格式不正确
                    return -1;
                else
                {
                    data = bufptr + pos;
                    retlen = i-pos+1;

                    valid_len = frame_reverse_escape(data,retlen);
                    m_downlinkFID = (data[1] << 8) | data[2];
                    qDebug("m_downlinkFID:%04x",m_downlinkFID);
                    m_downlinkMSN = (bufptr[12] << 8) | bufptr[13];
                    print_data(data,valid_len);
                    OnParseValidPacket(&data[14],valid_len-14);
                    rc = 1;
                    break;
                }
            }
        }
        len += retlen;
        if ( rc > 0 )
            break;
    }
    return len;
}

//构建数据包用于发送
uint16_t CBdUgdClientProtocol::BuildPacket(uint8_t *txdata, BD_PACKET_FUNC_ID_E type, void *pPacket, uint8_t res)
{
    assert(txdata != NULL);
    uint16_t dataLen;
    dataLen = Packing(type,pPacket,res);//打包,增加协议头、尾
    dataLen = frame_escape(m_gprsDataTX,dataLen);//转义
    memcpy(txdata,m_gprsDataTX,dataLen);
    print_data(txdata,dataLen);
    return dataLen;
}

//发送Tcp连接所需心跳包
void CBdUgdClientProtocol::SendHeartbeatPacket()
{
    SendPacket(BdTerminalRptsHeartbeat_E,NULL);
}

//获取系统时间
time_t CBdUgdClientProtocol::GetSysTime(uint8_t *datetime,uint8_t len)
{
    if(len < 6 || datetime == NULL)
        return 0;
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    //rawtime = CBdNetService::GetUTCTime(rawtime);
    info = gmtime(&rawtime );
    //tminfo = localtime(&rawTime);
    datetime[0] = (info->tm_year+1900) % 100;
    datetime[1] = info->tm_mon+1;
    datetime[2] = info->tm_mday;
    datetime[3] = info->tm_hour;
    datetime[4] = info->tm_min;
    datetime[5] = info->tm_sec;

    //printf("GetSysTime-----sec:%ld---------%02d/%02d/%02d %02d:%02d:%02d\n",rawtime, datetime[0],datetime[1],datetime[2],datetime[3],datetime[4],datetime[5]);
    return rawtime;
}

void CBdUgdClientProtocol::GetTime(time_t nowTime,uint8_t *datetime,uint8_t len)
{
    if(len < 6 || datetime == NULL)
        return;
#if 0
    struct tm* tminfo;
    nowTime = CBdNetService::GetUTCTime(nowTime);
    tminfo = gmtime(&nowTime);
    datetime[0] = (tminfo->tm_year+1900) % 100;
    datetime[1] = tminfo->tm_mon+1;
    datetime[2] = tminfo->tm_mday;
    datetime[3] = tminfo->tm_hour;
    datetime[4] = tminfo->tm_min;
    datetime[5] = tminfo->tm_sec;
#endif
}


//设置设备ID 7Bytes BCD
void CBdUgdClientProtocol::SetDeviceId(char *id)
{
    if(id != NULL)
    {
        int bcdlen = 0;
        memset(m_deviceID,0,7);
        str2bcd(m_deviceID,(char*)id,&bcdlen);
        if(bcdlen > 0)
        {
             m_deviceID[bcdlen] = 0;
             m_deviceID[7] = frame_xor(m_deviceID,7);
             m_deviceId = QString(id);
        }
    }
}
