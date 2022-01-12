#include "st_client_table.h"
#include "st_clientnode_applayer.h"
#include <assert.h>
//#include "st_cross_svr_node.h"
//#include "st_cross_svr_msg.h"
#include <functional>
#include <QList>
#include <QStringList>
#include <QTcpSocket>
#include "st_client_event.h"
#include <QTimer>

namespace ExampleServer{
	using namespace std::placeholders;
	st_client_table::st_client_table(
			ZPNetwork::zp_net_Engine * NetEngine,
			ZPTaskEngine::zp_pipeline * taskeng,
			ZPDatabase::DatabaseResource * pDb,
			ZP_Cluster::zp_ClusterTerm * pCluster,
			QObject *parent) :
		QObject(parent)
	  ,m_pThreadEngine(NetEngine)
	  ,m_pTaskEngine(taskeng)
	  ,m_pDatabaseRes(pDb)
	  ,m_pCluster(pCluster)
      ,m_nBalanceMax(10240)
	{

		m_nHeartBeatingDeadThrd = 180;
		connect (m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_NewClientConnected,this,&st_client_table::on_evt_NewClientConnected,Qt::QueuedConnection);
		connect (m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_ClientEncrypted,this,&st_client_table::on_evt_ClientEncrypted,Qt::QueuedConnection);
		connect (m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_ClientDisconnected,this,&st_client_table::on_evt_ClientDisconnected,Qt::QueuedConnection);
		connect (m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_Data_recieved,this,&st_client_table::on_evt_Data_recieved,Qt::QueuedConnection);
		connect (m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_Data_transferred,this,&st_client_table::on_evt_Data_transferred,Qt::QueuedConnection);

		connect (m_pCluster,&ZP_Cluster::zp_ClusterTerm::evt_NewSvrConnected,this,&st_client_table::on_evt_NewSvrConnected,Qt::QueuedConnection);
		connect (m_pCluster,&ZP_Cluster::zp_ClusterTerm::evt_NewSvrDisconnected,this,&st_client_table::on_evt_NewSvrDisconnected,Qt::QueuedConnection);
		connect (m_pCluster,&ZP_Cluster::zp_ClusterTerm::evt_RemoteData_recieved,this,&st_client_table::on_evt_RemoteData_recieved,Qt::QueuedConnection);
		connect (m_pCluster,&ZP_Cluster::zp_ClusterTerm::evt_RemoteData_transferred,this,&st_client_table::on_evt_RemoteData_transferred,Qt::QueuedConnection);
		m_pCluster->SetNodeFactory(
					std::bind(&st_client_table::cross_svr_node_factory,
							  this,
							  _1,_2,_3)
					);


        m_delNoActiveNodeTimer = new QTimer(this);
        connect(m_delNoActiveNodeTimer, &QTimer::timeout, this, &st_client_table::on_evt_DelNoActiveNodeTimeout);
        m_delNoActiveNodeTimer->setInterval(10000);
        m_delNoActiveNodeTimer->start();
	}
	void st_client_table::setBalanceMax(int nmax)
	{
		m_nBalanceMax = nmax;
	}

	int st_client_table::balanceMax()
	{
		return m_nBalanceMax;
	}


	int st_client_table::heartBeatingThrd()
	{
		return m_nHeartBeatingDeadThrd;
	}
	void st_client_table::setHeartBeatingThrd(int h)
	{
		m_nHeartBeatingDeadThrd = h;
	}

	//Database and disk resources
	QString st_client_table::Database_UserAcct()
	{
		return m_strDBName_useraccount;
	}
	void st_client_table::setDatabase_UserAcct(QString  s)
	{
		m_strDBName_useraccount = s;
	}
	QString st_client_table::Database_Event()
	{
		return m_strDBName_event;
	}
	void st_client_table::setDatabase_Event(QString  s)
	{
		m_strDBName_event = s;
	}
	QString st_client_table::largeFileFolder()
	{
		return m_largeFileFolder;
	}
	void st_client_table::setLargeFileFolder(QString  s)
	{
		m_largeFileFolder = s;
	}

