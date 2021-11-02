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
    //��ʼ����־�����
    bool initLogTableManager(QSqlDatabase db);
    QString getLogTableName(const QString &devNumber);
    QString getLogTableName(int devNumber);
    bool logTableExists(const QString &devNumber);
    void setLogTableExists(const QString &devNumber, bool bExists);
    //�ж����ݿ��Ƿ����
    bool schemataExists(QSqlDatabase db, const QString &dbName);
    //�ж����ݰ����Ƿ����
    bool tableExists(QSqlDatabase db, const QString &tableName);
    //�������ݱ�
    bool createTable(QSqlDatabase db, GS_BODY_TYPE nType);
    //ɾ�����ݱ�ֱ��ɾ���������ݺͽṹ�����ã�������������������
    bool deleteTable(QSqlDatabase db, GS_BODY_TYPE nType);
    //�����ݱ��в�������
    bool insertItem(QSqlDatabase db, GS_BODY_TYPE nType,const GS_DB_BODY_C &dbBody);
    //ɾ�����ݱ���ĳһ��
    bool deleteItem(QSqlDatabase db, GS_BODY_TYPE nType, const GS_DB_BODY_C &dbBody);
    //�������ݱ���ĳһ��
    bool updateItem(QSqlDatabase db, GS_BODY_TYPE nType,const GS_DB_BODY_C &dbBody);
    //����������ѯ���ݱ�ĳһ��
    bool queryItem(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBody);
    //��ȡ���ж��Ԫ��,���б���ṹ��ָ���ڴ�����ʹ�ú��ͷ�
    bool queryItems(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBodys, int startId = 0, int nItemCntMax = S_QUERY_ITEM_NUM_MAX);
    //���ݲ�����������ȡ��������
    bool queryItems(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBodys, const QString &queryCondition, int startId = 0, int nItemCntMax = S_QUERY_ITEM_NUM_MAX);
    //��ѯ��������������
    int queryMatchItemsCount(QSqlDatabase db, GS_BODY_TYPE nType, const QString &queryCondition);
    //��ȡ������������
    int queryItemsCount(QSqlDatabase db, GS_BODY_TYPE nType);
    QString errInfo();
    //�������ݱ���չ����
    void setExtTableName(const QString &name) { m_extTableName = name; }

private:
    QString strListToStrs(const QStringList &list);
    void putDb2Struct(GS_BODY_TYPE nType, GS_DB_BODY_C &body, const QSqlQuery &query);


signals:

public slots:

private:
    QString m_errInfo;
    QString m_extTableName; //���ݱ���չ����
    QList<LOG_TABLE_MAN_S> m_logTableManager;
    QMutex m_logManMutex;
};
}

#endif // DATABASETOOL_H
