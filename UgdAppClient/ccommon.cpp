#include "ccommon.h"
#include <QMutex>
#include <QListWidget>
#include <QListWidgetItem>
#include "cversioninfo.h"
#include "cproductitem.h"
#include "cuseritem.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "ctcpclient.h"
#include <QDateTime>

QSettings Settings("AppSettings",QSettings::IniFormat);
CCommon::CCommon()
{

}

CCommon *CCommon::Instance()
{
    static QMutex s_dataMutex;
    static CCommon *pInstance = nullptr;
    s_dataMutex.lock();
    if(pInstance == nullptr)
        pInstance = new CCommon;
    s_dataMutex.unlock();
    return pInstance;
}

void CCommon::RemoveItem(QListWidget *listWidget,int row)
{
    if(listWidget == nullptr)
        return;
    if(row+1 > listWidget->count())
        return;
    QListWidgetItem *item = listWidget->item(row);
    QWidget *pWidget = listWidget->itemWidget(item);
    listWidget->removeItemWidget(item);
    listWidget->takeItem(row);
    delete item;
    pWidget->deleteLater();
}

void CCommon::RemoveAllItems(QListWidget *listWidget)
{
    if(listWidget == nullptr)
        return;
    int count = listWidget->count();
    while (count--) {
        RemoveItem(listWidget,count);
    }
}

QStringList CCommon::GetProductList()
{
    QStringList list;
    QJsonDocument ackJson;
    if(m_tcpClient->proxyCall(BdRequestQueryProductList_E,nullptr,ackJson))
    {
        QJsonObject ackObj = ackJson.object();
        if(ackObj.contains("data"))
        {
            QJsonArray productList = ackObj["data"].toArray();
            QString sName;
            foreach (QJsonValue value, productList) {
                list << value.toString();
            }
        }
    }
    return list;
}

