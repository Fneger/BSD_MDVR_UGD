#ifndef ST_CLIENT_FILE_H
#define ST_CLIENT_FILE_H

#include <QMutex>
#include <QString>
#include <QJsonDocument>
#include "st_message.h"

class QTimer;
namespace BdUgdServer {
class st_client_file
{
public:
    static st_client_file *Instance();
    //�򿪲�Ʒ��Ϣ�ļ�
    bool openProductJson(const QString &fileName, QJsonDocument &doc);
    //�����Ʒ��Ϣ�ļ�
    bool saveProductJson(const QString &fileName,QJsonDocument &doc);
    //���û���Ȩ��Ϣ�ļ�
    bool openAuthUsers(QJsonDocument &doc);
    //�����û���Ȩ��Ϣ�ļ�
    bool saveAuthUsers(QJsonDocument &doc);
    //�����Ȩ�û�
    bool addAuthUser(const USER_INFO_S &userInfo);
    //�Ƴ���Ȩ�û�
    bool removeAuthUser(const USER_INFO_S &userInfo);
    //�༭�û���Ϣ
    bool editAuthUser(const USER_INFO_S &userInfo);
    //�ͻ��˵�¼
    bool loginClient(const BD_TMN_LOGIN_INFO_S loginInfo, USER_INFO_S &userInfo);

private:
    void slot_bakTimerTimeout();

private:
    st_client_file();
    ~st_client_file();

    static st_client_file *S_pThis;
    //�����Ʒ�����Ϣ
    QMutex m_mProductMutex;
    QMutex m_mAuthList;
    QTimer *m_bakTimer; //��ʱ����
};
}


#endif // ST_CLIENT_FILE_H
