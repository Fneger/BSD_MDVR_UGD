#include "st_clientnode_basetrans.h"
#include "st_client_table.h"
#include <assert.h>
#include <QJsonDocument>
#include <QTcpSocket>

namespace ExampleServer{
	st_clientNode_baseTrans::st_clientNode_baseTrans(st_client_table * pClientTable, QObject * pClientSock ,QObject *parent) :
		zp_plTaskBase(parent)
	{
        m_bUUIDRecieved = false;
		m_currentReadOffset = 0;
		m_currentMessageSize = 0;
		m_pClientSock = pClientSock;
        m_uuid = 0xffffffffffffffff;//Not Valid
		m_pClientTable = pClientTable;
		bTermSet = false;
        m_last_Report = QDateTime::currentDateTime();
	}
    quint64 st_clientNode_baseTrans::uuid()
	{
        return m_uuid;
	}
	QObject * st_clientNode_baseTrans::sock()
	{
		return m_pClientSock;
	}
    bool st_clientNode_baseTrans::uuidValid()
	{
        return m_bUUIDRecieved;
	}
	QDateTime st_clientNode_baseTrans::lastActiveTime()
	{
		return m_last_Report;
	}
	qint32 st_clientNode_baseTrans::bytesLeft()
	{
        return 0;
	}
	//judge whether id is valid.
    bool st_clientNode_baseTrans::bIsValidUserId(quint64 id)
	{
        return id >=0x01 && id <=0x0FFFFFFFFFFFFFFF;
	}

	//The main functional method, will run in thread pool
	int st_clientNode_baseTrans::run()
	{
		if (bTermSet==true)
		{
			//qDebug()<<QString("%1(%2) Node Martked Deleted, return.\n").arg((unsigned int)this).arg(ref());
			return 0;
        }
		int nCurrSz = -1;
        int nMessage = m_nMessageBlockSize;
		while (--nMessage>=0 && nCurrSz!=0  )
        {
			m_mutex_rawData.lock();
			if (m_list_RawData.size())
            {
                m_receivedBlock.append(*m_list_RawData.begin());
                m_list_RawData.pop_front();
            }
			m_mutex_rawData.unlock();
            if (m_receivedBlock.isEmpty()==false && m_receivedBlock.isNull()==false)
			{
                m_currentReadOffset = filter_message(m_receivedBlock,m_currentReadOffset);
                if (m_currentReadOffset >= m_receivedBlock.size())
				{
                    m_receivedBlock.truncate(0);
                    m_currentReadOffset = 0;
#if 0
					m_mutex_rawData.lock();
					if (m_list_RawData.empty()==false)
						m_list_RawData.pop_front();
					else
						assert(false);
					m_currentReadOffset = 0;
					m_mutex_rawData.unlock();
#endif
				}
			}
			else
			{
				m_mutex_rawData.lock();
				//pop empty cabs
				if (m_list_RawData.empty()==false)
					m_list_RawData.pop_front();
				m_mutex_rawData.unlock();
			}
			m_mutex_rawData.lock();
			nCurrSz = m_list_RawData.size();
			m_mutex_rawData.unlock();
		}
		m_mutex_rawData.lock();
		nCurrSz = m_list_RawData.size();
		m_mutex_rawData.unlock();
		if (nCurrSz==0)
			return 0;
		return -1;
	}