bool CCommon::GetProductInfo(const QString &name, PRODUCT_INFO_S &info)
{
    PRODUCT_INFO_S productInfo;
    QJsonDocument ackJson;
    if(m_tcpClient->proxyCall(BdRequestQueryProductInfo_E,name.toUtf8().data(),ackJson))
    {
        QJsonObject ackObj = ackJson.object();
        if(!ackObj.contains("data"))
            return false;
        if(ackObj["data"].isNull())
            return false;
        QJsonObject productObj = ackObj["data"].toObject();
        if(productObj.contains("ProductName"))
            productInfo.productName = productObj["ProductName"].toString();
        if(productObj.contains("ProductTypeId"))
            productInfo.productTypeId = productObj["ProductTypeId"].toString().toULongLong();
        if(productObj.contains("Date"))
            productInfo.date = productObj["Date"].toString();
        if(productObj.contains("ProductDesp"))
            productInfo.productDesp = productObj["ProductDesp"].toString();
        if(productObj.contains("CrcDefaultVersion"))
        {
            QJsonObject crcObj = productObj["CrcDefaultVersion"].toObject();
            GetVersionInfo(productInfo.crcDefaultVersion,crcObj);
        }
        if(productObj.contains("SaveFilePath"))
            productInfo.saveFilePath = productObj["SaveFilePath"].toString();

        if(productObj.contains("CrcVersionList"))
        {
            QJsonArray jsonArray = productObj["CrcVersionList"].toArray();
            foreach(QJsonValue jsonValue,jsonArray){
                QJsonObject crcObj = jsonValue.toObject();
                VERSION_INFO_S crcInfo;
                GetVersionInfo(crcInfo,crcObj);
                productInfo.crcVersionList[crcInfo.name] = crcInfo;
            }

        }
        if(productObj.contains("McuInfos"))
        {
            QJsonArray jsonArray = productObj["McuInfos"].toArray();

            foreach(QJsonValue jsonValue,jsonArray){
                QJsonObject mcuTypeObj = jsonValue.toObject();
                MCU_INFO_S mcuTypeInfo;
                if(mcuTypeObj.contains("McuTypeName"))
                    mcuTypeInfo.mcuTypeName = mcuTypeObj["McuTypeName"].toString();
                if(mcuTypeObj.contains("McuTypeId"))
                    mcuTypeInfo.mcuTypeId = mcuTypeObj["McuTypeId"].toString().toULongLong();
                if(mcuTypeObj.contains("McuDefaultVersion"))
                {
                    QJsonObject mcuObj = mcuTypeObj["McuDefaultVersion"].toObject();
                    GetVersionInfo(mcuTypeInfo.mcuDefaultVersion,mcuObj);
                }
                if(mcuTypeObj.contains("McuVersionList"))
                {
                    QJsonArray jsonArray = mcuTypeObj["McuVersionList"].toArray();
                    foreach(QJsonValue jsonValue,jsonArray){
                        QJsonObject mcuObj = jsonValue.toObject();
                        VERSION_INFO_S mcuInfo;
                        GetVersionInfo(mcuInfo,mcuObj);
                        mcuTypeInfo.mcuVersionList[mcuInfo.name] = mcuInfo;
                    }
                }
                productInfo.mcuInfos[mcuTypeInfo.mcuTypeName] = mcuTypeInfo;
            }
        }

        if(productObj.contains("GroupFileDefaultVersion"))
        {
            QJsonObject groupFileObj = productObj["GroupFileDefaultVersion"].toObject();
            if(groupFileObj.contains("Name"))
                productInfo.groupFileDefaultVersion.name = groupFileObj["Name"].toString();
            if(groupFileObj.contains("Date"))
                productInfo.groupFileDefaultVersion.date = groupFileObj["Date"].toString();
            if(groupFileObj.contains("SavePath"))
                productInfo.groupFileDefaultVersion.savePath = groupFileObj["SavePath"].toString();
            if(groupFileObj.contains("Message"))
                productInfo.groupFileDefaultVersion.name = groupFileObj["Message"].toString();
            if(groupFileObj.contains("IsDefault"))
                productInfo.groupFileDefaultVersion.isDefault = groupFileObj["IsDefault"].toBool();
            productInfo.groupFileDefaultVersion.fileInfos.clear();
            if(groupFileObj.contains("FileInfos"))
            {
                QJsonArray fileArray = groupFileObj["FileInfos"].toArray();
                foreach (QJsonValue fileValue, fileArray) {
                    QJsonObject fileObj = fileValue.toObject();
                    VERSION_INFO_S fileInfo;
                    GetVersionInfo(fileInfo,fileObj);
                    productInfo.groupFileDefaultVersion.fileInfos[fileInfo.fileName] = fileInfo;
                }
            }
        }

        if(productObj.contains("GroupFileInfos"))
        {
            QJsonArray groupFileArray = productObj["GroupFileInfos"].toArray();
            foreach(QJsonValue groupFileValue,groupFileArray){
                QJsonObject groupFileObj = groupFileValue.toObject();
                GROUP_FILE_INFO_S groupFileInfo;
                if(groupFileObj.contains("Name"))
                    groupFileInfo.name = groupFileObj["Name"].toString();
                if(groupFileObj.contains("Date"))
                    groupFileInfo.date = groupFileObj["Date"].toString();
                if(groupFileObj.contains("SavePath"))
                    groupFileInfo.savePath = groupFileObj["SavePath"].toString();
                if(groupFileObj.contains("Message"))
                    groupFileInfo.message = groupFileObj["Message"].toString();
                if(groupFileObj.contains("IsDefault"))
                    groupFileInfo.isDefault = groupFileObj["IsDefault"].toBool();
                groupFileInfo.fileInfos.clear();
                if(groupFileObj.contains("FileInfos"))
                {
                    QJsonArray fileArray = groupFileObj["FileInfos"].toArray();
                    foreach (QJsonValue fileValue, fileArray) {
                        QJsonObject fileObj = fileValue.toObject();
                        VERSION_INFO_S fileInfo;
                        GetVersionInfo(fileInfo,fileObj);
                        groupFileInfo.fileInfos[fileInfo.fileName] = fileInfo;
                    }
                }
                productInfo.groupFileInfos[groupFileInfo.name] = groupFileInfo;
            }
        }
    }
    qDebug() << __FUNCTION__ << productInfo.crcVersionList.keys();
    info = productInfo;
    return true;
}