	ZPDatabase::DatabaseResource * st_client_table::dbRes()
	{
		return m_pDatabaseRes;
	}

	st_client_table::~st_client_table()
	{
        m_delNoActiveNodeTimer->stop();
        m_delNoActiveNodeTimer->deleteLater();
	}
	void  st_client_table::KickDeadClients()
	{
		m_hash_mutex.lock();
		for (std::unordered_map<QObject *,st_clientNode_baseTrans *>::iterator p =m_hash_sock2node.begin();
			 p!=m_hash_sock2node.end();p++)
		{
			(*p).second->CheckHeartBeating();
		}
		m_hash_mutex.unlock();
	}
	bool st_client_table::regisitClientUUID(st_clientNode_baseTrans * c)
	{
		if (c->uuidValid()==false)
			return false;
		m_hash_mutex.lock();
		m_hash_uuid2node[c->uuid()] = c;
        m_hash_mutex.unlock();
        broadcast_client_uuid(c->uuid(),true);
		return true;
	}

    st_clientNode_baseTrans *  st_client_table::clientNodeFromUUID(quint64 uuid)
	{
		m_hash_mutex.lock();
		if (m_hash_uuid2node.find(uuid)!=m_hash_uuid2node.end())
		{
			m_hash_mutex.unlock();
			return m_hash_uuid2node[uuid];
		}
		m_hash_mutex.unlock();

		return NULL;
	}

	st_clientNode_baseTrans *  st_client_table::clientNodeFromSocket(QObject * sock)
	{
		m_hash_mutex.lock();
		if (m_hash_sock2node.find(sock)!=m_hash_sock2node.end())
		{
			m_hash_mutex.unlock();
			return m_hash_sock2node[sock];
		}
		m_hash_mutex.unlock();
		return NULL;

	}

    st_clientNode_baseTrans * st_client_table::clientNodeFromEventInfo(const EVENT_INFO_S &event)
    {
        m_hash_mutex.lock();
        QString id;
        for(auto& pNode:m_hash_uuid2node)
        {
            TERMINAL_INFO_S tmnInfo = pNode.second->tmnInfo();
            switch (event.idType) {
            case EventRevDeviceNum_E:
                id = tmnInfo.deviceNum;
                break;
            case EventRevVideoId_E:
                id = tmnInfo.phoneNumber;
                break;
            case EventRevPhoneNum_E:
                id = tmnInfo.licenseNum;
                break;
            case EventRevLicenseNum_E:
                id = tmnInfo.videoId;
                break;
            default:

                break;
            }
            if(event.id == id)
                return pNode.second;
        }
        m_hash_mutex.unlock();
        return NULL;
    }
	//this event indicates new client encrypted.
	void st_client_table::on_evt_ClientEncrypted(QObject * clientHandle)
    {//不在此处创建任务节点，接收到数据后再创建
        return;
		bool nHashContains = false;
		st_clientNode_baseTrans * pClientNode = 0;
		m_hash_mutex.lock();
		nHashContains = (m_hash_sock2node.find(clientHandle)!=m_hash_sock2node.end())?true:false;
		if (false==nHashContains)
		{
			st_clientNode_baseTrans * pnode = new st_clientNodeAppLayer(this,clientHandle,0);
			//using queued connection of send and revieve;
            connect (pnode,&st_clientNode_baseTrans::evt_SendDataToClient,m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_SendDataToClient,Qt::QueuedConnection);
			connect (pnode,&st_clientNode_baseTrans::evt_close_client,m_pThreadEngine,&ZPNetwork::zp_net_Engine::KickClients,Qt::QueuedConnection);
			connect (pnode,&st_clientNode_baseTrans::evt_Message,this,&st_client_table::evt_Message,Qt::QueuedConnection);
			m_hash_sock2node[clientHandle] = pnode;
			nHashContains = true;
            pClientNode = pnode;
		}
		else
		{
			pClientNode =  m_hash_sock2node[clientHandle];
		}
		m_hash_mutex.unlock();
		assert(nHashContains!=0 && pClientNode !=0);
	}

