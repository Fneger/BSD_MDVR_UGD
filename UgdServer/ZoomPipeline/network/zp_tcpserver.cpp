#include "zp_tcpserver.h"
#include <assert.h>
namespace ZPNetwork{
	ZP_TcpServer::ZP_TcpServer(QObject *parent )
        : QAsioTcpServer(10240,parent)
	{

	}

    void ZP_TcpServer::incomingConnection(Session * session)
	{
        emit evt_NewClientArrived(session);
	}
}
