#include "st_packet_tool.h"
#include <QDebug>
#include "st_message.h"

namespace BdUgdServer {
    bool st_packet_tool::bcd2str(char *str, uint8_t *bcd, int bcd_len)
    {
        if(bcd==NULL || bcd_len<=0 || str == NULL)
            return false;
        int i,j;
        str[0] = bcd[0];
        for(i=1,j=1;i<bcd_len;i++,j+=2)
        {
            str[j]= (bcd[i]>>4) > 9 ? (bcd[i]>>4)-10+'A' : (bcd[i]>>4)+'0';
            str[j+1]=(bcd[i] & 0x0F) >9 ? (bcd[i] & 0x0F)-10+'A' : (bcd[i] & 0x0F)+'0';
        }
        str[j]='\0';
        return true;
    }

    bool st_packet_tool::str2bcd(uint8_t *bcd, char *str,int *bcd_len)
    {
        uint8_t byteHigh = 0x00;
        uint8_t byteLow = 0x00;
        size_t strLen = strlen(str);
        if(bcd==NULL || strLen<=0 || str == NULL)
            return false;
        (*bcd_len) = 1;
        bcd[0] = str[0];
        int i = 1;
        while(i < strLen){
            if(i != strLen - 1)
            {
                byteHigh = str[i] - '0';
                byteLow = str[i+1] - '0';
                bcd[(*bcd_len)++] = (byteHigh<<4) | byteLow;
                i += 2;
            }
            else
            {
                bcd[(*bcd_len)++] = (str[i]-'0') << 4;
                i++;
            }
        }
        return true;
    }

    uint8_t st_packet_tool::frame_xor(uint8_t *ptr,uint16_t len)
    {
        assert(ptr != NULL && len > 0);
        uint8_t res = ptr[0];
        for(int i=0; i<len; i++)
            res ^= ptr[i];
        return res;
    }

    //数据帧转义,返回转义后帧长度 0xE7->0xE6+0x02,0xE6->0xE6+0x01
    uint16_t st_packet_tool::frame_escape(uint8_t *ptr,uint16_t len)
    {
        if(!frame_check(ptr,len))
            return 0;
        uint8_t *data = (uint8_t*)malloc(DATA_PAYLOAD);
        if(data == NULL)
            return 0;
        uint16_t pos = 0;
        data[pos++] = ptr[0];
        for(int i=1; i<len-1; i++)
        {
            if(ptr[i] == 0xE7)
            {
                data[pos++] = 0xE6;
                data[pos++] = 0x02;
            }
            else if(ptr[i] == 0xE6)
            {
                data[pos++] = 0xE6;
                data[pos++] = 0x01;
            }
            else
                data[pos++] = ptr[i];
        }
        data[pos++] = ptr[len-1];
        memcpy(ptr,data,pos);
        free(data);
        return pos;
    }

    //数据帧反转义,返回反转义后帧长度 0xE6+0x02->0xE7,0xE6+0x01->0xE6
    uint16_t st_packet_tool::frame_reverse_escape(uint8_t *ptr,uint16_t len)
    {
//        if(!frame_check(ptr,len))
//            return 0;
        uint8_t *data = (uint8_t*)malloc(DATA_PAYLOAD);
        if(data == NULL)
            return 0;
        uint16_t pos = 0;
        data[pos++] = ptr[0];
        for(int i=1; i<len-1; i++)
        {
            if(ptr[i] == 0xE6 && ptr[i+1] == 0x02)
            {
                data[pos++] = 0xE7;
                i++;
            }
            else if(ptr[i] == 0xE6 && ptr[i+1] == 0x01)
            {
                data[pos++] = 0xE6;
                i++;
            }
            else
                data[pos++] = ptr[i];
        }
        data[pos++] = ptr[len-1];
        memcpy(ptr,data,pos);
        free(data);
        return pos;
    }

    //判断数据帧是否合法
    bool st_packet_tool::frame_check(uint8_t *ptr,uint16_t len)
    {
        if(ptr == NULL && len < 16)
            return false;
        if(ptr[0] != 0xE7 || ptr[len - 1] != 0xE7)
            return false;
        return true;
    }

    void st_packet_tool::print_data(uint8_t *ptr,uint16_t len)
    {
        QString dstr;
        for(int i=0; i<len; i++)
        {

            dstr += QString("%1 ").arg(ptr[i], 2, 16, QLatin1Char('0'));
        }
        //qDebug() << dstr;
    }
}