	//this event indicates new client connected.
	void  st_client_table::on_evt_NewClientConnected(QObject * clientHandle)
    {//不在此处创建任务节点，接收到数据后再创建
        return;
		bool nHashContains = false;
		st_clientNode_baseTrans * pClientNode = 0;
		m_hash_mutex.lock();
		nHashContains = (m_hash_sock2node.find(clientHandle)!=m_hash_sock2node.end())?true:false;
		if (false==nHashContains)
		{
			st_clientNode_baseTrans * pnode = new st_clientNodeAppLayer(this,clientHandle,0);
			//using queued connection of send and revieve;
            connect (pnode,&st_clientNode_baseTrans::evt_SendDataToClient,m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_SendDataToClient,Qt::QueuedConnection);
			connect (pnode,&st_clientNode_baseTrans::evt_close_client,m_pThreadEngine,&ZPNetwork::zp_net_Engine::KickClients,Qt::QueuedConnection);
			connect (pnode,&st_clientNode_baseTrans::evt_Message,this,&st_client_table::evt_Message,Qt::QueuedConnection);
			m_hash_sock2node[clientHandle] = pnode;
			nHashContains = true;
            pClientNode = pnode;
		}
		else
		{
			pClientNode =  m_hash_sock2node[clientHandle];
		}
		m_hash_mutex.unlock();
		assert(nHashContains!=0 && pClientNode !=0);
	}

	//this event indicates a client disconnected.
	void  st_client_table::on_evt_ClientDisconnected(QObject * clientHandle)
	{
		bool nHashContains  = false;
		st_clientNode_baseTrans * pClientNode = 0;
		m_hash_mutex.lock();
		nHashContains = (m_hash_sock2node.find(clientHandle)!=m_hash_sock2node.end())?true:false;
		if (nHashContains)
			pClientNode =  m_hash_sock2node[clientHandle];
		if (pClientNode)
		{
			m_hash_sock2node.erase(clientHandle);
			if (pClientNode->uuidValid())
			{
				//This is important. some time m_hash_sock2node and m_hash_uuid2node, same uuid has different socket.
				if (m_hash_uuid2node.find(pClientNode->uuid())!=m_hash_uuid2node.end())
					if (m_hash_uuid2node[pClientNode->uuid()]==pClientNode)
					{
						m_hash_uuid2node.erase(pClientNode->uuid());
						broadcast_client_uuid(pClientNode->uuid(),false);
					}
			}

			pClientNode->bTermSet = true;
            disconnect (pClientNode,&st_clientNode_baseTrans::evt_SendDataToClient,m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_SendDataToClient);
			disconnect (pClientNode,&st_clientNode_baseTrans::evt_close_client,m_pThreadEngine,&ZPNetwork::zp_net_Engine::KickClients);
			disconnect (pClientNode,&st_clientNode_baseTrans::evt_Message,this,&st_client_table::evt_Message);
            nodeReadyToDel(pClientNode);
			//qDebug()<<QString("%1(ref %2) Node Push in queue.\n").arg((unsigned int)pClientNode).arg(pClientNode->ref());
		}
        m_hash_mutex.unlock();
	}

