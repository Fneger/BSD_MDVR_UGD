#ifndef DATABASETOOL_H
#define DATABASETOOL_H

#include <QObject>
#include <QMutex>
#include <QSqlDatabase>
#include "st_message.h"
#include "st_global_def.h"

using namespace GsGlobalDef;
using namespace BdUgdServer;

namespace ZPDatabase {

class DatabaseTool : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseTool(QObject *parent = nullptr);
    static DatabaseTool *Instance();
    //初始化日志表管理
    bool initLogTableManager(QSqlDatabase db);
    QString getLogTableName(const QString &devNumber);
    QString getLogTableName(int devNumber);
    bool logTableExists(const QString &devNumber);
    void setLogTableExists(const QString &devNumber, bool bExists);
    //判断数据库是否存在
    bool schemataExists(QSqlDatabase db, const QString &dbName);
    //判断数据包表是否存在
    bool tableExists(QSqlDatabase db, const QString &tableName);
    //创建数据表
    bool createTable(QSqlDatabase db, GS_BODY_TYPE nType);
    //删除数据表，直接删除表中数据和结构，慎用！！！！！！！！！！
    bool deleteTable(QSqlDatabase db, GS_BODY_TYPE nType);
    //往数据表中插入数据
    bool insertItem(QSqlDatabase db, GS_BODY_TYPE nType,const GS_DB_BODY_C &dbBody);
    //删除数据表中某一项
    bool deleteItem(QSqlDatabase db, GS_BODY_TYPE nType, const GS_DB_BODY_C &dbBody);
    //更新数据表中某一项
    bool updateItem(QSqlDatabase db, GS_BODY_TYPE nType,const GS_DB_BODY_C &dbBody);
    //根据条件查询数据表某一项
    bool queryItem(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBody);
    //获取表中多个元素,其中保存结构体指针内存需在使用后释放
    bool queryItems(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBodys, int startId = 0, int nItemCntMax = S_QUERY_ITEM_NUM_MAX);
    //根据查找条件，获取表中数据
    bool queryItems(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBodys, const QString &queryCondition, int startId = 0, int nItemCntMax = S_QUERY_ITEM_NUM_MAX);
    //查询满足条件的条数
    int queryMatchItemsCount(QSqlDatabase db, GS_BODY_TYPE nType, const QString &queryCondition);
    //获取表中数据条数
    int queryItemsCount(QSqlDatabase db, GS_BODY_TYPE nType);
    QString errInfo();
    //设置数据表扩展名称
    void setExtTableName(const QString &name) { m_extTableName = name; }

private:
    QString strListToStrs(const QStringList &list);
    void putDb2Struct(GS_BODY_TYPE nType, GS_DB_BODY_C &body, const QSqlQuery &query);


signals:

public slots:

private:
    QString m_errInfo;
    QString m_extTableName; //数据表扩展名称
    QList<LOG_TABLE_MAN_S> m_logTableManager;
    QMutex m_logManMutex;
};
}

#endif // DATABASETOOL_H