	//push new binary data into queue
	int st_clientNode_baseTrans::push_new_data(const  QByteArray &  dtarray)
    {
		int res = 0;
		m_mutex_rawData.lock();

		m_list_RawData.push_back(dtarray);
		res = m_list_RawData.size();
		m_mutex_rawData.unlock();
		m_last_Report = QDateTime::currentDateTime();
		return res;
	}
	//!deal one message, affect m_currentRedOffset,m_currentMessageSize,m_currentHeader
	//!return bytes Used.
	int st_clientNode_baseTrans::filter_message(QByteArray  block, int offset)
    {
        QByteArray ba = block;
        ba.truncate(14);
        qDebug() << ba;
        const int blocklen = block.length();
        int rc = 0;
        int retlen = 0;
        int valid_len;
        uint8_t *data = NULL;
        uint8_t *bufptr = (uint8_t*)block.data();
        int i=0;
        bool finded = false;
        int pos = 0;
        for(i=offset; i<blocklen; i++)
        {
            if(bufptr[i] == 0xE7 && finded == false)//找到数据包开始标志
            {
                pos = i;
                data = bufptr + pos;
                finded = true;
            }
            else if(bufptr[i] == 0xE7 && finded == true) //找到数据包结束标志
            {
                retlen = i-pos+1;
                if(retlen > DATA_BUFF_LEN_MAX || retlen < 16)//服务器下行数据长度不超过257或数据包太短，格式不正确
                    return ++i;
                else
                {
                    valid_len = frame_reverse_escape(data,retlen);
                    m_currentMessageSize = valid_len;
                    m_currentBlock = QByteArray((char*)data,valid_len);
                    deal_current_message_block();
                    rc = 1;
                    i++;
                    offset = i;
                    break;
                }
            }
        }
        //qDebug() << "filter length" << block.size() << offset << i;
#if 0
		while (blocklen>offset)
		{
			const char * dataptr = block.constData();

            if(block_data[i] == 0xE7 && finded == false)//找到数据包开始标志
            {
                pos = i;
                finded = true;
            }
            else if(block_data[i] == 0xE7 && finded == true) //找到数据包结束标志
            {
                retlen = i-pos+1;
                if(retlen > S_MSG_LEN_MAX)//服务器下行数据长度不超过257或数据包太短，格式不正确
                    break;
                else if(retlen < 16)
                {
                    i = 0;
                    break;
                }
                else
                {
                    data = block_data + pos;
                    retlen = i-pos+1;

                    valid_len = frame_reverse_escape(data,retlen);
                    //print_data(data,valid_len);
                    m_currentMessageSize = valid_len;
                    m_currentBlock = QByteArray((char*)data,valid_len);
                    deal_current_message_block();
                    rc = 1;
                    i++;
                    break;
                }
            }
            i++;
            if ( rc > 0 )
                break;
            if(0)
			{
				const char * ptrCurrData = m_currentBlock.constData();
				emit evt_Message(this,tr("Client Send a unknown start Header %1 %2. Close client immediately.")
								 .arg((int)(ptrCurrData[0])).arg((int)(ptrCurrData[1])));
				m_currentMessageSize = 0;
				m_currentBlock = QByteArray();
				offset = blocklen;
				emit evt_close_client(this->sock());
			}
		} // end while block len > offset
        offset += i;
#endif
        return offset;
	}
	//in Trans-Level, do nothing.
    int st_clientNode_baseTrans::deal_current_message_block()
	{
		//First, get uuid as soon as possible
        uint8_t *ptr = (uint8_t*)m_currentBlock.data();
        if(m_currentBlock.length() > DATA_BUFF_LEN_MAX) //服务器下行数据长度不超过257
            return -1;
        if(m_currentBlock.length() < 16) //数据包太短，格式不正确
            return -1;
        m_UplinkFID = (ptr[1] << 8) | ptr[2];

        QStringList msg;
        msg << "m_UplinkFID " + QString(" 0x%1").arg(m_UplinkFID,4,16,QChar('0')) + "\n";

        m_UplinkMSN = (ptr[12] << 8) | ptr[13];
        memcpy(m_DeviceID,&ptr[5],7);
        quint64 source_id = (quint64)(m_DeviceID[6] | (m_DeviceID[5] << 8) | (m_DeviceID[4] << 16) | (m_DeviceID[3] << 24) |
                ((quint64)m_DeviceID[2] << 32) | ((quint64)m_DeviceID[1] << 40) | ((quint64)m_DeviceID[0] << 48));
        qDebug() << msg << m_UplinkMSN << QString("source_id %1").arg(source_id,16,16,QChar('0')) << m_bUUIDRecieved;
        if(m_bUUIDRecieved == false)
        {
            if (bIsValidUserId( source_id) )
            {
                m_bUUIDRecieved = true;
                m_uuid =  source_id;
                //regisit client node to hash-table;
                m_pClientTable->regisitClientUUID(this);
            }
            else //Invalid
            {
                emit evt_Message(this,tr("Client ID is invalid! Close client immediatly."));
                m_currentBlock = QByteArray();
                emit evt_close_client(this->sock());
                return -1;
            }
        }
        else
        {
            if (!( bIsValidUserId(source_id)|| (source_id==0xffffffffffffffff)))
            {
                emit evt_Message(this,tr("Client ID is invalid! Close client immediatly."));
                m_currentBlock = QByteArray();
                emit evt_close_client(this->sock());
                return -1;
            }
            if (bIsValidUserId(source_id)==true && m_uuid != source_id)
            {
                emit evt_Message(this,tr("Client ID Changed in Runtime! Close client immediatly.") +
                                 QString("new id = %1,old id = %2.%3").arg(source_id,16,16,QChar('0')).arg(m_uuid,16,16,QChar('0')).arg((quint64)this));
                m_currentBlock = QByteArray();
                emit evt_close_client(this->sock());
                return -1;
            }
        }
		return 0;
    }

    uint16_t st_clientNode_baseTrans::sendPacket(uint16_t type, void *pPacket, uint8_t res)
    {
//        if(m_server->GetNetTrans())
//            return 0;
        uint16_t dataLen;
        dataLen = packing(type,pPacket,res);
        dataLen = frame_escape((uint8_t*)m_txBuf.data(),dataLen);
        dataLen = sendData((uint8_t*)m_txBuf.data(),dataLen);
        return dataLen;
    }

    uint16_t st_clientNode_baseTrans::sendData(uint8_t *ptr,uint16_t len)
    {
//        if(m_tcpSocket == nullptr)
//            return 0;
        print_data(ptr,len);
        QByteArray sendData((char*)ptr,len);
        emit evt_SendDataToClient(this->sock(),sendData);
        qDebug() << QString("TestServer send len:%1, write len:%2").arg(len).arg(len);
        return len;
    }

	void st_clientNode_baseTrans::CheckHeartBeating()
	{
		QDateTime dtm = QDateTime::currentDateTime();
		qint64 usc = this->m_last_Report.secsTo(dtm);
		if (usc >=m_pClientTable->heartBeatingThrd())
		{
			emit evt_Message(this,tr("Client ") + QString("%1").arg((unsigned int)((quint64)this)) + tr(" is dead, kick out."));
			emit evt_close_client(this->sock());
		}
	}
}