QMap<QString,PRODUCT_INFO_S> CCommon::LoadProducts()
{
    QMap<QString,PRODUCT_INFO_S> products;
    QJsonDocument ackJson;
    if(m_tcpClient->proxyCall(BdRequestQueryProductList_E,nullptr,ackJson))
    {
        QJsonObject ackObj = ackJson.object();
        if(ackObj.contains("data"))
        {
            QJsonArray productList = ackObj["data"].toArray();
            QString sName;
            foreach (QJsonValue jsonValue, productList) {
                sName = jsonValue.toString();
                if(m_tcpClient->proxyCall(BdRequestQueryProductInfo_E,sName.toUtf8().data(),ackJson))
                {
                    ackObj = ackJson.object();
                    if(!ackObj.contains("data"))
                        continue;
                    if(ackObj["data"].isNull())
                        continue;

                    QJsonObject productObj = ackObj["data"].toObject();
                    PRODUCT_INFO_S productInfo;
                    if(productObj.contains("ProductName"))
                        productInfo.productName = productObj["ProductName"].toString();
                    if(productObj.contains("ProductTypeId"))
                        productInfo.productTypeId = productObj["ProductTypeId"].toString().toULongLong();
                    if(productObj.contains("Date"))
                        productInfo.date = productObj["Date"].toString();
                    if(productObj.contains("ProductDesp"))
                        productInfo.productDesp = productObj["ProductDesp"].toString();
                    if(productObj.contains("CrcDefaultVersion"))
                    {
                        QJsonObject crcObj = productObj["CrcDefaultVersion"].toObject();
                        GetVersionInfo(productInfo.crcDefaultVersion,crcObj);
                    }
                    if(productObj.contains("SaveFilePath"))
                        productInfo.saveFilePath = productObj["SaveFilePath"].toString();

                    if(productObj.contains("CrcVersionList"))
                    {
                        QJsonArray jsonArray = productObj["CrcVersionList"].toArray();
                        foreach(QJsonValue jsonValue,jsonArray){
                            QJsonObject crcObj = jsonValue.toObject();
                            VERSION_INFO_S crcInfo;
                            GetVersionInfo(crcInfo,crcObj);
                            productInfo.crcVersionList[crcInfo.name] = crcInfo;
                        }
                    }
                    if(productObj.contains("McuInfos"))
                    {
                        QJsonArray jsonArray = productObj["McuInfos"].toArray();

                        foreach(QJsonValue jsonValue,jsonArray){
                            QJsonObject mcuTypeObj = jsonValue.toObject();
                            MCU_INFO_S mcuTypeInfo;
                            if(mcuTypeObj.contains("McuTypeName"))
                                mcuTypeInfo.mcuTypeName = mcuTypeObj["McuTypeName"].toString();
                            if(mcuTypeObj.contains("McuTypeId"))
                                mcuTypeInfo.mcuTypeId = mcuTypeObj["McuTypeId"].toString().toULongLong();
                            if(mcuTypeObj.contains("McuDefaultVersion"))
                            {
                                QJsonObject mcuObj = mcuTypeObj["McuDefaultVersion"].toObject();
                                GetVersionInfo(mcuTypeInfo.mcuDefaultVersion,mcuObj);
                            }
                            if(mcuTypeObj.contains("McuVersionList"))
                            {
                                QJsonArray jsonArray = mcuTypeObj["McuVersionList"].toArray();
                                foreach(QJsonValue jsonValue,jsonArray){
                                    QJsonObject mcuObj = jsonValue.toObject();
                                    VERSION_INFO_S mcuInfo;
                                    GetVersionInfo(mcuInfo,mcuObj);
                                    mcuTypeInfo.mcuVersionList[mcuInfo.name] = mcuInfo;
                                }
                            }
                            productInfo.mcuInfos[mcuTypeInfo.mcuTypeName] = mcuTypeInfo;
                        }
                    }

                    if(productObj.contains("GroupFileDefaultVersion"))
                    {
                        QJsonObject groupFileObj = productObj["GroupFileDefaultVersion"].toObject();
                        if(groupFileObj.contains("Name"))
                            productInfo.groupFileDefaultVersion.name = groupFileObj["Name"].toString();
                        if(groupFileObj.contains("Date"))
                            productInfo.groupFileDefaultVersion.date = groupFileObj["Date"].toString();
                        if(groupFileObj.contains("SavePath"))
                            productInfo.groupFileDefaultVersion.savePath = groupFileObj["SavePath"].toString();
                        if(groupFileObj.contains("Message"))
                            productInfo.groupFileDefaultVersion.name = groupFileObj["Message"].toString();
                        if(groupFileObj.contains("IsDefault"))
                            productInfo.groupFileDefaultVersion.isDefault = groupFileObj["IsDefault"].toBool();
                        productInfo.groupFileDefaultVersion.fileInfos.clear();
                        if(groupFileObj.contains("FileInfos"))
                        {
                            QJsonArray fileArray = groupFileObj["FileInfos"].toArray();
                            foreach (QJsonValue fileValue, fileArray) {
                                QJsonObject fileObj = fileValue.toObject();
                                VERSION_INFO_S fileInfo;
                                GetVersionInfo(fileInfo,fileObj);
                                productInfo.groupFileDefaultVersion.fileInfos[fileInfo.fileName] = fileInfo;
                            }
                        }
                    }

                    if(productObj.contains("GroupFileInfos"))
                    {
                        QJsonArray groupFileArray = productObj["GroupFileInfos"].toArray();
                        foreach(QJsonValue groupFileValue,groupFileArray){
                            QJsonObject groupFileObj = groupFileValue.toObject();
                            GROUP_FILE_INFO_S groupFileInfo;
                            if(groupFileObj.contains("Name"))
                                groupFileInfo.name = groupFileObj["Name"].toString();
                            if(groupFileObj.contains("Date"))
                                groupFileInfo.date = groupFileObj["Date"].toString();
                            if(groupFileObj.contains("SavePath"))
                                groupFileInfo.savePath = groupFileObj["SavePath"].toString();
                            if(groupFileObj.contains("Message"))
                                groupFileInfo.message = groupFileObj["Message"].toString();
                            if(groupFileObj.contains("IsDefault"))
                                groupFileInfo.isDefault = groupFileObj["IsDefault"].toBool();
                            groupFileInfo.fileInfos.clear();
                            if(groupFileObj.contains("FileInfos"))
                            {
                                QJsonArray fileArray = groupFileObj["FileInfos"].toArray();
                                foreach (QJsonValue fileValue, fileArray) {
                                    QJsonObject fileObj = fileValue.toObject();
                                    VERSION_INFO_S fileInfo;
                                    GetVersionInfo(fileInfo,fileObj);
                                    groupFileInfo.fileInfos[fileInfo.fileName] = fileInfo;
                                }
                            }
                            productInfo.groupFileInfos[groupFileInfo.name] = groupFileInfo;
                        }
                    }
                    products[productInfo.productName] = productInfo;

                }
            }
        }
    }
    return products;
}