	//some data arrival
	void  st_client_table::on_evt_Data_recieved(QObject *  clientHandle,QByteArray  datablock )
	{
		//Push Clients to nodes if it is not exist
		bool nHashContains = false;
		st_clientNode_baseTrans * pClientNode = 0;
		m_hash_mutex.lock();
		nHashContains = (m_hash_sock2node.find(clientHandle)!=m_hash_sock2node.end())?true:false;
		if (false==nHashContains)
		{
            if(datablock.length() < 16) //数据包太短，格式不正确
            {
                qWarning() << __FUNCTION__ << "packet format error" << datablock.length();
                return;
            }
            uint8_t dev_id[8];
            uint8_t *ptr = (uint8_t*)datablock.data();
            memcpy(dev_id,&ptr[5],7);
            st_packet_tool tool;
            quint16 pos = 0;
            quint64 source_id = tool.get8bytes(dev_id,&pos) >> 8;
            qDebug() << __FUNCTION__ << __LINE__ << source_id;
            st_clientNode_baseTrans * pNode = findNodeFromToBeDel(source_id);
            if(pNode != nullptr)
            {
                removeNodeFromToBeDel(pNode);
                pNode->setClientSock(clientHandle);
            }
            else
            {
                pNode = new st_clientNodeAppLayer(this,clientHandle,0);
            }
			//using queued connection of send and revieve;
            connect (pNode,&st_clientNode_baseTrans::evt_SendDataToClient,m_pThreadEngine,&ZPNetwork::zp_net_Engine::evt_SendDataToClient,Qt::QueuedConnection);
            connect (pNode,&st_clientNode_baseTrans::evt_close_client,m_pThreadEngine,&ZPNetwork::zp_net_Engine::KickClients,Qt::QueuedConnection);
            connect (pNode,&st_clientNode_baseTrans::evt_Message,this,&st_client_table::evt_Message,Qt::QueuedConnection);
            m_hash_sock2node[clientHandle] = pNode;
			nHashContains = true;
            pClientNode = pNode;
		}
		else
        {
            pClientNode =  m_hash_sock2node[clientHandle];
		}

		assert(nHashContains!=0 && pClientNode !=0);


		int nblocks =  pClientNode->push_new_data(datablock);
		if (nblocks<=1)
			m_pTaskEngine->pushTask(pClientNode);
		m_hash_mutex.unlock();

	}

	//a block of data has been successfuly sent
	void  st_client_table::on_evt_Data_transferred(QObject *   /*clientHandle*/,qint64 /*bytes sent*/)
	{

	}

	//this event indicates new svr successfully hand-shaked.
	void st_client_table::on_evt_NewSvrConnected(QString  svrHandle)
	{
#if 0
		//Send All Client UUIDs to new Svr
		m_hash_mutex.lock();
		QList<quint32> uuids;
		for(std::unordered_map<quint32,st_clientNode_baseTrans *>::iterator p =  m_hash_uuid2node.begin();
			p!=m_hash_uuid2node.end();p++)
			uuids.push_back((*p).first);
		int nNodeSz = uuids.size();
		if (nNodeSz>0)
		{
			int nMsgLen = sizeof(EXAMPLE_CROSSSVR_MSG::tag_msgHearder) +  nNodeSz * sizeof(quint32);
			QByteArray array(nMsgLen,0);
			EXAMPLE_CROSSSVR_MSG * pMsg = (EXAMPLE_CROSSSVR_MSG *) array.data();
			pMsg->header.Mark = 0x4567;
			pMsg->header.version = 1;
			pMsg->header.messageLen = nNodeSz * sizeof(quint32);
			pMsg->header.mesageType = 0x01;
			int ct = -1;
			foreach (quint32 uuid,uuids)
				pMsg->payload.uuids[++ct] = uuid;
			m_pCluster->SendDataToRemoteServer(svrHandle,array);
		}
		m_hash_mutex.unlock();
		emit evt_Message(this,tr("Send Initial UUIDs to Remote Svr:") + svrHandle);
#endif
	}
    void st_client_table::broadcast_client_uuid(quint64 uuid, bool bActive)
	{
#if 0
		QStringList svrs = m_pCluster->SvrNames();
		if (svrs.empty()==false)
		{
			int nMsgLen = sizeof(EXAMPLE_CROSSSVR_MSG::tag_msgHearder) +  sizeof(quint32);
			QByteArray array(nMsgLen,0);
			EXAMPLE_CROSSSVR_MSG * pMsg = (EXAMPLE_CROSSSVR_MSG *) array.data();
			pMsg->header.Mark = 0x4567;
			pMsg->header.version = 1;
			pMsg->header.messageLen = sizeof(quint32);
			pMsg->header.mesageType = bActive==true?0x01:0x02;
			pMsg->payload.uuids[0] = uuid;
			foreach (QString svr,svrs)
				m_pCluster->SendDataToRemoteServer(svr,array);
		}
#endif
	}
    bool st_client_table::SendToNode(quint64 uuid, QByteArray  msg)
	{
		bool bres = false;
		m_hash_mutex.lock();
		if (m_hash_uuid2node.find(uuid)!=m_hash_uuid2node.end())
		{
			st_clientNode_baseTrans * pAppLayer = qobject_cast<st_clientNode_baseTrans *>(m_hash_uuid2node[uuid]);
			if (pAppLayer)
			{
				this->m_pThreadEngine->SendDataToClient(pAppLayer->sock(),msg);
				bres = true;
			}
		}
		m_hash_mutex.unlock();
		//emit evt_Message(this,tr("Recieved remote user-data to uuid:%1,DATA:%2").arg(uuid).arg(QString("HEX")+QString(msg.toHex())));
		return bres;
	}

