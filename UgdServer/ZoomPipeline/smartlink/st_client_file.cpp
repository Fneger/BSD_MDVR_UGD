#include "st_client_file.h"
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

namespace BdUgdServer {
st_client_file *st_client_file::S_pThis = nullptr;
st_client_file::st_client_file()
{
    m_bakTimer = new QTimer();
    m_bakTimer->setInterval(24*3600*1000);
    //m_bakTimer->start();
}

st_client_file::~st_client_file()
{
    delete m_bakTimer;
}


st_client_file *st_client_file::Instance()
{
    static QMutex s_dataMutex;
    s_dataMutex.lock();
    if(S_pThis == nullptr)
        S_pThis = new st_client_file;
    s_dataMutex.unlock();
    return S_pThis;
}

void st_client_file::slot_bakTimerTimeout()
{

}

bool st_client_file::openProductJson(const QString &fileName,QJsonDocument &doc)
{
    bool res = false;
    QString strJson;
    m_mProductMutex.lock();
    QFile file(fileName);

    if(file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        strJson = file.readAll();
        //qDebug() << "strJson" << strJson;
        res = true;
        if(file.size() > 10)
        {
            QJsonParseError err;
            doc = QJsonDocument::fromJson(strJson.toUtf8(),&err);
            if (QJsonParseError::NoError == err.error)
                res = true;
            else
                res = false;
        }
        file.close();
    }
    m_mProductMutex.unlock();
    return res;
}

bool st_client_file::saveProductJson(const QString &fileName,QJsonDocument &doc)
{
    m_mProductMutex.lock();
    QByteArray byte_array = doc.toJson(QJsonDocument::Indented);
    QString json_str(byte_array);
    //根据实际填写路径
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "file error!";
        m_mProductMutex.unlock();
        return false;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    in << json_str;
    in.setGenerateByteOrderMark(false);

    file.close();   // 关闭file
    m_mProductMutex.unlock();
    return true;
}

bool st_client_file::openAuthUsers(QJsonDocument &doc)
{
    bool res = false;
    QString strJson;
    m_mAuthList.lock();
    QFile file("authlist.json");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        strJson = file.readAll();
        file.close();
    }
    m_mAuthList.unlock();
    if(file.size() == 0)
        res = true;
    QJsonParseError err;
    doc = QJsonDocument::fromJson(strJson.toUtf8(),&err);
    if (QJsonParseError::NoError == err.error)
    {
        res = true;
    }
    return res;
}

bool st_client_file::saveAuthUsers(QJsonDocument &doc)
{
    bool res = false;
    QByteArray byte_array = doc.toJson(QJsonDocument::Indented);
    QString json_str(byte_array);
    //根据实际填写路径
    m_mAuthList.lock();
    QFile file("authlist.json");

    if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        in << json_str;
        in.setGenerateByteOrderMark(false);
        file.close();   // 关闭file
        res = true;
    }
    else
        qDebug() << "file error!";
    m_mAuthList.unlock();
    return res;
}

bool st_client_file::addAuthUser(const USER_INFO_S &userInfo)
{
    if(userInfo.name.length() <= 0 || userInfo.password.length() <= 0)
        return false;
    bool res = false;
    QJsonDocument userDoc;
    if(openAuthUsers(userDoc))
    {
        QJsonObject usersObj = userDoc.object();
        if(!usersObj.contains(userInfo.name))
        {
            QJsonObject userObj;
            userObj["Name"] = userInfo.name;
            userObj["Password"] = userInfo.password;
            userObj["Auth"] = userInfo.auth;
            userObj["Date"] = userInfo.date;
            userObj["Message"] = userInfo.message;
            QJsonArray productArray;
            foreach (QString str, userInfo.products) {
                productArray << str;
            }
            userObj["Products"] = productArray;
            usersObj[userInfo.name] = userObj;
            userDoc.setObject(usersObj);
            if(saveAuthUsers(userDoc))
                res = true;
        }

    }
    return res;
}

bool st_client_file::removeAuthUser(const USER_INFO_S &userInfo)
{
    bool res = false;
    QJsonDocument userDoc;
    if(openAuthUsers(userDoc))
    {
        QJsonObject usersObj = userDoc.object();
        if(usersObj.contains(userInfo.name))
        {
            usersObj.remove(userInfo.name);
            userDoc.setObject(usersObj);
            if(saveAuthUsers(userDoc))
                res = true;
        }
        else
            res = true;
    }
    return res;
}

bool st_client_file::editAuthUser(const USER_INFO_S &userInfo)
{
    bool res = false;
    if(removeAuthUser(userInfo))
    {
        if(addAuthUser(userInfo))
            res = true;
    }
    return res;
}

bool st_client_file::loginClient(const BD_TMN_LOGIN_INFO_S loginInfo, USER_INFO_S &userInfo)
{
    bool auth = false;
    QJsonDocument userDoc;
    QString srcName(loginInfo.user_name);
    if(openAuthUsers(userDoc))
    {
        QJsonObject usersObj = userDoc.object();
        if(usersObj.contains(srcName))
        {
            QJsonObject userObj = usersObj[srcName].toObject();
            if(userObj.contains("Name") && userObj.contains("Password") && userObj.contains("Auth") && userObj.contains("Date")
                    && userObj.contains("Message") && userObj.contains("Products"))
            {
                QString srcPasswd(loginInfo.password);
                QString dstPasswd = userObj["Password"].toString();
                QString dstName = userObj["Name"].toString();
                if((srcPasswd == dstPasswd) && (srcName == dstName) && dstPasswd.length() > 0 && dstName.length() > 0)
                {
                    auth = true;
                    userInfo.name = srcName;
                    userInfo.password = srcPasswd;
                    userInfo.auth = userObj["Auth"].toInt();
                    userInfo.date = userObj["Date"].toString();
                    userInfo.message = userObj["Message"].toString();
                    userInfo.products.clear();
                    QJsonArray productArray = userObj["Products"].toArray();
                    foreach (QJsonValue val, productArray) {
                        userInfo.products << val.toString();
                    }
                    qDebug() << QString("User:%1,Auth:%2.login successful!").arg(userInfo.name).arg(userInfo.auth);
                }
            }
        }
    }
    return auth;
}
}

