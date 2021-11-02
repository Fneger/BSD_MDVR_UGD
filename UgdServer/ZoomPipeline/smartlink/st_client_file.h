#ifndef ST_CLIENT_FILE_H
#define ST_CLIENT_FILE_H

#include <QMutex>
#include <QString>
#include <QJsonDocument>
#include "st_message.h"
#include "st_client_table.h"
#include "st_global_def.h"

using namespace GsGlobalDef;

class st_client_table;
class QTimer;
namespace BdUgdServer {

class st_client_file
{
public:
    static st_client_file *Instance();
    //打开产品信息文件
    bool openProductJson(const QString &fileName, QJsonDocument &doc);
    //从数据库中获取产品信息
    bool openProductDb(const QString &fileName, QJsonDocument &doc);
    //保存产品信息文件
    bool saveProductJson(const QString &fileName,QJsonDocument &doc);
    //添加数据
    bool addDb(GS_BODY_TYPE nType, const GS_DB_BODY_C &body);
    //删除数据
    bool removeDb(GS_BODY_TYPE nType, const GS_DB_BODY_C &body);
    //更新数据库
    bool updateDb(GS_BODY_TYPE nType, const GS_DB_BODY_C &body);
    //查询数据库
    bool queryDb(GS_BODY_TYPE nType, GS_DB_BODY_C &body);
    bool queryDb(GS_BODY_TYPE nType, GS_DB_BODY_C &body, const QString &queryCondition);
    //删除产品
    bool removeProductDb(const QString &productName);
    //创建日志数据表
    bool createLogTableDb(const QString &name);
    //删除日志数据表，在产品删除后，暂不删除相关日志数据
    bool deleteLogTableDb(const QString &name);
    //添加日志数据
    bool addLogDb(const BODY_LOG_INFO_S &log);
    //查询日志数据,根据匹配条件查找
    bool queryLogDb(const QString &devNumber, GS_DB_BODY_C &body, const QString &queryCondition, int startId = 0, int nItemCntMax = S_QUERY_ITEM_NUM_MAX);
    //查询满足匹配条件的数据条数
    int queryLogMatchCountDb(const QString &devNumber, const QString &queryCondition);
    //打开用户授权信息文件
    bool openAuthUsers(QJsonDocument &doc);
    bool openAuthUsersDb(QJsonDocument &doc);
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
    void setClientTable(ExampleServer::st_client_table *pClient);
    //如果需要同步json到数据库
    void syncJson2Database();
    void putInfo2Json(GS_BODY_TYPE nType, const GS_DB_BODY_C &body, QJsonObject &infoObj);
    void putInfos2Json(GS_BODY_TYPE nType, const GS_DB_BODY_C &bodys, QJsonObject &infosObj);

private:
    void slot_bakTimerTimeout();


private:
    st_client_file();
    ~st_client_file();

    static st_client_file *S_pThis;
    //储存产品相关信息
    QMutex m_mProductMutex;
    QMutex m_mAuthList;
    QTimer *m_bakTimer; //定时备份
    ExampleServer::st_client_table * m_pClientTable;
};
}


#endif // ST_CLIENT_FILE_H