	//this event indicates a client disconnected.
	void st_client_table::on_evt_NewSvrDisconnected(QString  svrHandle)
	{
		//remove all client-maps belongs to this server.
		this->cross_svr_del_uuids(svrHandle,NULL,0);
		emit evt_Message(this,"Svr DisConnected. " + svrHandle);
	}

	//some data arrival
	void st_client_table::on_evt_RemoteData_recieved(QString  svrHandle,QByteArray  array )
	{
		emit evt_Message(this,tr("Recieved %1 bytes Msg from ").arg(array.length()) + svrHandle);
	}

	//a block of data has been successfuly sent
	void st_client_table::on_evt_RemoteData_transferred(QObject *  /*svrHandle*/,qint64 /*bytes sent*/)
	{

	}
	ZP_Cluster::zp_ClusterNode * st_client_table::cross_svr_node_factory(
			ZP_Cluster::zp_ClusterTerm * pTerm,
			QObject * psock,
			QObject * parent)
	{
#if 0
		st_cross_svr_node * pNode = new st_cross_svr_node(pTerm,psock,parent);
		pNode->setClientTable(this);
		//connect (pNode,&st_cross_svr_node::evt_SendToNode,this,&st_client_table::SendToNode,Qt::QueuedConnection);
		return pNode;
#endif
        return NULL;
	}
	//reg new uuids in m_hash_remoteClient2SvrName
    void st_client_table::cross_svr_add_uuids(QString  svrname, quint64 *pUUIDs, int nUUIDs)
	{
		m_mutex_cross_svr_map.lock();
		for (int i=0;i<nUUIDs;i++)
			m_hash_remoteClient2SvrName[pUUIDs[i]] = svrname;
		m_mutex_cross_svr_map.unlock();
		emit evt_Message(this,tr("Recieved remote %1 client uuid(s) from svr ").arg(nUUIDs) + svrname);
	}

	//del uuids in m_hash_remoteClient2SvrName, pUUIDs =0 means del all uuids belong to svrname
    void st_client_table::cross_svr_del_uuids(QString  svrname,quint64 * pUUIDs , int nUUIDs)
	{
		m_mutex_cross_svr_map.lock();
		if (pUUIDs==NULL)
		{
            QList<quint64> keys;
            for(std::unordered_map<quint64,QString>::iterator p =
				m_hash_remoteClient2SvrName.begin();
				p!=m_hash_remoteClient2SvrName.end();p++)
			{
				if ((*p).second == svrname )
					keys.push_back((*p).first);
			}
            foreach (quint64 key, keys)
			{
				m_hash_remoteClient2SvrName.erase(key);
			}
		}
		else
		{
			for (int i=0;i<nUUIDs;i++)
				m_hash_remoteClient2SvrName.erase(pUUIDs[i]);
		}
		m_mutex_cross_svr_map.unlock();
		emit evt_Message(this,tr("Removed remote %1 client uuid(s) from svr ").arg(nUUIDs) + svrname);
	}
	void st_client_table::cross_svr_send_data(QString  svrname,QByteArray  arr)
	{
#if 0
		int nMsgLen = sizeof(EXAMPLE_CROSSSVR_MSG::tag_msgHearder);
		QByteArray array(nMsgLen,0);
		EXAMPLE_CROSSSVR_MSG * pMsg = (EXAMPLE_CROSSSVR_MSG *) array.data();
		pMsg->header.Mark = 0x4567;
		pMsg->header.version = 1;
		pMsg->header.messageLen = arr.size();
		pMsg->header.mesageType = 0x03;
		array.append(arr);
		m_pCluster->SendDataToRemoteServer(svrname,array);
#endif
	}

