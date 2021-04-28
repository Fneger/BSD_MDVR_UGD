#ifndef ST_CLIENT_FILE_H
#define ST_CLIENT_FILE_H

#include <QMutex>
#include <QString>
#include <QJsonDocument>
#include "st_message.h"

namespace BdUgdServer {
class st_client_file
{
public:
    static st_client_file *Instance();
    //打开产品信息文件
    bool openProductJson(QJsonDocument &doc);
    //保存产品信息文件
    bool saveProductJson(QJsonDocument &doc);
    //打开用户授权信息文件
    bool openAuthUsers(QJsonDocument &doc);
    //保存用户授权信息文件
    bool saveAuthUsers(QJsonDocument &doc);
    //添加授权用户
    bool addAuthUser(const USER_INFO_S &userInfo);
    //移除授权用户
    bool removeAuthUser(const USER_INFO_S &userInfo);
    //编辑用户信息
    bool editAuthUser(const USER_INFO_S &userInfo);
    //客户端登录
    bool loginClient(const BD_TMN_LOGIN_INFO_S loginInfo, USER_INFO_S &userInfo);

private:
    st_client_file();

    static st_client_file *S_pThis;
    //储存产品相关信息
    QString m_sProductInfosJson;
    QMutex m_mProductMutex;
    QMutex m_mAuthList;
};
}


#endif // ST_CLIENT_FILE_H