void CCommon::getCpuId(unsigned int CPUInfo[4], unsigned int InfoType)
{
#ifdef Q_OS_WIN
#if defined(__GNUC__)// GCC
    __cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER)// MSVC
    #if _MSC_VER >= 1400 //VC2005才支持__cpuid
        __cpuid((int*)(void*)CPUInfo, (int)(InfoType));
    #else //其他使用getcpuidex
        getCpuIdex(CPUInfo, InfoType, 0);
    #endif
#endif
#endif
}

void CCommon::getCpuIdex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER) // MSVC
    #if defined(_WIN64)	// 64位下不支持内联汇编. 1600: VS2010, 据说VC2008 SP1之后才支持__cpuidex.
        __cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
    #else
        if (NULL==CPUInfo)	return;
        _asm{
        // load. 读取参数到寄存器.
        mov edi, CPUInfo;
        mov eax, InfoType;
        mov ecx, ECXValue;
        // CPUID
        cpuid;
        // save. 将寄存器保存到CPUInfo
        mov	[edi], eax;
        mov	[edi+4], ebx;
        mov	[edi+8], ecx;
        mov	[edi+12], edx;
        }
    #endif
#endif
}

//获取机器cpuId
QString CCommon::getCpuId()
{
    QString cpu_id = "";
    unsigned int dwBuf[4];
    unsigned long long ret;
    getCpuId(dwBuf, 1);
    ret = dwBuf[3];
    ret = ret << 32;
    cpu_id = QString::number(dwBuf[3], 16).toUpper();
    cpu_id = cpu_id + QString::number(dwBuf[0], 16).toUpper();
    //return cpu_id;

    quint8  idBuf[8];
    idBuf[0] =  dwBuf[3] & 0xFF;
    idBuf[1] = (dwBuf[3]>>8) & 0xFF;
    idBuf[2] =  dwBuf[2] & 0xFF;
    idBuf[3] = (dwBuf[2]>>8) & 0xFF;
    idBuf[4] =  dwBuf[1] & 0xFF;
    idBuf[5] = (dwBuf[1]>>8) & 0xFF;
    idBuf[6] =  dwBuf[0] & 0xFF;
    idBuf[7] = (dwBuf[0]>>8) & 0xFF;
    quint64 IdNum = *((quint64*)idBuf);
//    qint64 msec = QDateTime::currentMSecsSinceEpoch();
//    IdNum += msec;
    QString str,buf;
    for(int i=0;i<8;i++)
    {
        str  = "00";
        str += QString::number(idBuf[i], 16).toUpper();
        str = str.right(2);
        buf += str;
    }
    buf = QString("%1").arg(IdNum);
    while(buf.length() < 14){
        buf += "0";
    }
    if(buf.length() > 12)
        buf.remove(0,buf.length() - 11);
    buf = "U" + buf;
    return buf;
}