    QString  st_client_table::cross_svr_find_uuid(quint64 uuid)
	{
		QString svr = "";
		m_mutex_cross_svr_map.lock();
		if (m_hash_remoteClient2SvrName.find(uuid)!=m_hash_remoteClient2SvrName.end())
			svr = m_hash_remoteClient2SvrName[uuid];
		m_mutex_cross_svr_map.unlock();
		return svr;
	}

	bool st_client_table::NeedRedirect(quint8 bufAddresses[/*64*/],quint16 * pnPort)
	{
		if (m_pCluster->clientNums()<m_nBalanceMax)
			return false;
		QString strServerName = m_pCluster->minPayloadServer(bufAddresses,pnPort);
		if (strServerName==m_pCluster->name())
			return false;
		return true;
	}

    void st_client_table::on_evt_DelNoActiveNodeTimeout()
    {
        //Try to delete objects
        qint64 currSec = QDateTime::currentSecsSinceEpoch();
        qint64 noActiveTime = 0;
        QList <st_clientNode_baseTrans *> toBedel;
        m_mutex_nodeToBeDel.lock();
        foreach(st_clientNode_baseTrans * pdelobj,m_nodeToBeDel)
        {
            noActiveTime = currSec - pdelobj->activeTime();
            if (pdelobj->ref() ==0 && noActiveTime > S_ACTIVE_TIMEOUT)
                toBedel.push_back(pdelobj);
            else
            {
                //qDebug()<<QString("%1(ref %2) Waiting in del queue...%3\n").arg((unsigned int)pdelobj).arg(pdelobj->ref()).arg(noActiveTime);
            }
        }
        foreach(st_clientNode_baseTrans * pdelobj,toBedel)
        {
            qDebug() << __FUNCTION__ << __LINE__ << "Del Node" << pdelobj->uuid();
            m_nodeToBeDel.removeAll(pdelobj);

            //qDebug()<<QString("%1(ref %2) deleting.\n").arg((unsigned int)pdelobj).arg(pdelobj->ref());
            pdelobj->deleteLater();

        }
        m_mutex_nodeToBeDel.unlock();
    }

    void st_client_table::nodeReadyToDel(st_clientNode_baseTrans *pNode)
    {
        if(pNode == nullptr)
            return;
        QMutexLocker autoLock(&m_mutex_nodeToBeDel);
        foreach(st_clientNode_baseTrans * pdelobj,m_nodeToBeDel)
        {
            if(pdelobj == pNode) //删除节点已存在，无需重新添加
                return;
        }
        emit evt_Message(this,tr("Socket Client ") + QString("%1").arg((unsigned int)((quint64)this)) + tr(" is dead, kick out."));
        m_pThreadEngine->KickClients(pNode->sock());
        m_nodeToBeDel.push_back(pNode);
    }

    st_clientNode_baseTrans *st_client_table::findNodeFromToBeDel(quint64 nId)
    {
        st_clientNode_baseTrans *pNode = nullptr;
        QMutexLocker autoLock(&m_mutex_nodeToBeDel);
        foreach (st_clientNode_baseTrans *pDelNode, m_nodeToBeDel) {
            if(pDelNode->uuid() == nId)
            {
                pNode = pDelNode;
                qDebug() << "Finded node" << pDelNode->uuid();
                break;
            }
        }
        return pNode;
    }

     void st_client_table::removeNodeFromToBeDel(st_clientNode_baseTrans *pNode)
     {
         if(pNode == nullptr)
             return;
         QMutexLocker autoLock(&m_mutex_nodeToBeDel);
         m_nodeToBeDel.removeAll(pNode);
     }

}

