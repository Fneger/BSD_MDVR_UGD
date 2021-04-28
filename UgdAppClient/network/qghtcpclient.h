#ifndef QGHTCPCLIENT_H
#define QGHTCPCLIENT_H
#include <QList>
#include <QTcpSocket>
#include "ssl_config.h"

class QGHTcpClient : public QTcpSocket
{
	Q_OBJECT

public:
	QGHTcpClient(QObject *parent,int nPayLoad = 4096);
	~QGHTcpClient();
    quint64 uuid();
	void geneGlobalUUID(QString  globalUuidFile);
private:

    quint64 m_uuid;	int m_nPayLoad;
	QList<QByteArray> m_buffer_sending;
	QList<qint64> m_buffer_sending_offset;
public slots:
	void some_data_sended(qint64);
	void SendData(QByteArray dtarray);

};

#if (ZP_WANTSSL!=0)
#include <QSslSocket>
class QGHSslClient : public QSslSocket
{
	Q_OBJECT

public:
	QGHSslClient(QObject *parent,int nPayLoad = 4096);
	~QGHSslClient();
    quint64 uuid();
	void geneGlobalUUID(QString  globalUuidFile);
private:

    quint64 m_uuid;
	int m_nPayLoad;
	QList<QByteArray> m_buffer_sending;
	QList<qint64> m_buffer_sending_offset;
public slots:
	void some_data_sended(qint64);
	void SendData(QByteArray dtarray);

};
#endif
#endif // QGHTCPCLIENT_H