QString CCommon::getCpuId(const QString id)
{
    QString snCode;
    QString idStr = id;
    idStr.replace("-","");
    for(int i=0; i<idStr.count(); ++i)
    {
        snCode += idStr.mid(i,1);
        if((i+1)%4 == 0 && i != idStr.count()-1)
            snCode += "-";
    }
    return snCode;
}

void CCommon::PutVersionInfo(const VERSION_INFO_S &info,QJsonObject &infoObj)
{
    infoObj["Name"] = info.name;
    infoObj["Date"] = info.date;
    infoObj["IsDefault"] = info.isDefault;
    infoObj["Message"] = info.message;
    infoObj["FileName"] = info.fileName;
    infoObj["FileSize"] = QString::number(info.fileSize);
    infoObj["Custom"] = info.custom;
}

void CCommon::GetVersionInfo(VERSION_INFO_S &info, const QJsonObject &infoObj)
{
    if(infoObj.contains("Name"))
        info.name = infoObj["Name"].toString();
    if(infoObj.contains("Date"))
        info.date = infoObj["Date"].toString();
    if(infoObj.contains("IsDefault"))
        info.isDefault = infoObj["IsDefault"].toBool();
    if(infoObj.contains("Message"))
        info.message = infoObj["Message"].toString();
    if(infoObj.contains("FileName"))
        info.fileName = infoObj["FileName"].toString();
    if(infoObj.contains("FileSize"))
        info.fileSize = infoObj["FileSize"].toString().toULong();
    if(infoObj.contains("Custom"))
        info.custom = infoObj["Custom"].toString();
}

