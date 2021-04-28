#ifndef ZP_TcpServer_H
#define ZP_TcpServer_H

//#include <QTcpServer>
#include "qasiotcpserver.h"

namespace ZPNetwork{
    class ZP_TcpServer : public QAsioTcpServer
	{
		Q_OBJECT

	public:
        ZP_TcpServer(QObject *parent = nullptr);
	protected:
        void incomingConnection(Session *session);
	signals:
        void evt_NewClientArrived(void * session);
	};
}
#endif // ZP_TcpServer_H
