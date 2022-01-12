#include "databasetool.h"
#include <QMutex>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>


static const QString S_ERR_STR_NOT_OPEN = "Database(%1) is not open";

namespace ZPDatabase {

using namespace BdUgdServer;

QMap<int, LOG_TABLE_MAN_S> DatabaseTool::m_logTableManager;
QMap<int, LOG_TABLE_MAN_S> DatabaseTool::m_tmnInfoLogTableManager;
QMutex DatabaseTool::m_logManMutex;

DatabaseTool::DatabaseTool(QObject *parent) : QObject(parent)
{
    m_errInfo = "";
}

bool DatabaseTool::initLogTableManager(QSqlDatabase db, int devNum, bool isTmnInfo)
{
    DatabaseTool dbTool;
    int tableNum = devNum / S_LOG_TABLE_DEV_MAX;
    if(isTmnInfo)
    {
        if(m_tmnInfoLogTableManager.contains(tableNum))
            return true;
        LOG_TABLE_MAN_S man;
        man.tableName = dbTool.getLogTableName(devNum);
        man.isCreated = dbTool.tableExists(db, man.tableName + "_" + S_LOG_INFO_TABLE_NAME);
        if(man.isCreated)
        {
            man.savedItemNum = dbTool.queryItemsCount(db, TB_LOG_INFO_E);
        }
        else
            man.savedItemNum = 0;
        man.startDevNumber = devNum;
        man.endDevNumber = devNum + S_LOG_TABLE_DEV_MAX;
        man.isFull = false;
        m_tmnInfoLogTableManager[tableNum] = man;
    }
    else
    {
        if(m_logTableManager.contains(tableNum))
            return true;
        LOG_TABLE_MAN_S man;
        man.tableName = dbTool.getLogTableName(devNum);
        man.isCreated = dbTool.tableExists(db, man.tableName + "_" + S_LOG_INFO_TABLE_NAME);
        if(man.isCreated)
        {
            man.savedItemNum = dbTool.queryItemsCount(db, TB_LOG_INFO_E);
        }
        else
            man.savedItemNum = 0;
        man.startDevNumber = devNum;
        man.endDevNumber = devNum + S_LOG_TABLE_DEV_MAX;
        man.isFull = false;
        m_logTableManager[tableNum] = man;
    }

    return true;
}

QString DatabaseTool::getLogTableName(const QString &devNumber)
{
    int num = devNumber.toInt();
    return getLogTableName(num);
}

QString DatabaseTool::getLogTableName(int devNumber)
{
    int startNum = devNumber/S_LOG_TABLE_DEV_MAX;
    return QString("%1").arg(startNum, 8, 10, QChar('0'));
}

bool DatabaseTool::logTableExists(QSqlDatabase db, const QString &devNumber, bool isTmnInfo)
{
    bool res = false;
    m_logManMutex.lock();
    if(isTmnInfo)
    {
        int devNum = devNumber.toInt();
        initLogTableManager(db, devNum, isTmnInfo);
        int tableNum = devNum / S_LOG_TABLE_DEV_MAX;
        if(!m_tmnInfoLogTableManager.contains(tableNum))
        {
            m_logManMutex.unlock();
            return res;
        }

        res = m_tmnInfoLogTableManager[tableNum].isCreated;
    }
    else
    {
        int devNum = devNumber.toInt();
        initLogTableManager(db, devNum);
        int tableNum = devNum / S_LOG_TABLE_DEV_MAX;
        if(!m_logTableManager.contains(tableNum))
        {
            m_logManMutex.unlock();
            return res;
        }

        res = m_logTableManager[tableNum].isCreated;
    }
    m_logManMutex.unlock();
    return res;
}

void DatabaseTool::setLogTableExists(QSqlDatabase db, const QString &devNumber, bool bExists, bool isTmnInfo)
{
    if(isTmnInfo)
    {
        m_logManMutex.lock();
        int devNum = devNumber.toInt();
        initLogTableManager(db, devNum, isTmnInfo);
        int tableNum = devNumber.toInt() / S_LOG_TABLE_DEV_MAX;
        if(!m_tmnInfoLogTableManager.contains(tableNum))
        {
            m_logManMutex.unlock();
            return;
        }

        LOG_TABLE_MAN_S man = m_tmnInfoLogTableManager[tableNum];
        man.isCreated = bExists;
        m_tmnInfoLogTableManager[tableNum]  = man;
        m_logManMutex.unlock();
    }
    else
    {
        m_logManMutex.lock();
        int devNum = devNumber.toInt();
        initLogTableManager(db, devNum);
        int tableNum = devNumber.toInt() / S_LOG_TABLE_DEV_MAX;
        if(!m_logTableManager.contains(tableNum))
        {
            m_logManMutex.unlock();
            return;
        }

        LOG_TABLE_MAN_S man = m_logTableManager[tableNum];
        man.isCreated = bExists;
        m_logTableManager[tableNum]  = man;
        m_logManMutex.unlock();
    }
}

QString DatabaseTool::errInfo()
{
    return m_errInfo;
}

QString DatabaseTool::strListToStrs(const QStringList &list)
{
    QString listStr;
    if(!list.isEmpty())
    {
        foreach (QString str, list) {
            if(str == "")
                continue;
            listStr +=str + ",";
        }
    }
    return listStr;
}

void DatabaseTool::putDb2Struct(GS_BODY_TYPE nType, GS_DB_BODY_C &body, const QSqlQuery &query)
{
    //此处字段顺序必须与数据库中定义顺序对应
    switch (nType) {
    case TB_USERS_INFO_E:
    {
        USER_INFO_S info;
        info.id = query.value(0).toInt();
        info.name = query.value(1).toString();
        info.auth = query.value(2).toInt();
        info.date = query.value(3).toString();
        info.password = query.value(4).toString();
        info.products = query.value(5).toString().split(",");
        info.log_path = query.value(6).toString();
        info.auth_bits = query.value(7).toLongLong();
        info.message = query.value(8).toString();
        body = info;
    }
        break;
    case TB_PRODUCTS_INFO_E:
    {
        BODY_PRODUCT_INFO_S info;
        info.id = query.value(0).toInt();
        info.name = query.value(1).toString();
        info.desp = query.value(2).toString();
        info.addDate = query.value(3).toString();
        info.modifyDate = query.value(4).toString();
        info.savePath = query.value(5).toString();
        body = info;
    }
        break;
    case TB_CRC_VERSION_INFO_E:
    {
        BODY_CRC_VERSION_INFO_S info;
        info.id = query.value(0).toInt();
        info.productName = query.value(1).toString();
        info.name = query.value(2).toString();
        info.date = query.value(3).toString();
        info.isDefaultVersion = query.value(4).toBool();
        info.message = query.value(5).toString();
        info.fileName = query.value(6).toString();
        info.fileSize = query.value(7).toUInt();
        info.custom = query.value(8).toString();
        body = info;
    }
        break;
    case TB_MCU_VERSION_INFO_E:
    {
        BODY_MCU_VERSION_INFO_S info;
        info.id = query.value(0).toInt();
        info.productName = query.value(1).toString();
        info.mcuTypeName = query.value(2).toString();
        info.name = query.value(3).toString();
        info.date = query.value(4).toString();
        info.isDefaultVersion = query.value(5).toBool();
        info.message = query.value(6).toString();
        info.fileName = query.value(7).toString();
        info.fileSize = query.value(8).toUInt();
        body = info;
    }
        break;
    case TB_GROUP_VERSION_INFO_E:
    {
        BODY_GROUP_VERSION_INFO_S info;
        info.id = query.value(0).toInt();
        info.productName = query.value(1).toString();
        info.name = query.value(2).toString();
        info.date = query.value(3).toString();
        info.isDefaultVersion = query.value(4).toBool();
        info.message = query.value(5).toString();
        info.savePath = query.value(6).toString();
        info.subFileName = query.value(7).toString();
        info.subFileDate = query.value(8).toString();
        info.subFilePath = query.value(9).toString();
        info.subFileSize = query.value(10).toUInt();
        info.subFileMessage = query.value(11).toString();
        body = info;
    }
        break;
    case TB_LOG_INFO_E:
    {
        BODY_LOG_INFO_S info;
        info.id = query.value(0).toInt();
        info.productName = query.value(1).toString();
        info.devNumber = query.value(2).toString();
        info.imeiNumber = query.value(3).toString();
        info.fwVersion = query.value(4).toString();
        info.type = (GS_LOG_TYPE)query.value(5).toInt();
        info.subtype = query.value(6).toInt();
        info.result = (GS_LOG_RESULT)query.value(7).toInt();
        info.dateTime = query.value(8).toDateTime();
        info.serverDateTime = query.value(9).toDateTime();
        info.message = query.value(10).toString();
        body = info;
    }
        break;
    case TB_TMN_INFO_LOG_INFO_E:
    {
        BODY_TMN_INFO_S info;
        info.id = query.value(0).toInt();
        info.productName = query.value(1).toString();
        info.deviceNum = query.value(2).toString();
        info.phoneNumber = query.value(3).toString();
        info.licenseNum = query.value(4).toString();
        info.videoId = query.value(5).toString();
        info.terminalId = query.value(6).toString();
        info.imeiNumber = query.value(7).toString();
        info.crcVersion = query.value(8).toString();
        info.mcuVersion = query.value(9).toString();
        info.authorizationStatus = query.value(10).toInt();
        info.serverInfos.clear();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(query.value(11).toString().toUtf8(),&err);
        if (QJsonParseError::NoError != err.error)
        {
            QJsonArray serverInfos = doc.array();
            foreach (QJsonValue val, serverInfos) {
                QJsonObject serverInfoObj = val.toObject();
                if(!serverInfoObj.contains("IpAddr") || !serverInfoObj.contains("Port"))
                    break;
                BODY_SERVER_INFO_S serverInfo;
                serverInfo.ipAddr = serverInfoObj["IpAddr"].toString();
                serverInfo.port = serverInfoObj["Port"].toInt();
                info.serverInfos << serverInfo;
            }
        }
        info.dateTime = query.value(8).toDateTime();
        info.serverDateTime = query.value(9).toDateTime();
        body = info;
    }
        break;
    }
}

bool DatabaseTool::schemataExists(QSqlDatabase db, const QString &dbName)
{
    bool res = false;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        QString sql = "SELECT * FROM information_schema.SCHEMATA where SCHEMA_NAME=?;";
        query.prepare(sql);
        query.addBindValue(dbName);
        if(query.exec() == true)
        {
            if (query.next())
            {
                res = true;
            }
            else
            {
                m_errInfo = QString("Database(%1) does not exist").arg(dbName);
            }
        }
        else
        {
            m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
            //db.close();
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::tableExists(QSqlDatabase db, const QString &tableName)
{
    bool res = false;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        QString sql = "SELECT table_name FROM information_schema.TABLES WHERE table_schema=? and table_name =?;";
        query.prepare(sql);
        query.addBindValue(db.databaseName());
        query.addBindValue(tableName);
        if(query.exec() == true)
        {
            if (query.next())
            {
                res = true;
            }
            else
            {
                m_errInfo = QString("Database table(%1) does not exist").arg(tableName);
            }
        }
        else
        {
            m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
            //db.close();
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::createTable(QSqlDatabase db, GS_BODY_TYPE nType, const QString &extTableName)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__;
        return res;
    }
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);
        QString tableName;
        QString sql;
        switch (nType) {
        case TB_USERS_INFO_E:
            tableName = S_USERS_INFO_TABLE_NAME;
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` INT UNSIGNED AUTO_INCREMENT,"
                          "`name` VARCHAR(64) NOT NULL,"
                          "`auth` INT UNSIGNED NOT NULL,"
                          "`date` VARCHAR(32),"
                          "`password` VARCHAR(32) NOT NULL,"
                          "`products` VARCHAR(4096),"
                          "`log_path` VARCHAR(128),"
                          "`auth_bits` BIGINT,"
                          "`message` VARCHAR(4096),"
                          "PRIMARY KEY ( `id` )"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        case TB_PRODUCTS_INFO_E:
            tableName = S_PRODUCTS_INFO_TABLE_NAME;
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` INT UNSIGNED AUTO_INCREMENT,"
                          "`name` VARCHAR(64) NOT NULL,"
                          "`desp` VARCHAR(4096),"
                          "`add_date` VARCHAR(32),"
                          "`modify_date` VARCHAR(32),"
                          "`save_path` VARCHAR(128) NOT NULL,"
                          "PRIMARY KEY ( `id` )"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        case TB_CRC_VERSION_INFO_E:
            tableName = S_CRC_VERSION_INFO_TABLE_NAME;
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` INT UNSIGNED AUTO_INCREMENT,"
                          "`product_name` VARCHAR(64) NOT NULL,"
                          "`name` VARCHAR(64) NOT NULL,"
                          "`date` VARCHAR(32),"
                          "`is_default_version` BOOLEAN NOT NULL,"
                          "`message` VARCHAR(4096),"
                          "`file_name` VARCHAR(128),"
                          "`file_size` INT UNSIGNED NOT NULL,"
                          "`custom` VARCHAR(16),"
                          "PRIMARY KEY ( `id` ),"
                          "INDEX `product_name` (`product_name`)"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        case TB_MCU_VERSION_INFO_E:
            tableName = S_MCU_VERSION_INFO_TABLE_NAME;
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` INT UNSIGNED AUTO_INCREMENT,"
                          "`product_name` VARCHAR(64) NOT NULL,"
                          "`mcu_type_name` VARCHAR(64) NOT NULL,"
                          "`name` VARCHAR(64) NOT NULL,"
                          "`date` VARCHAR(32),"
                          "`is_default_version` BOOLEAN NOT NULL,"
                          "`message` VARCHAR(4096),"
                          "`file_name` VARCHAR(128),"
                          "`file_size` INT UNSIGNED NOT NULL,"
                          "PRIMARY KEY ( `id` ),"
                          "INDEX `product_name` (`product_name`),"
                          "INDEX `mcu_type_name` (`mcu_type_name`)"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        case TB_GROUP_VERSION_INFO_E:
            tableName = S_GROUP_VERSION_INFO_TABLE_NAME;
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` INT UNSIGNED AUTO_INCREMENT,"
                          "`product_name` VARCHAR(64) NOT NULL,"
                          "`name` VARCHAR(64) NOT NULL,"
                          "`date` VARCHAR(32),"
                          "`is_default_version` BOOLEAN NOT NULL,"
                          "`message` VARCHAR(4096),"
                          "`save_path` VARCHAR(128) NOT NULL,"
                          "`sub_file_name` VARCHAR(128),"
                          "`sub_file_date` VARCHAR(32),"
                          "`sub_file_path` VARCHAR(128),"
                          "`sub_file_size` INT UNSIGNED,"
                          "`sub_file_message` VARCHAR(4096),"
                          "PRIMARY KEY ( `id` ),"
                          "INDEX `product_name` (`product_name`)"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        case TB_LOG_INFO_E:
            tableName = QString("%1_%2").arg(extTableName).arg(S_LOG_INFO_TABLE_NAME);
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` BIGINT(20) AUTO_INCREMENT,"
                          "`product_name` VARCHAR(64) NOT NULL,"
                          "`dev_number` VARCHAR(64) NOT NULL,"
                          "`imei_number` VARCHAR(64),"
                          "`fw_version` VARCHAR(128) NOT NULL,"
                          "`type` INT UNSIGNED NOT NULL,"
                          "`subtype` INT UNSIGNED NOT NULL,"
                          "`result` INT UNSIGNED NOT NULL,"
                          "`date_time` DATETIME NOT NULL,"
                          "`server_date_time` DATETIME NOT NULL,"
                          "`message` VARCHAR(4096) NOT NULL,"
                          "PRIMARY KEY ( `id` ),"
                          "INDEX `product_name` (`product_name`),"
                          "INDEX `dev_number` (`dev_number`),"
                          "INDEX `type` (`type`),"
                          "INDEX `subtype` (`subtype`),"
                          "INDEX `result` (`result`)"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        case TB_TMN_INFO_LOG_INFO_E:
            tableName = QString("%1_%2").arg(extTableName).arg(S_TMN_INFO_LOG_INFO_TABLE_NAME);
            sql = QString("CREATE TABLE IF NOT EXISTS `%1` ("
                          "`id` BIGINT(20) AUTO_INCREMENT,"
                          "`product_name` VARCHAR(64) NOT NULL,"
                          "`device_num` VARCHAR(64) NOT NULL,"
                          "`phone_number` VARCHAR(64),"
                          "`license_num` VARCHAR(64),"
                          "`video_id` VARCHAR(64),"
                          "`terminal_id` VARCHAR(64),"
                          "`imei_number` VARCHAR(64),"
                          "`crc_version` VARCHAR(128),"
                          "`mcu_version` VARCHAR(64),"
                          "`authorization_status` INT UNSIGNED NOT NULL,"
                          "`server_infos` VARCHAR(1024),"
                          "`date_time` DATETIME NOT NULL,"
                          "`server_date_time` DATETIME NOT NULL,"
                          "PRIMARY KEY ( `id` ),"
                          "INDEX `product_name` (`product_name`),"
                          "INDEX `dev_number` (`dev_number`),"
                          "INDEX `type` (`type`),"
                          "INDEX `subtype` (`subtype`),"
                          "INDEX `result` (`result`)"
                          ")ENGINE=InnoDB DEFAULT CHARSET=utf8;").arg(tableName);
            break;
        }
        query.prepare(sql);
        if(query.exec() == true)
        {
            if (tableExists(db, tableName))
            {
                res = true;
                qDebug() << QString("Create table(%1) Ok").arg(tableName);
            }
            else
            {
                m_errInfo = QString("Create table(%1) failed").arg(tableName);
            }
        }
        else
        {
            m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
            //db.close();
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::deleteTable(QSqlDatabase db, GS_BODY_TYPE nType, const QString &extTableName)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__;
        return res;
    }
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);
        QString tableName;
        QString sql;
        switch (nType) {
        case TB_USERS_INFO_E:
            tableName = S_USERS_INFO_TABLE_NAME;
            break;
        case TB_PRODUCTS_INFO_E:
            tableName = S_PRODUCTS_INFO_TABLE_NAME;
            break;
        case TB_CRC_VERSION_INFO_E:
            tableName = S_CRC_VERSION_INFO_TABLE_NAME;
            break;
        case TB_MCU_VERSION_INFO_E:
            tableName = S_MCU_VERSION_INFO_TABLE_NAME;
            break;
        case TB_GROUP_VERSION_INFO_E:
            tableName = S_GROUP_VERSION_INFO_TABLE_NAME;
            break;
        case TB_LOG_INFO_E:
            tableName = QString("%1_%2").arg(extTableName).arg(S_LOG_INFO_TABLE_NAME);
            break;
        case TB_TMN_INFO_LOG_INFO_E:
            tableName = QString("%1_%2").arg(extTableName).arg(S_TMN_INFO_LOG_INFO_TABLE_NAME);
            break;
        }
        sql = QString("DROP TABLE %1;").arg(tableName);
        query.prepare(sql);
        if(query.exec() == true)
        {
            res = true;
        }
        else
        {
            m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
            //db.close();
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::insertItem(QSqlDatabase db, GS_BODY_TYPE nType, const GS_DB_BODY_C &dbBody, const QString &extTableName)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" <<nType << __FUNCTION__;
        return res;
    }
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        QString sql;
        switch (nType) {
        case TB_USERS_INFO_E:
        {
             USER_INFO_S body =  boost::any_cast<USER_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1"
                          "(name, auth, date, password, products, log_path, auth_bits, message)"
                          "VALUES"
                          "(:name, :auth, :date, :password, :products, :log_path, :auth_bits, :message);")
                    .arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":name", body.name);
            query.bindValue(":auth", body.auth);
            query.bindValue(":date", body.date);
            query.bindValue(":password", body.password);
            query.bindValue(":products", strListToStrs(body.products));
            query.bindValue(":log_path", body.log_path);
            query.bindValue(":auth_bits", body.auth_bits);
            query.bindValue(":message", body.message);

        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
             BODY_PRODUCT_INFO_S body =  boost::any_cast<BODY_PRODUCT_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1"
                          "(name, desp, add_date, modify_date, save_path)"
                          "VALUES"
                          "(:name, :desp, :add_date, :modify_date, :save_path);")
                    .arg(S_PRODUCTS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":name", body.name);
            query.bindValue(":desp", body.desp);
            query.bindValue(":add_date", body.addDate);
            query.bindValue(":modify_date", body.modifyDate);
            query.bindValue(":save_path", body.savePath);
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
             BODY_CRC_VERSION_INFO_S body =  boost::any_cast<BODY_CRC_VERSION_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1"
                          "(product_name, name, date, is_default_version, message, file_name, file_size, custom)"
                          "VALUES"
                          "(:product_name, :name, :date, :is_default_version, :message, :file_name, :file_size, :custom);")
                    .arg(S_CRC_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":product_name", body.productName);
            query.bindValue(":name", body.name);
            query.bindValue(":date", body.date);
            query.bindValue(":is_default_version", body.isDefaultVersion);
            query.bindValue(":message", body.message);
            query.bindValue(":file_name", body.fileName);
            query.bindValue(":file_size", body.fileSize);
            query.bindValue(":custom", body.custom);
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
             BODY_MCU_VERSION_INFO_S body =  boost::any_cast<BODY_MCU_VERSION_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1"
                          "(product_name,mcu_type_name, name, date, is_default_version, message, file_name, file_size)"
                          "VALUES"
                          "(:product_name, :mcu_type_name, :name, :date, :is_default_version, :message, :file_name, :file_size);")
                    .arg(S_MCU_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":product_name", body.productName);
            query.bindValue(":mcu_type_name", body.mcuTypeName);
            query.bindValue(":name", body.name);
            query.bindValue(":date", body.date);
            query.bindValue(":is_default_version", body.isDefaultVersion);
            query.bindValue(":message", body.message);
            query.bindValue(":file_name", body.fileName);
            query.bindValue(":file_size", body.fileSize);

        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
             BODY_GROUP_VERSION_INFO_S body =  boost::any_cast<BODY_GROUP_VERSION_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1"
                          "(product_name, name, date, is_default_version, message, save_path, sub_file_name, sub_file_date, sub_file_path, sub_file_size, sub_file_message)"
                          "VALUES"
                          "(:product_name, :name, :date, :is_default_version, :message, :save_path, :sub_file_name, :sub_file_date, :sub_file_path, :sub_file_size, :sub_file_message);")
                    .arg(S_GROUP_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":product_name", body.productName);
            query.bindValue(":name", body.name);
            query.bindValue(":date", body.date);
            query.bindValue(":is_default_version", body.isDefaultVersion);
            query.bindValue(":message", body.message);
            query.bindValue(":save_path", body.savePath);
            query.bindValue(":sub_file_name", body.subFileName);
            query.bindValue(":sub_file_date", body.subFileDate);
            query.bindValue(":sub_file_path", body.subFilePath);
            query.bindValue(":sub_file_size", body.subFileSize);
            query.bindValue(":sub_file_message", body.subFileMessage);
        }
            break;
        case TB_LOG_INFO_E:
        {
            BODY_LOG_INFO_S body =  boost::any_cast<BODY_LOG_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1_%2"
                          "(product_name, dev_number, imei_number, fw_version, type, subtype, result, date_time, server_date_time, message)"
                          "VALUES"
                          "(:product_name, :dev_number, :imei_number, :fw_version, :type, :subtype, :result, :date_time, :server_date_time, :message);")
                    .arg(extTableName).arg(S_LOG_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":product_name", body.productName);
            query.bindValue(":dev_number", body.devNumber);
            query.bindValue(":imei_number", body.imeiNumber);
            query.bindValue(":fw_version", body.fwVersion);
            query.bindValue(":type", body.type);
            query.bindValue(":subtype", body.subtype);
            query.bindValue(":result", body.result);
            query.bindValue(":date_time", body.dateTime);
            query.bindValue(":server_date_time", body.serverDateTime);
            query.bindValue(":message", body.message);
        }
            break;
        case TB_TMN_INFO_LOG_INFO_E:
        {
            BODY_TMN_INFO_S body =  boost::any_cast<BODY_TMN_INFO_S>(dbBody);
            sql = QString("INSERT INTO %1_%2"
                          "(product_name, device_num, phone_number, license_num, video_id, terminal_id, imei_number, crc_version, mcu_version, authorization_status, "
                          "server_infos, date_time, server_date_time)"
                          "VALUES"
                          "(:product_name, :device_num, :phone_number, :license_num, :video_id, :terminal_id, :imei_number, :crc_version, :mcu_version, :authorization_status, "
                          ":server_infos, :date_time, :server_date_time);")
                    .arg(extTableName).arg(S_TMN_INFO_LOG_INFO_TABLE_NAME);
            query.prepare(sql);
            query.bindValue(":product_name", body.productName);
            query.bindValue(":device_num", body.deviceNum);
            query.bindValue(":phone_number", body.phoneNumber);
            query.bindValue(":license_num", body.licenseNum);
            query.bindValue(":video_id", body.videoId);
            query.bindValue(":terminal_id", body.terminalId);
            query.bindValue(":imei_number", body.imeiNumber);
            query.bindValue(":crc_version", body.crcVersion);
            query.bindValue(":mcu_version", body.mcuVersion);
            query.bindValue(":authorization_status", body.authorizationStatus);
            QJsonDocument doc;
            QJsonObject obj;
            QJsonArray objArr;

            foreach (BODY_SERVER_INFO_S info, body.serverInfos) {
                obj["IpAddr"] = info.ipAddr;
                obj["Port"] = info.port;
                objArr << obj;
            }
            doc.setArray(objArr);
            query.bindValue(":server_infos", QString(doc.toJson(QJsonDocument::Indented).data()));
            query.bindValue(":date_time", body.dateTime);
            query.bindValue(":server_date_time", body.serverDateTime);
        }
            break;
        }

        if(query.exec())
        {
            res = true;
        }
        else
        {
            m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
            //db.close();
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::deleteItem(QSqlDatabase db, GS_BODY_TYPE nType, const GS_DB_BODY_C &dbBody)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" <<nType << __FUNCTION__;
        return res;
    }
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        QString sql;
        switch (nType) {
        case TB_USERS_INFO_E:
        {
            USER_INFO_S body =  boost::any_cast<USER_INFO_S>(dbBody);
            sql = QString("DELETE FROM %1 WHERE name=?;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            BODY_PRODUCT_INFO_S body =  boost::any_cast<BODY_PRODUCT_INFO_S>(dbBody);
            sql = QString("DELETE FROM %1 WHERE name=?;").arg(S_PRODUCTS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            BODY_CRC_VERSION_INFO_S body =  boost::any_cast<BODY_CRC_VERSION_INFO_S>(dbBody);
            sql = QString("DELETE FROM %1 WHERE product_name=? AND name=?;").arg(S_CRC_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.productName);
            query.addBindValue(body.name);
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            BODY_MCU_VERSION_INFO_S body =  boost::any_cast<BODY_MCU_VERSION_INFO_S>(dbBody);
            if(!body.mcuTypeName.isEmpty() && body.name.isEmpty())
            {
                sql = QString("DELETE FROM %1 WHERE product_name=? AND mcu_type_name=?;").arg(S_MCU_VERSION_INFO_TABLE_NAME);
                query.prepare(sql);
                query.addBindValue(body.productName);
                query.addBindValue(body.mcuTypeName);
            }
            else if(!body.mcuTypeName.isEmpty() && !body.name.isEmpty())
            {
                sql = QString("DELETE FROM %1 WHERE product_name=? AND mcu_type_name=? AND name=?;").arg(S_MCU_VERSION_INFO_TABLE_NAME);
                query.prepare(sql);
                query.addBindValue(body.productName);
                query.addBindValue(body.mcuTypeName);
                query.addBindValue(body.name);
            }
            else if(body.mcuTypeName.isEmpty() && body.name.isEmpty())
            {
                sql = QString("DELETE FROM %1 WHERE product_name=?;").arg(S_MCU_VERSION_INFO_TABLE_NAME);
                query.prepare(sql);
                query.addBindValue(body.productName);
            }

        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            BODY_GROUP_VERSION_INFO_S body =  boost::any_cast<BODY_GROUP_VERSION_INFO_S>(dbBody);
            if(body.name.isEmpty() && body.subFileName.isEmpty())
            {
                sql = QString("DELETE FROM %1 WHERE product_name=?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME);
                query.prepare(sql);
                query.addBindValue(body.productName);
            }
            else if(!body.name.isEmpty() && !body.subFileName.isEmpty())
            {
                sql = QString("DELETE FROM %1 WHERE product_name=? AND name=? AND sub_file_name=?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME);
                query.prepare(sql);
                query.addBindValue(body.productName);
                query.addBindValue(body.name);
                query.addBindValue(body.subFileName);
            }
            else if(!body.name.isEmpty() && body.subFileName.isEmpty())
            {
                sql = QString("DELETE FROM %1 WHERE product_name=? AND name=?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME);
                query.prepare(sql);
                query.addBindValue(body.productName);
                query.addBindValue(body.name);
            }

        }
            break;
        }
        if(query.exec())
            res = true;
        else
        {
            m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
            //db.close();
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::updateItem(QSqlDatabase db, GS_BODY_TYPE nType, const GS_DB_BODY_C &dbBody)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__;
        return res;
    }
    QString sql;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);
        switch (nType) {
        case TB_USERS_INFO_E:
        {
            USER_INFO_S body =  boost::any_cast<USER_INFO_S>(dbBody);
            sql = QString("UPDATE %1 SET auth=?, date=?, products=?, password=?, log_path=?, auth_bits=?, message=? WHERE id=?;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.auth);
            query.addBindValue(body.date);
            query.addBindValue(body.password);
            query.addBindValue(strListToStrs(body.products));
            query.addBindValue(body.log_path);
            query.addBindValue(body.auth_bits);
            query.addBindValue(body.message);
            query.addBindValue(body.id);
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            BODY_PRODUCT_INFO_S body =  boost::any_cast<BODY_PRODUCT_INFO_S>(dbBody);
            sql = QString("UPDATE %1 SET desp=?, modify_date=?, save_path=? WHERE name=?;").arg(S_PRODUCTS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.desp);
            query.addBindValue(body.modifyDate);
            query.addBindValue(body.savePath);
            query.addBindValue(body.name);
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            BODY_CRC_VERSION_INFO_S body =  boost::any_cast<BODY_CRC_VERSION_INFO_S>(dbBody);
            sql = QString("UPDATE %1 SET is_default_version=?, message=?, file_name=?, file_size=?, custom=? WHERE product_name=? AND name=?;").arg(S_CRC_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.isDefaultVersion);
            query.addBindValue(body.message);
            query.addBindValue(body.fileName);
            query.addBindValue(body.fileSize);
            query.addBindValue(body.custom);
            query.addBindValue(body.productName);
            query.addBindValue(body.name);
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            BODY_MCU_VERSION_INFO_S body =  boost::any_cast<BODY_MCU_VERSION_INFO_S>(dbBody);
            sql = QString("UPDATE %1 SET mcu_type_name=?, is_default_version=?, message=?, file_name=?, file_size=? WHERE product_name=? AND name=?;").arg(S_MCU_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.mcuTypeName);
            query.addBindValue(body.isDefaultVersion);
            query.addBindValue(body.message);
            query.addBindValue(body.fileName);
            query.addBindValue(body.fileSize);
            query.addBindValue(body.productName);
            query.addBindValue(body.name);
        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            BODY_GROUP_VERSION_INFO_S body =  boost::any_cast<BODY_GROUP_VERSION_INFO_S>(dbBody);
            sql = QString("UPDATE %1 SET is_default_version=?, message=?, save_path=?, sub_file_name=?, sub_file_date=?, sub_file_path=?, sub_file_size=?, sub_file_message=? WHERE product_name=? AND name=? AND sub_file_name=?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.isDefaultVersion);
            query.addBindValue(body.message);
            query.addBindValue(body.savePath);
            query.addBindValue(body.subFileName);
            query.addBindValue(body.subFileDate);
            query.addBindValue(body.subFilePath);
            query.addBindValue(body.subFileSize);
            query.addBindValue(body.subFileMessage);
            query.addBindValue(body.productName);
            query.addBindValue(body.name);
            query.addBindValue(body.subFileName);
        }
            break;
        }
        if (query.exec())
            res = true;
        else
             m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::queryItem(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBody)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__;
        return res;
    }
    QString sql;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        switch (nType) {
        case TB_USERS_INFO_E:
        {
            USER_INFO_S body =  boost::any_cast<USER_INFO_S>(dbBody);
            sql = QString("SELECT * FROM %1 WHERE name = ?;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
            if (true==query.exec())
            {
                if (query.next())
                {
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    dbBody = tBody;
                    res = true;
                }
                else
                    m_errInfo = QString("Query user info failed,%1").arg(body.name);
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            BODY_PRODUCT_INFO_S body =  boost::any_cast<BODY_PRODUCT_INFO_S>(dbBody);
            sql = QString("SELECT * FROM %1 WHERE name = ?;").arg(S_PRODUCTS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
            if (true==query.exec())
            {
                if (query.next())
                {
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    dbBody = tBody;
                    res = true;
                }
                else
                    m_errInfo = QString("Query products info failed,%1").arg(body.name);
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            BODY_CRC_VERSION_INFO_S body =  boost::any_cast<BODY_CRC_VERSION_INFO_S>(dbBody);
            sql = QString("SELECT * FROM %1 WHERE name = ? AND product_name = ?;").arg(S_CRC_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
            query.addBindValue(body.productName);
            if (true==query.exec())
            {
                if (query.next())
                {
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    dbBody = tBody;
                    res = true;
                }
                else
                    m_errInfo = QString("Query user info failed,%1").arg(body.name);
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            BODY_MCU_VERSION_INFO_S body =  boost::any_cast<BODY_MCU_VERSION_INFO_S>(dbBody);
            sql = QString("SELECT * FROM %1 WHERE name = ? AND product_name = ? AND mcu_type_name=?;").arg(S_MCU_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
            query.addBindValue(body.productName);
            query.addBindValue(body.mcuTypeName);
            if (true==query.exec())
            {
                if (query.next())
                {
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    dbBody = tBody;
                    res = true;
                }
                else
                    m_errInfo = QString("Query user info failed,%1").arg(body.name);
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            BODY_GROUP_VERSION_INFO_S body =  boost::any_cast<BODY_GROUP_VERSION_INFO_S>(dbBody);
            sql = QString("SELECT * FROM %1 WHERE name = ? AND product_name = ?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(body.name);
            query.addBindValue(body.productName);
            if (true==query.exec())
            {
                if (query.next())
                {
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    dbBody = tBody;
                    res = true;
                }
                else
                    m_errInfo = QString("Query user info failed,%1").arg(body.name);
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(res)
        return res;
__SQL_QUERY_FAILED:
    m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::queryItems(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBodys, const QString &extTableName, int startId, int nItemCntMax)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__;
        return res;
    }
    QString sql;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        switch (nType) {
        case TB_USERS_INFO_E:
        {
            sql = QString("SELECT * FROM %1 LIMIT ?,?;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<USER_INFO_S> items = boost::any_cast<QList<USER_INFO_S>>(dbBodys);
                items.clear();

                while (query.next()) {
                    USER_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<USER_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query all user info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            sql = QString("SELECT * FROM %1 LIMIT ?,?;").arg(S_PRODUCTS_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_PRODUCT_INFO_S> items = boost::any_cast<QList<BODY_PRODUCT_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    //此处字段顺序必须与数据库中定义顺序对应
                    BODY_PRODUCT_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_PRODUCT_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query all products info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            sql = QString("SELECT * FROM %1 LIMIT ?,?;").arg(S_CRC_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_CRC_VERSION_INFO_S> items = boost::any_cast<QList<BODY_CRC_VERSION_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_CRC_VERSION_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_CRC_VERSION_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query all crc version info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            sql = QString("SELECT * FROM %1 LIMIT ?,?;").arg(S_MCU_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_MCU_VERSION_INFO_S> items = boost::any_cast<QList<BODY_MCU_VERSION_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_MCU_VERSION_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_MCU_VERSION_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query all mcu version info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            sql = QString("SELECT * FROM %1 LIMIT ?,?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_GROUP_VERSION_INFO_S> items = boost::any_cast<QList<BODY_GROUP_VERSION_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_GROUP_VERSION_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_GROUP_VERSION_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query all group version info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(res)
        return res;
__SQL_QUERY_FAILED:
    m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

bool DatabaseTool::queryItems(QSqlDatabase db, GS_BODY_TYPE nType, GS_DB_BODY_C &dbBodys, const QString &queryCondition, const QString &extTableName, int startId, int nItemCntMax)
{
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__ << __LINE__;
        return res;
    }

    if(queryCondition.isEmpty())
    {
        qWarning() << "The query condition cannot be empty!" << __FUNCTION__ << __LINE__;
        return res;
    }
    QString sql;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        switch (nType) {
        case TB_USERS_INFO_E:
        {
            sql = QString("SELECT * FROM %1 WHERE %2 LIMIT ?,?;").arg(S_USERS_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<USER_INFO_S> items = boost::any_cast<QList<USER_INFO_S>>(dbBodys);
                items.clear();

                while (query.next()) {
                    USER_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<USER_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Specify conditions query user info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            sql = QString("SELECT * FROM %1 WHERE %2 LIMIT ?,?;").arg(S_PRODUCTS_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);

            if (true==query.exec())
            {
                QList<BODY_PRODUCT_INFO_S> items = boost::any_cast<QList<BODY_PRODUCT_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_PRODUCT_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_PRODUCT_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Specify conditions query products info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            sql = QString("SELECT * FROM %1 WHERE %2 LIMIT ?,?;").arg(S_CRC_VERSION_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);

            if (true==query.exec())
            {
                QList<BODY_CRC_VERSION_INFO_S> items = boost::any_cast<QList<BODY_CRC_VERSION_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_CRC_VERSION_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_CRC_VERSION_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Specify conditions query crc version info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            sql = QString("SELECT * FROM %1 WHERE %2 LIMIT ?,?;").arg(S_MCU_VERSION_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_MCU_VERSION_INFO_S> items = boost::any_cast<QList<BODY_MCU_VERSION_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_MCU_VERSION_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_MCU_VERSION_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Specify conditions query mcu version info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            sql = QString("SELECT * FROM %1 WHERE %2 LIMIT ?,?;").arg(S_GROUP_VERSION_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);

            if (true==query.exec())
            {
                QList<BODY_GROUP_VERSION_INFO_S> items = boost::any_cast<QList<BODY_GROUP_VERSION_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_GROUP_VERSION_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_GROUP_VERSION_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Specify conditions query group version info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_LOG_INFO_E:
        {
            sql = QString("SELECT * FROM %1_%2 WHERE %3 LIMIT ?,?;").arg(extTableName).arg(S_LOG_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_LOG_INFO_S> items = boost::any_cast<QList<BODY_LOG_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_LOG_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_LOG_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query log info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        case TB_TMN_INFO_LOG_INFO_E:
        {
            sql = QString("SELECT * FROM %1_%2 WHERE %3 LIMIT ?,?;").arg(extTableName).arg(S_TMN_INFO_LOG_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
            query.addBindValue(startId);
            query.addBindValue(nItemCntMax);
            if (true==query.exec())
            {
                QList<BODY_TMN_INFO_S> items = boost::any_cast<QList<BODY_TMN_INFO_S>>(dbBodys);
                items.clear();
                while (query.next()) {
                    BODY_TMN_INFO_S body;
                    GS_DB_BODY_C tBody;
                    putDb2Struct(nType, tBody, query);
                    body = boost::any_cast<BODY_TMN_INFO_S>(tBody);
                    items << body;
                    res = true;
                }
                if(!res)
                    m_errInfo = QString("Query log info failed");
                dbBodys = items;
            }
            else
                goto __SQL_QUERY_FAILED;
        }
            break;
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(res)
        return res;
__SQL_QUERY_FAILED:
    m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

int DatabaseTool::queryMatchItemsCount(QSqlDatabase db, GS_BODY_TYPE nType, const QString &queryCondition, const QString &extTableName)
{
    int count = 0;
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__ << __LINE__;
        return res;
    }

    if(queryCondition.isEmpty())
    {
        qWarning() << "The query condition cannot be empty!" << __FUNCTION__ << __LINE__;
        return res;
    }
    QString sql;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        switch (nType) {
        case TB_USERS_INFO_E:
        {
            sql = QString("SELECT count(*) FROM %1 WHERE %2;").arg(S_USERS_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            sql =  QString("SELECT count(*) FROM %1 WHERE %2;").arg(S_PRODUCTS_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            sql =  QString("SELECT count(*) FROM %1 WHERE %2;").arg(S_CRC_VERSION_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            sql =  QString("SELECT count(*) FROM %1 WHERE %2;").arg(S_MCU_VERSION_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            sql =  QString("SELECT count(*) FROM %1 WHERE %2;").arg(S_GROUP_VERSION_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
        }
            break;
        case TB_LOG_INFO_E:
        {
            sql = QString("SELECT count(*) FROM %1_%2 WHERE %3;").arg(extTableName).arg(S_LOG_INFO_TABLE_NAME)
                    .arg(queryCondition);
            query.prepare(sql);
        }
            break;
        }
        if(true==query.exec())
        {
            if(query.next())
            {
                count = query.value(0).toInt();
                res = true;
            }
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(res)
        return count;
    m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

int DatabaseTool::queryItemsCount(QSqlDatabase db, GS_BODY_TYPE nType)
{
    int count = 0;
    bool res = false;
    if(nType >= TB_TYPE_MAX || nType < 0)
    {
        qDebug() << "Unsupported structure type" << nType << __FUNCTION__ << __LINE__;
        return res;
    }

    QString sql;
    if (db.isValid()==true && db.isOpen()==true )
    {
        QSqlQuery query(db);

        switch (nType) {
        case TB_USERS_INFO_E:
        {
            sql = QString("SELECT count(1) AS count FROM %1;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
        }
            break;
        case TB_PRODUCTS_INFO_E:
        {
            sql = QString("SELECT count(1) AS count FROM %1;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
        }
            break;
        case TB_CRC_VERSION_INFO_E:
        {
            sql = QString("SELECT count(1) AS count FROM %1;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
        }
            break;
        case TB_MCU_VERSION_INFO_E:
        {
            sql = QString("SELECT count(1) AS count FROM %1;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
        }
            break;
        case TB_GROUP_VERSION_INFO_E:
        {
            sql = QString("SELECT count(1) AS count FROM %1;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
        }
            break;
        case TB_LOG_INFO_E:
        {
            sql = QString("SELECT count(1) AS count FROM %1;").arg(S_USERS_INFO_TABLE_NAME);
            query.prepare(sql);
        }
            break;
        }
        if(true==query.exec())
        {
            if(query.next())
            {
                count = query.value(0).toInt();
                res = true;
            }
        }
    }
    else
        m_errInfo = QString(S_ERR_STR_NOT_OPEN).arg(db.databaseName());
    if(res)
        return count;
    m_errInfo = QString("Failed to execute mysql command,%1").arg(sql);
    if(!res)
        qDebug() << m_errInfo;
    return res;
}

}



