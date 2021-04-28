#include "st_client_protocol.h"
#include <QStringList>
#include <QJsonDocument>

namespace BdUgdServer {
    st_client_protocol::st_client_protocol()
    {
        m_txBuf.resize(DATA_PAYLOAD);
    }

    uint16_t st_client_protocol::packing(uint16_t type, void *pPacket, uint8_t res)
    {
        m_txBuf.fill(0);
        uint8_t *ptr = (uint8_t*)m_txBuf.data();
        m_DownlinkFID = type;
        //Package header
        ptr[0] = 0xE7;
        //Message Function ID
        ptr[1] = 0xFF & (m_DownlinkFID >> 8);
        ptr[2] = 0xFF & m_DownlinkFID;
        //Message Properties

        //Device ID
        for(int i=0; i<7; i++)
            ptr[5+i] = m_DeviceID[i];
        //Message serial number
        ptr[12] = 0xFF & (m_DownlinkMSN >> 8);
        ptr[13] = 0xFF & m_DownlinkMSN;
        uint16_t msglen = 0;
        uint8_t *vdata = &ptr[14];
        switch (type) {
        case BdPlatformResponse_E:
            //@msn��Corresponding to the downlink message serial number
            //Initiated by the platform to set the downlink data of the message serial number
            vdata[msglen++] = 0xFF & (m_UplinkMSN >> 8);
            vdata[msglen++] = 0xFF & m_UplinkMSN;
            //@dfId:Corresponding to the downlink setting function ID
            //Initiated by the platform set up a commondownlink data response 2 bytes
            //is the function ID number of theplatform downlink data
            vdata[msglen++] = 0xFF & (m_UplinkFID >> 8);
            vdata[msglen++] = 0xFF & m_UplinkFID;
            //@result:Corresponds to the downlink setting returns result
            //0x00 setting successful,0x01 Setting failed,0XFF wrong format
            vdata[msglen++] = res;
            break;
        case BdTerminalLoginResponse_E:
        {
            BD_TMN_LOGIN_RESPONSE_S *loginResponse = static_cast<BD_TMN_LOGIN_RESPONSE_S*>(pPacket);
            memcpy(vdata,loginResponse->platform_time,6);
            msglen += 6;
            put2bytes(vdata,loginResponse->models_id,&msglen);
            put2bytes(vdata,loginResponse->displacement,&msglen);
            vdata[msglen++] = loginResponse->needUpgrade;
            vdata[msglen++] = loginResponse->auth;
        }
            break;
        case BdPlatformAckJson_E:
        case BdPlatformJson_E:
        {
            int fill_len = msglen;
            assert(pPacket != NULL);
            char *json = (char*)pPacket;
            msglen += strlen(json) + 1;
            if(msglen + 1024 > m_txBuf.size())
            {
                uint8_t *pBuff = (uint8_t*)malloc(DATA_PAYLOAD);
                if(pBuff == NULL)
                    return 0;
                memcpy(pBuff,ptr,fill_len);
                m_txBuf.resize(msglen + 1024);
                ptr = (uint8_t*)m_txBuf.data();
                memcpy(ptr,pBuff,fill_len);
                vdata = &ptr[14];
                free(pBuff);
            }
            strcpy((char*)vdata,json);

         }
            break;
        case BdPlarformSendFileData_E:
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
        }
        //Message Properties
        msglen += 14 + 2; //֡ͷ+֡β
        m_MsgProperties = 0;
        m_MsgProperties |= msglen;
        ptr[3] = 0xFF & (m_MsgProperties >> 8);
        ptr[4] = 0xFF & m_MsgProperties;
        ptr[msglen-2] = frame_xor(&ptr[1],msglen-3); //check

        ptr[msglen-1] = 0xE7;
        m_DownlinkMSN++;
        return msglen;
    }

}
