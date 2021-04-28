#ifndef CLIENTNODE_H
#define CLIENTNODE_H

#include <QObject>
#include <QMutex>
#include <QSet>
#include "st_clientnode_basetrans.h"
#include "st_message.h"
//#include "st_msg_applayer.h"
using namespace BdUgdServer;
class QFile;
namespace ExampleServer{
    class st_client_table;
	class st_clientNodeAppLayer : public st_clientNode_baseTrans
	{
		Q_OBJECT
		friend class st_clien_table;
	public:
		explicit st_clientNodeAppLayer(st_client_table * pClientTable, QObject * pClientSock,QObject *parent = 0);
    protected:
		//!in App-Layer, it Split the messages into dealers.
        int deal_current_message_block();
        bool clientLogout();
        bool reponseRequest(const char *rJson,QJsonDocument &ackDoc);
		//data items

    private:
        //获取系统时间
        time_t getSysTime(uint8_t *datetime,uint8_t len);
        //接收文件数据
        bool receiveFileData(BD_TRANSFER_FILE_DATA_S &body);
        //关闭正在上传文件
        void closeUploadFile();
        //创建所要上传文件
        bool openUploadFile(const QString &fileName);
        //发送文件数据
        bool sendFileData(BD_TMN_DOWNLOAD_FILE_DATA_S &body,BD_TRANSFER_FILE_DATA_S &tBody);
        //打开所要下载文件
        bool openDownloadFile(const QString &fileName);
        //关闭正在下载文件
        void closeDownloadFile();
        //创建多级目录
        QString mkMutiDir(const QString path);
        //移动文件
        bool moveFile(const QString srcFullPath,const QString dstFullPath);

	protected:
        QMutex m_mutex_equipID;
		//log in
        bool m_bLoggedIn;
        QFile *m_uploadFileHandle;
        BD_REQUEST_UPLOAD_FILE_S m_uploadFileInfo;
        uint32_t m_receivedBytes;   //已接收字节数
        QFile *m_downloadFileHandle;
        uint32_t m_sentBytes;
        uint64_t m_totalReceivedTestBytes; //所接收到总测试字节数
        USER_INFO_S m_userInfo;
	};

}

#endif // CLIENTNODE_H