QString CCommon::GetEventName(int type)
{
    QString name;
    switch (type) {
    case EventGetDeviceFiles_E:
        name = QObject::tr("Get Device File");
        break;
    case EventUpgradeDeviceCrc_E:
        name  = QObject::tr("Upgrade Device CRC");
        break;
    case EventUpgradeDeviceMcu_E:
        name  = QObject::tr("Upgrade Device MCU");
        break;
    case EventUpgradeDevicePara_E:
        name  = QObject::tr("Upgrade Device Group File");
        break;
    default:
        break;
    }
    return name;
}

QString CCommon::GetEventDeviceIdTypeName(int type)
{
    QString name;
    switch (type) {
    case EventRevDeviceNum_E:
        name = QObject::tr("Device Number");
        break;
    case EventRevVideoId_E:
        name = QObject::tr("Video Id");
        break;
    case EventRevPhoneNum_E:
        name = QObject::tr("Phone Number");
        break;
    case EventRevLicenseNum_E:
        name = QObject::tr("License Plate");
        break;
    }
    return name;
}

QString CCommon::GetEventStatusName(int status)
{
    QString name;
    switch (status) {
    case EventStatusWaitting_E:
         name = QObject::tr("Waitting");
        break;
    case EventStatusExecuting_E:
         name = QObject::tr("In Execution");
        break;
    case EventStatusSucceed_E:
         name = QObject::tr("Succeed ");
        break;
    case EventStatusFailed_E:
         name = QObject::tr("Failed");
        break;
    }
    return name;
}

QMap<quint64,EVENT_INFO_S> CCommon::QueryEvents()
{
    QMap<quint64,EVENT_INFO_S> events;
    QJsonDocument ackDoc;
    if(m_tcpClient->proxyCall(BdRequestQueryPcClientEvents_E,NULL,ackDoc))
    {
        QJsonObject ackObj = ackDoc.object();
        if(ackObj.contains("data"))
        {
            QJsonArray eventArray = ackObj["data"].toArray();
            foreach (QJsonValue eventVal, eventArray) {
                EVENT_INFO_S info;
                QJsonObject eventObj = eventVal.toObject();
                if(eventObj.contains("EventSn"))
                    info.eventSn = eventObj["EventSn"].toString().toULongLong();
                if(eventObj.contains("EventType"))
                    info.eventType = eventObj["EventType"].toInt();
                if(eventObj.contains("IdType"))
                    info.idType = eventObj["IdType"].toInt();
                if(eventObj.contains("Id"))
                    info.id = eventObj["Id"].toString();
                if(eventObj.contains("EventPara1"))
                    info.eventPara1 = eventObj["EventPara1"].toString();
                if(eventObj.contains("EventPara2"))
                    info.eventPara2 = eventObj["EventPara2"].toString();
                if(eventObj.contains("AckPara1"))
                    info.ackPara1 = eventObj["AckPara1"].toString();
                if(eventObj.contains("AckPara2"))
                    info.ackPara2 = eventObj["AckPara2"].toString();
                if(eventObj.contains("Status"))
                    info.status = eventObj["Status"].toInt();
                events[info.eventSn] = info;
            }
        }
    }
    return events;
}

bool CCommon::AddLog2Db(CTcpClient *pClient,
                        const QString &productName,
                        const QString &devNum,
                        const QString &imeiNumber,
                        const QString &fwVer,
                        int type,
                        int subtype,
                        int result,
                        const QString &msg)
{
    if(pClient == nullptr)
        return false;
    BD_REQUEST_LOG_S info;
    info.productName = productName;
    info.devNumber = devNum;
    info.imeiNumber = imeiNumber;
    info.fwVersion = fwVer;
    info.type = type;
    info.subtype = subtype;
    info.result = result;
    info.dateTime = QDateTime::currentDateTime();
    info.message = msg;
    QJsonDocument ackDoc;
    return pClient->proxyCall(BdRequestAddLog_E,&info,ackDoc);
}

