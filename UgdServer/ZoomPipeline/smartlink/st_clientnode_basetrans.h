#ifndef ST_CLIENTNODE_BASETRANS_H
#define ST_CLIENTNODE_BASETRANS_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QDateTime>
#include "st_message.h"
#include "../pipeline/zp_pltaskbase.h"
#include "st_client_protocol.h"

using namespace BdUgdServer;
namespace ExampleServer{
	class st_client_table;

    class st_clientNode_baseTrans : public ZPTaskEngine::zp_plTaskBase, public BdUgdServer::st_client_protocol
	{
		Q_OBJECT
		friend class st_clien_table;
	public:
        explicit st_clientNode_baseTrans(st_client_table * pClientTable, QObject * pClientSock,QObject *parent = 0);
        inline TERMINAL_INFO_S tmnInfo()const { return m_terminalInfo; }
        void setClientSock(QObject * pClientSock);

		//!deal at most m_nMessageBlockSize messages per deal_message();
		static const int m_nMessageBlockSize = 8;
		//!The main functional method, will run in thread pool
		int run();

		//push new binary data into queue
		int push_new_data(const  QByteArray &  dtarray);

        quint64 uuid();
		QObject * sock();
        bool uuidValid();
        qint64 lastActiveTime();
		qint32 bytesLeft();

		void CheckHeartBeating();
        inline uint16_t uplinkFid()const { return m_UplinkFID; }
		bool bTermSet;
	protected:
		//!deal one message, affect m_currentRedOffset,m_currentMessageSize,m_currentHeader
		//!return bytes Used.
		int filter_message(QByteArray , int offset);
		//!in Trans-Layer, it does nothing.
        virtual int deal_current_message_block();

		//judge whether id is valid.
        bool bIsValidUserId(quint64 id);
        uint16_t sendPacket(uint16_t type, void *pPacket, uint8_t res);
        uint16_t sendData(uint8_t *ptr,uint16_t len);

	protected:
		//The current Read Offset, from m_list_RawData's beginning
		int m_currentReadOffset;
		//Current Message Offset, according to m_currentHeader
		int m_currentMessageSize;
		//Current un-procssed message block.for large blocks,
		//this array will be re-setted as soon as some part of data has been
		//dealed, eg, send a 200MB block, the 200MB data will be splitted into pieces
		QByteArray m_currentBlock;
		//current Header
        BD_MSG_HEADER_S m_currentHeader;
		//The raw data queue and its mutex
		QList<QByteArray> m_list_RawData;
		QMutex m_mutex_rawData;

        //UUID of this equipment
        bool m_bUUIDRecieved;
        quint64 m_uuid;  //Client ID
		//Client socket handle of this connection
		QObject * m_pClientSock;
		st_client_table * m_pClientTable;
        qint64 m_lastReportSec;
        QByteArray m_receivedBlock;
        USER_INFO_S m_userInfo;
        TERMINAL_INFO_S m_terminalInfo;
        QList<EVENT_INFO_S> m_events;
        QMutex m_eventMutex;
        bool m_bEventIsEcecuting;

	signals:

		void evt_SendDataToClient(QObject * objClient,QByteArray   dtarray);
		void evt_close_client(QObject * objClient);
		void evt_Message (QObject * psource,QString );

	};
}
#endif // ST_CLIENTNODE_BASETRANS_H
