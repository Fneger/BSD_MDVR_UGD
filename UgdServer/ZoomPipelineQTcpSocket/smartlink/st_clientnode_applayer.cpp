#include "st_clientnode_applayer.h"
#include "st_client_table.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include "st_client_file.h"

namespace ExampleServer{
	st_clientNodeAppLayer::st_clientNodeAppLayer(st_client_table * pClientTable, QObject * pClientSock ,QObject *parent) :
		st_clientNode_baseTrans(pClientTable,pClientSock,parent)
      , m_uploadFileHandle(nullptr)
      , m_receivedBytes(0)
      , m_downloadFileHandle(nullptr)
      , m_sentBytes(0)
      , m_totalReceivedTestBytes(0)
	{
        m_userInfo.auth = -1;
        m_bLoggedIn= false;
        USER_INFO_S userInfo;
        userInfo.name = "admin";
        userInfo.password = "456123";
        userInfo.auth = BdUserAuthAdmin_E;
        st_client_file::Instance()->addAuthUser(userInfo);
    }

	//!deal current message
    int st_clientNodeAppLayer::deal_current_message_block()
    {
        if(st_clientNode_baseTrans::deal_current_message_block() < 0)
            return -1;
        uint8_t *msgdata = (uint8_t*)&m_currentBlock.data()[14];
        uint16_t msgpos = 0;
        QStringList msg;
        switch (uplinkFid()) {
        case BdTerminalRptsHeartbeat_E:
            msg << "BdUgdServer Receive TerminalRptsHeartbeat_E\n";
            sendPacket(BdPlatformResponse_E,NULL,0);
            break;
        case BdTerminalLogin_E:
        {
            msg << "BdUgdServer Receive BdTerminalLogin_E\n";

            BD_TMN_LOGIN_INFO_S loginInfo;
            memcpy(loginInfo.tsft_version,msgdata,sizeof (loginInfo.tsft_version));
            msgpos += sizeof (loginInfo.tsft_version);
            memcpy(loginInfo.stf_date,&msgdata[msgpos],sizeof (loginInfo.stf_date));
            msgpos += sizeof (loginInfo.stf_date);
            memcpy(loginInfo.cpu_id,&msgdata[msgpos],sizeof (loginInfo.cpu_id));
            msgpos += sizeof (loginInfo.cpu_id);
            loginInfo.terminal_type = msgdata[msgpos++];
            memcpy(loginInfo.user_name,&msgdata[msgpos],sizeof (loginInfo.user_name));
            msgpos += sizeof (loginInfo.user_name);
            memcpy(loginInfo.password,&msgdata[msgpos],sizeof (loginInfo.password));
            msgpos += sizeof (loginInfo.password);

            BD_TMN_LOGIN_RESPONSE_S loginResponse;
            getSysTime(loginResponse.platform_time,6);
            loginResponse.models_id = 0xaaaa;
            loginResponse.displacement = 0;
            loginResponse.needUpgrade = 0;
            if(st_client_file::Instance()->loginClient(loginInfo,m_userInfo))
            {
                loginResponse.auth = m_userInfo.auth+1;
            }
            else
                loginResponse.auth = 0;
            sendPacket(BdTerminalLoginResponse_E,&loginResponse,0);
        }
            break;
        case BdTerminalUploadFileData_E:
        {
            msg << "BdUgdServer Receive BdTerminalUploadFileData_E\n";
            BD_TRANSFER_FILE_DATA_S body;
            body.currently_sent_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.sent_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.not_sent_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.file_size = get4bytes(&msgdata[msgpos],&msgpos);
            uint32_t datalen = sizeof (body.file_data);
            memcpy(body.file_data,&msgdata[msgpos],datalen);
            msgpos += datalen;
            if(receiveFileData(body))
                sendPacket(BdPlatformResponse_E,NULL,0);
            else
                sendPacket(BdPlatformResponse_E,NULL,BdDataTransmissionError_E);
        }
            break;
        case BdTerminalDownloadFileData_E:
        {
            BD_TMN_DOWNLOAD_FILE_DATA body;
            body.request_receive_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.received_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.not_received_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.file_size = get4bytes(&msgdata[msgpos],&msgpos);
            BD_TRANSFER_FILE_DATA_S tBody;
            memset(&tBody,0,sizeof(BD_TRANSFER_FILE_DATA_S));
            sendFileData(body,tBody);
            sendPacket(BdPlarformSendFileData_E,&tBody,0);
        }
            break;
        case BdTerminalStartClientTest_E:
        {
            m_totalReceivedTestBytes = 0;
            sendPacket(BdPlatformResponse_E,NULL,0);
        }
            break;
        case BdTerminalClientTest_E:
        {
            BD_TMN_CLIENT_TEST_S body;
            body.currently_sent_bytes = get4bytes(&msgdata[msgpos],&msgpos);
            body.total_sent_bytes = get8bytes(&msgdata[msgpos],&msgpos);
            memcpy(body.test_data,&msgdata[msgpos],body.currently_sent_bytes);
            msgpos += body.currently_sent_bytes;
            m_totalReceivedTestBytes += body.currently_sent_bytes;
            if(m_totalReceivedTestBytes == body.total_sent_bytes)
                sendPacket(BdPlatformResponse_E,NULL,0);
            else
                sendPacket(BdPlatformResponse_E,NULL,BdDataTransmissionError_E);
        }
            break;
        case BdTerminalJson_E:
        {
             msg << "BdUgdServer Receive BdTerminalJson_E\n";
//             QByteArray json;
//             json.append((char*)msgdata,strlen((char*)msgdata)+1);
//             //strcpy(json,(char*)msgdata);
             //msg << QString::fromUtf8((char*)msgdata) + "\n";
             QJsonDocument ackDoc;
             reponseRequest((char*)msgdata,ackDoc);
             //qDebug() << ackDoc;
             sendPacket(BdPlatformAckJson_E,ackDoc.toJson(QJsonDocument::Indented).data(),0);
        }
            break;
        case BdTerminalResponse_E:
            msg << "BdUgdServer Receive TerminalResponse_E\n";
            break;
        default:
            msg << "BdUgdServer Other Function ID\n";
            break;
        }
        qDebug() << msg;
        return  0;
    }

    bool st_clientNodeAppLayer::clientLogout()
    {
        return true;
    }

    bool st_clientNodeAppLayer::reponseRequest(const char *rJson, QJsonDocument &ackDoc)
    {
        if(rJson == nullptr)
            return false;
        bool res = false;
        QString strJson(rJson);
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8(),&err);
        ackDoc = QJsonDocument();
        QJsonObject ackJsonObj;
        if (QJsonParseError::NoError != err.error)
        {
            ackJsonObj["code"] = BdJsonParseError_E;
            ackJsonObj["msg"] ="Parse Json Error!";
        }
        else
        {
            QJsonObject jsonObj = doc.object();
            if(!jsonObj.contains("data") || !jsonObj.contains("RequestCode"))
            {
                ackJsonObj["code"] = BdJsonFormatError_E;
                ackJsonObj["msg"] ="Json format error!";
            }
            else
            {
                int requestCode = jsonObj["RequestCode"].toInt();
                switch (requestCode) {
                case BdRequestAddOtherFileVersion_E:
                case BdRequestSetDefaultMcuVersion_E:
                case BdRequestSaveProductInfo_E: //添加产品
                {
                    QJsonDocument productDoc;
                    if(st_client_file::Instance()->openProductJson(productDoc))
                    {
                        QJsonObject productObj = productDoc.object();
                        QJsonObject newProductObj = jsonObj["data"].toObject();
                        productObj[newProductObj["ProductName"].toString()] = newProductObj;
                        //判断是否有新版本加入到历史版本中
                        //......
                        productDoc.setObject(productObj);
                        if(st_client_file::Instance()->saveProductJson(productDoc))
                        {
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] = "Save product successfully!";
                        }
                        else
                        {
                            ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                            ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        }
                    }
                    else
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                    }

                }
                    break;
                case BdRequestDeleteProductInfo_E:
                {
                    QJsonDocument productDoc;
                    if(st_client_file::Instance()->openProductJson(productDoc))
                    {
                        QJsonObject productObj = productDoc.object();
                        QJsonObject delProductObj = jsonObj["data"].toObject();
                        if(productObj.contains(delProductObj["ProductName"].toString()))
                        {
                            productObj.remove(delProductObj["ProductName"].toString());
                            productDoc.setObject(productObj);
                            if(st_client_file::Instance()->saveProductJson(productDoc))
                            {
                                ackJsonObj["code"] = BdExecutionSucceeded_E;
                                ackJsonObj["msg"] = "Delete product successfully!";
                            }
                            else
                            {
                                ackJsonObj["code"] = BdDeleteProductInfoFailed_E;
                                ackJsonObj["msg"] ="Deleting failed, Failed to save product information file!";
                            }
                        }
                        else
                        {
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] = "Delete product successfully,product does not exist!";
                        }
                    }
                }
                    break;
                case BdRequestQueryProductList_E:
                {
                    if(m_userInfo.auth < 0)
                    {
                        ackJsonObj["code"] = BdUserNotLogged_E;
                        ackJsonObj["msg"] = "The user is not logged in!";
                        goto REPONSE_END;
                    }
                    if(m_userInfo.auth == BdUserAuthAdmin_E)
                    {
                        QJsonDocument productDoc;
                        if(st_client_file::Instance()->openProductJson(productDoc))
                        {
                            QJsonArray jsonArray;
                            QStringList keys = productDoc.object().keys();
                            foreach (QString key, keys) {
                                jsonArray << key;
                            }
                            ackJsonObj["data"] = jsonArray;
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] ="Query the product list successfully!";
                        }
                        else
                        {
                            ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                            ackJsonObj["msg"] ="Failed to open product information file!";
                        }
                    }
                    else
                    {
                        QJsonArray jsonArray;
                        QStringList keys = m_userInfo.products;
                        foreach (QString key, keys) {
                            jsonArray << key;
                        }
                        ackJsonObj["data"] = jsonArray;
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] ="Query the product list successfully!";
                    }
                }
                    break;
                case BdRequestQueryProductInfo_E:
                {
                    QJsonDocument productDoc;
                    if(st_client_file::Instance()->openProductJson(productDoc))
                    {
                        QJsonObject productObj = productDoc.object();
                        QString productName = jsonObj["data"].toObject()["ProductName"].toString();
                        if(productObj.contains(productName))
                        {
                            ackJsonObj["data"] = productObj[productName];
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] ="Query the product information successfully!";
                        }
                        else
                        {
                            ackJsonObj["code"] = BdProductDontExist_E;
                            ackJsonObj["msg"] ="The product does not exist!";
                        }
                    }
                    else
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                    }
                }
                    break;
                case BdRequestUploadFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("RemoteFilePath"))
                        m_uploadFileInfo.remoteFilePath = dataObj["RemoteFilePath"].toString();
                    if(dataObj.contains("RemoteFileName"))
                        m_uploadFileInfo.remoteFileName = dataObj["RemoteFileName"].toString();
                    if(dataObj.contains("FileSize"))
                        m_uploadFileInfo.fileSize = dataObj["FileSize"].toString().toULong();
                    if(m_uploadFileInfo.remoteFilePath.length() <= 0 || m_uploadFileInfo.fileSize == 0
                            || m_uploadFileInfo.remoteFileName.length() <= 0)
                    {
                        ackJsonObj["code"] = BdUploadFileRequiredInfoError_E;
                        ackJsonObj["msg"] ="The information required to upload the file is wrong!";
                    }
                    else
                    {
                        QDir dir(m_uploadFileInfo.remoteFilePath);
                        if(!dir.exists())
                        {
                            mkMutiDir(m_uploadFileInfo.remoteFilePath);
                        }
                        if(!openUploadFile(m_uploadFileInfo.remoteFilePath + m_uploadFileInfo.remoteFileName))
                        {
                            ackJsonObj["code"] = BdCreateFileFailed_E;
                            ackJsonObj["msg"] ="Failed to create file!";
                        }
                        else
                        {
                            m_receivedBytes = 0;
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] ="Ready to receive file data!";
                        }
                    }
                }
                    break;
                case BdRequestStopUploadFile_E:
                {
                    closeUploadFile();
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] ="Stop uploading files successfully!";
                }
                    break;
                case BdRequestDownloadFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("RemoteFileFullPath"))
                    {
                        if(openDownloadFile(dataObj["RemoteFileFullPath"].toString()))
                        {
                            m_sentBytes = 0;
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] ="Ready to send file data!";
                            QJsonObject ackObj;
                            ackObj["FileSize"] = m_downloadFileHandle->size();
                            ackJsonObj["data"] = ackObj;

                        }
                        else
                        {
                            ackJsonObj["code"] = BdOpenFileFialed_E;
                            ackJsonObj["msg"] ="Failed to open file!";
                        }
                    }
                    else
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Download file,parse Json Error!";
                    }
                }
                    break;
                case BdRequestStopDownloadFile_E:
                {
                    closeDownloadFile();
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] ="Stop downloading files successfully!";
                }
                    break;
                case BdRequestAddNewCrcVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("VersionInfo"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Add new CRC version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto REPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    QString productName = dataObj["ProductName"].toString();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto REPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    QJsonArray crcArray;
                    QJsonObject crcObj = dataObj["VersionInfo"].toObject();
                    if(currObj.contains("CrcVersionList"))
                        crcArray = currObj["CrcVersionList"].toArray();
                    crcArray.push_front(crcObj);
                    currObj["CrcVersionList"] = crcArray;
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    if(!st_client_file::Instance()->saveProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        goto REPONSE_END;
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add new CRC version successfully!";
                    }
                }
                    break;
                case BdRequestRemoveCrcVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("VersionInfo"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Remove CRC version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto REPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    QString productName = dataObj["ProductName"].toString();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto REPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    QJsonArray crcArray;
                    if(currObj.contains("CrcVersionList"))
                        crcArray = currObj["CrcVersionList"].toArray();
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove new CRC version successfully!";
                        goto REPONSE_END;
                    }
                    QJsonObject crcObj = dataObj["VersionInfo"].toObject();
                    if(!crcObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdRemoveCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Remove CRC version Json parsing error!";
                        goto REPONSE_END;
                    }
                    int index = -1;
                    QString deleteName = crcObj["Name"].toString();
                    foreach (QJsonValue crcValue, crcArray) {
                        index++;
                        QJsonObject crcObj = crcValue.toObject();
                        if(!crcObj.contains("Name"))
                            continue;
                        if(crcObj["Name"].toString() == deleteName)
                        {
                            if(crcObj.contains("IsDefault"))
                            {
                                if(crcObj["IsDefault"].toBool())
                                    currObj["CrcDafaultVersion"] = "";
                            }
                            crcArray.removeAt(index);
                            break;
                        }
                    }
                    if(crcObj.contains("FileName") && currObj.contains("SaveFilePath"))
                    {
                        QFile file(currObj["SaveFilePath"].toString() + crcObj["FileName"].toString());
                        if(file.exists())
                            file.remove();
                    }
                    currObj["CrcVersionList"] = crcArray;
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    if(!st_client_file::Instance()->saveProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        goto REPONSE_END;
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove new CRC version successfully!";
                    }
                }
                    break;
                case BdRequestSetDefaultCrcVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("VersionInfo"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Remove CRC version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto REPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    QString productName = dataObj["ProductName"].toString();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto REPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    QJsonArray crcArray;
                    if(currObj.contains("CrcVersionList"))
                        crcArray = currObj["CrcVersionList"].toArray();
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove new CRC version successfully!";
                        goto REPONSE_END;
                    }
                    QJsonObject crcObj = dataObj["VersionInfo"].toObject();
                    if(!crcObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdRemoveCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Remove CRC version Json parsing error!";
                        goto REPONSE_END;
                    }

                    bool finded = false;
                    int index = -1;
                    QString newCrcDefaultVersion = crcObj["Name"].toString();
                    foreach (QJsonValue crcValue, crcArray) {
                        index++;
                        QJsonObject crcObj = crcValue.toObject();
                        if(!crcObj.contains("Name") || !crcObj.contains("IsDefault"))
                            continue;
                        if(crcObj["Name"].toString() == newCrcDefaultVersion)
                        {
                            crcObj["IsDefault"] = true;
                            crcArray.replace(index,crcObj);
                            finded = true;
                            break;
                        }
                    }
                    if(!finded)
                    {
                        ackJsonObj["code"] = BdCrcVersionDontExists_E;
                        ackJsonObj["msg"] = "The CRC version does not exist!";
                        goto REPONSE_END;
                    }
                    QString oldCrcDefaultVersion;
                    if(currObj.contains("CrcDafaultVersion"))
                        oldCrcDefaultVersion = currObj["CrcDafaultVersion"].toString();
                    if(oldCrcDefaultVersion.length() > 0) //移除旧默认版本设置
                    {
                        int index = -1;
                        QJsonObject crcObj;
                        foreach (QJsonValue crcValue, crcArray) {
                            index++;
                            crcObj = crcValue.toObject();
                            if(!crcObj.contains("Name") || !crcObj.contains("IsDefault"))
                                continue;
                            if(crcObj["Name"].toString() == oldCrcDefaultVersion)
                            {
                                crcObj["IsDefault"] = false;
                                crcArray.replace(index,crcObj);
                                break;
                            }
                        }
                    }

                    currObj["CrcDafaultVersion"] = newCrcDefaultVersion;
                    currObj["CrcVersionList"] = crcArray;
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    if(!st_client_file::Instance()->saveProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        goto REPONSE_END;
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Set default CRC version successfully!";
                    }
                }
                    break;
                case BdRequestAddMcuType_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("ProductName") && dataObj.contains("McuTypeInfo"))
                    {
                        QJsonObject mcuTypeObj = dataObj["McuTypeInfo"].toObject();
                        if(!mcuTypeObj.contains("McuTypeName") || !mcuTypeObj.contains("McuTypeId") || !mcuTypeObj.contains("McuDefaultVersion")
                                 || !mcuTypeObj.contains("McuVersionList"))
                        {
                            ackJsonObj["code"] = BdMcuTypeInfoParseError_E;
                            ackJsonObj["msg"] = "MCU type information parsing error!";
                            goto REPONSE_END;
                        }

                        QJsonDocument productDoc;
                        if(st_client_file::Instance()->openProductJson(productDoc))
                        {
                            QJsonObject productObj = productDoc.object();
                            QString productName = dataObj["ProductName"].toString();
                            if(productObj.contains(productName))
                            {
                                QJsonObject currObj = productObj[productName].toObject();
                                if(!currObj.contains("McuInfos"))
                                {
                                    ackJsonObj["code"] = BdProductInfoParseError_E;
                                    ackJsonObj["msg"] ="Product information parsing error!";
                                }
                                else
                                {
                                    QJsonArray mcuTypeArray = currObj["McuInfos"].toArray();
                                    QString curMcuTypeName = mcuTypeObj["McuTypeName"].toString();
                                    bool isExists = false;
                                    foreach (QJsonValue jsonValue, mcuTypeArray) {
                                        QJsonObject mcuTypeObj = jsonValue.toObject();
                                        if(mcuTypeObj.contains("McuTypeName"))
                                        {
                                            if(curMcuTypeName == mcuTypeObj["McuTypeName"].toString())
                                            {
                                                isExists = true;
                                                break;
                                            }
                                        }
                                    }
                                    if(isExists)
                                    {
                                        ackJsonObj["code"] = BdMcuTypeExists_E;
                                        ackJsonObj["msg"] ="MCU type already exists!";
                                    }
                                    else
                                    {
                                        mcuTypeArray << mcuTypeObj;
                                        currObj["McuInfos"] = mcuTypeArray;
                                        productObj[productName] = currObj;
                                        productDoc.setObject(productObj);
                                        if(st_client_file::Instance()->saveProductJson(productDoc))
                                        {
                                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                                            ackJsonObj["msg"] = "Succeeded in adding a new MCU type!";
                                        }
                                        else
                                        {
                                            ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                                            ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                                        }
                                    }
                                }
                            }
                            else
                            {
                                ackJsonObj["code"] = BdProductDontExist_E;
                                ackJsonObj["msg"] ="The product does not exist!";
                            }
                        }
                        else
                        {
                            ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                            ackJsonObj["msg"] ="Failed to open product information file!";
                        }
                    }
                    else
                    {
                        ackJsonObj["code"] = BdAddNewMcuTypeJsonParseError_E;
                        ackJsonObj["msg"] ="Add new type MCU Json parsing error!";
                    }
                }
                    break;
                case BdRequestAddNewMcuVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("VersionInfo") || !dataObj.contains("McuTypeName"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Add new MCU version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto REPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    QString productName = dataObj["ProductName"].toString();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto REPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    if(!currObj.contains("McuInfos"))
                    {
                        ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                        ackJsonObj["msg"] = "The MCU information parsing error in the product information!";
                        goto REPONSE_END;
                    }
                    QString newVersionMcuTypeName = dataObj["McuTypeName"].toString();
                    QJsonArray mcuTypeArray = currObj["McuInfos"].toArray();
                    bool finded = false;
                    int index = -1;
                    foreach (QJsonValue mcuTypeVal, mcuTypeArray) {
                        index++;
                        QJsonObject mcuTypeObj = mcuTypeVal.toObject();
                        if(!mcuTypeObj.contains("McuTypeName"))
                            continue;
                        if(mcuTypeObj["McuTypeName"].toString() == newVersionMcuTypeName)
                        {
                            if(mcuTypeObj.contains("McuVersionList"))
                            {
                                QJsonArray mcuArrar = mcuTypeObj["McuVersionList"].toArray();
                                QJsonObject mcuObj = dataObj["VersionInfo"].toObject();
                                mcuArrar.push_front(mcuObj);
                                mcuTypeObj["McuVersionList"] = mcuArrar;
                                mcuTypeArray.replace(index,mcuTypeObj);
                                currObj["McuInfos"] = mcuTypeArray;
                                finded = true;
                                break;
                            }
                        }
                    }
                    if(!finded)
                    {
                        ackJsonObj["code"] = BdMcuTypeDontExists_E;
                        ackJsonObj["msg"] ="The mcu type does not exist!";
                        goto REPONSE_END;
                    }
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    if(!st_client_file::Instance()->saveProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        goto REPONSE_END;
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add new MCU version successfully!";
                    }
                }
                    break;
                case BdRequestRemoveMcuVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("VersionInfo") || !dataObj.contains("McuTypeName"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Add new MCU version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto REPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    QString productName = dataObj["ProductName"].toString();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto REPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    if(!currObj.contains("McuInfos") && !currObj.contains("SaveFilePath"))
                    {
                        ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                        ackJsonObj["msg"] = "The MCU information parsing error in the product information!";
                        goto REPONSE_END;
                    }
                    QString saveFilePath = currObj["SaveFilePath"].toString();
                    QString removeVersionMcuTypeName = dataObj["McuTypeName"].toString();
                    QJsonObject removeMcuObj = dataObj["VersionInfo"].toObject();
                    if(!removeMcuObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Remove MCU version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonArray mcuTypeArray = currObj["McuInfos"].toArray();
                    bool finded = false;
                    int indexType = -1;
                    foreach (QJsonValue mcuTypeVal, mcuTypeArray) {
                        indexType++;
                        QJsonObject mcuTypeObj = mcuTypeVal.toObject();
                        if(!mcuTypeObj.contains("McuTypeName"))
                            continue;
                        if(mcuTypeObj["McuTypeName"].toString() == removeVersionMcuTypeName)
                        {
                            if(mcuTypeObj.contains("McuVersionList"))
                            {
                                QString removeMcuVersionName = removeMcuObj["Name"].toString();
                                QJsonArray mcuArrar = mcuTypeObj["McuVersionList"].toArray();
                                int index = -1;
                                foreach (QJsonValue mcuVal, mcuArrar) {
                                    index++;
                                    QJsonObject mcuObj = mcuVal.toObject();
                                    if(!mcuObj.contains("Name"))
                                        continue;
                                    if(mcuObj["Name"].toString() == removeMcuVersionName)
                                    {
                                        if(mcuObj.contains("FileName"))
                                        {
                                            QFile file(saveFilePath + mcuObj["FileName"].toString());
                                            if(file.exists())
                                                file.remove();
                                        }
                                        if(mcuObj.contains("IsDefault"))
                                        {
                                            if(mcuObj["IsDefault"].toBool())
                                                mcuTypeObj["McuDefaultVersion"] = "";
                                        }
                                        mcuArrar.removeAt(index);
                                        mcuTypeObj["McuVersionList"] = mcuArrar;
                                        mcuTypeArray.replace(indexType,mcuTypeObj);
                                        currObj["McuInfos"] = mcuTypeArray;
                                        break;
                                    }
                                }
                                finded = true;
                                break;
                            }
                        }
                    }
                    if(!finded)
                    {
                        ackJsonObj["code"] = BdMcuTypeDontExists_E;
                        ackJsonObj["msg"] ="The mcu type does not exist!";
                        goto REPONSE_END;
                    }
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    if(!st_client_file::Instance()->saveProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        goto REPONSE_END;
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add new MCU version successfully!";
                    }
                }
                    break;
                case BdRequestRemoveOtherFileVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("VersionInfo") || !dataObj.contains("FileTypeName"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Add new FILE version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto REPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    QString productName = dataObj["ProductName"].toString();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto REPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    if(!currObj.contains("FileInfos") && !currObj.contains("SaveFilePath"))
                    {
                        ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                        ackJsonObj["msg"] = "The FILE information parsing error in the product information!";
                        goto REPONSE_END;
                    }
                    QString saveFilePath = currObj["SaveFilePath"].toString();
                    QString removeVersionFileTypeName = dataObj["FileTypeName"].toString();
                    QJsonObject removeFileObj = dataObj["VersionInfo"].toObject();
                    if(!removeFileObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                        ackJsonObj["msg"] ="Remove FILE version Json parsing error!";
                        goto REPONSE_END;
                    }
                    QJsonArray fileTypeArray = currObj["FileInfos"].toArray();
                    bool finded = false;
                    int indexType = -1;
                    foreach (QJsonValue fileTypeVal, fileTypeArray) {
                        indexType++;
                        QJsonObject fileTypeObj = fileTypeVal.toObject();
                        if(!fileTypeObj.contains("FileTypeName"))
                            continue;
                        if(fileTypeObj["FileTypeName"].toString() == removeVersionFileTypeName)
                        {
                            if(fileTypeObj.contains("FileVersionList"))
                            {
                                QString removeFileVersionName = removeFileObj["Name"].toString();
                                QJsonArray fileArrar = fileTypeObj["FileVersionList"].toArray();
                                int index = -1;
                                foreach (QJsonValue fileVal, fileArrar) {
                                    index++;
                                    QJsonObject fileObj = fileVal.toObject();
                                    if(!fileObj.contains("Name"))
                                        continue;
                                    if(fileObj["Name"].toString() == removeFileVersionName)
                                    {
                                        if(fileObj.contains("FileName"))
                                        {
                                            QFile file(saveFilePath + fileObj["FileName"].toString());
                                            if(file.exists())
                                                file.remove();
                                        }
                                        if(fileObj.contains("IsDefault"))
                                        {
                                            if(fileObj["IsDefault"].toBool())
                                                fileTypeObj["FileDefaultVersion"] = "";
                                        }
                                        fileArrar.removeAt(index);
                                        fileTypeObj["FileVersionList"] = fileArrar;
                                        fileTypeArray.replace(indexType,fileTypeObj);
                                        currObj["FileInfos"] = fileTypeArray;
                                        break;
                                    }
                                }
                                finded = true;
                                break;
                            }
                        }
                    }
                    if(!finded)
                    {
                        ackJsonObj["code"] = BdFileTypeDontExists_E;
                        ackJsonObj["msg"] ="The FILE type does not exist!";
                        goto REPONSE_END;
                    }
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    if(!st_client_file::Instance()->saveProductJson(productDoc))
                    {
                        ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                        goto REPONSE_END;
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add new FILE version successfully!";
                    }
                }
                    break;
                case BdRequestRemoveMcuType_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("ProductName") && dataObj.contains("McuTypeName"))
                    {
                        QJsonDocument productDoc;
                        if(st_client_file::Instance()->openProductJson(productDoc))
                        {
                            QJsonObject productObj = productDoc.object();
                            QString productName = dataObj["ProductName"].toString();
                            if(productObj.contains(productName))
                            {
                                QJsonObject currObj = productObj[productName].toObject();
                                if(!currObj.contains("McuInfos"))
                                {
                                    ackJsonObj["code"] = BdProductInfoParseError_E;
                                    ackJsonObj["msg"] ="Product information parsing error!";
                                }
                                else
                                {
                                    QJsonArray mcuArray = currObj["McuInfos"].toArray();
                                    QString curMcuTypeName = dataObj["McuTypeName"].toString();
                                    bool isExists = false;
                                    int index = 0;
                                    foreach (QJsonValue jsonValue, mcuArray) {
                                        QJsonObject mcuObj = jsonValue.toObject();
                                        if(mcuObj.contains("McuTypeName"))
                                        {
                                            if(curMcuTypeName == mcuObj["McuTypeName"].toString())
                                            {
                                                mcuArray.removeAt(index);
                                                currObj["McuInfos"] = mcuArray;
                                                productObj[productName] = currObj;
                                                productDoc.setObject(productObj);
                                                if(st_client_file::Instance()->saveProductJson(productDoc))
                                                {
                                                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                                                    ackJsonObj["msg"] = "Succeeded in adding a new MCU type!";
                                                }
                                                else
                                                {
                                                    ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                                                    ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                                                }
                                                isExists = true;
                                                break;
                                            }
                                        }
                                        index++;
                                    }
                                    if(!isExists)
                                    {
                                        ackJsonObj["code"] = BdMcuTypeDontExists_E;
                                        ackJsonObj["msg"] ="The mcu type does not exist!";
                                    }
                                }
                            }
                            else
                            {
                                ackJsonObj["code"] = BdProductDontExist_E;
                                ackJsonObj["msg"] ="The product does not exist!";
                            }
                        }
                        else
                        {
                            ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                            ackJsonObj["msg"] ="Failed to open product information file!";
                        }
                    }
                    else
                    {
                        ackJsonObj["code"] = BdAddNewMcuTypeJsonParseError_E;
                        ackJsonObj["msg"] ="Add new type MCUJson parsing error!";
                    }
                }
                    break;
                case BdRequestAddUser_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("Name") && dataObj.contains("Password") && dataObj.contains("Auth") && dataObj.contains("Date")
                            && dataObj.contains("Message") && dataObj.contains("Products"))
                    {
                        USER_INFO_S user;
                        user.name = dataObj["Name"].toString();
                        user.password = dataObj["Password"].toString();
                        user.auth = dataObj["Auth"].toInt();
                        user.date = dataObj["Date"].toString();
                        user.message = dataObj["Message"].toString();
                        user.products.clear();
                        QJsonArray productArray = dataObj["Products"].toArray();
                        foreach (QJsonValue val, productArray) {
                            user.products << val.toString();
                        }
                        if(!st_client_file::Instance()->addAuthUser(user))
                        {
                            ackJsonObj["code"] = BdAddUserFailed_E;
                            ackJsonObj["msg"] = "Failed to add user!";
                            goto REPONSE_END;
                        }
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add new user successfully!";
                    }

                }
                    break;
                case BdRequestRemoveUser_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("Name") && dataObj.contains("Password") && dataObj.contains("Auth") && dataObj.contains("Date")
                            && dataObj.contains("Message") && dataObj.contains("Products"))
                    {
                        USER_INFO_S user;
                        user.name = dataObj["Name"].toString();
                        user.password = dataObj["Password"].toString();
                        user.auth = dataObj["Auth"].toInt();
                        user.date = dataObj["Date"].toString();
                        user.message = dataObj["Message"].toString();
                        user.products.clear();
                        QJsonArray productArray = dataObj["Products"].toArray();
                        foreach (QJsonValue val, productArray) {
                            user.products << val.toString();
                        }
                        if(!st_client_file::Instance()->removeAuthUser(user))
                        {
                            ackJsonObj["code"] = BdRemoveUserFailed_E;
                            ackJsonObj["msg"] = "Failed to remove user!";
                            goto REPONSE_END;
                        }
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove user successfully!";
                    }

                }
                    break;
                case BdRequestEditUser_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(dataObj.contains("Name") && dataObj.contains("Password") && dataObj.contains("Auth") && dataObj.contains("Date")
                            && dataObj.contains("Message") && dataObj.contains("Products"))
                    {
                        USER_INFO_S user;
                        user.name = dataObj["Name"].toString();
                        user.password = dataObj["Password"].toString();
                        user.auth = dataObj["Auth"].toInt();
                        user.date = dataObj["Date"].toString();
                        user.message = dataObj["Message"].toString();
                        user.products.clear();
                        QJsonArray productArray = dataObj["Products"].toArray();
                        foreach (QJsonValue val, productArray) {
                            user.products << val.toString();
                        }
                        if(!st_client_file::Instance()->editAuthUser(user))
                        {
                            ackJsonObj["code"] = BdEditUserFailed_E;
                            ackJsonObj["msg"] = "Failed to edit user!";
                            goto REPONSE_END;
                        }
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Edit user successfully!";
                    }

                }
                    break;
                case BdRequestGetUserList_E:
                {
                    QJsonDocument userDoc;
                    if(st_client_file::Instance()->openAuthUsers(userDoc))
                    {
                        ackJsonObj["data"] = userDoc.object();
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Get the user list successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdGetUserListFailed_E;
                        ackJsonObj["msg"] = "Failed to get the user list!";
                    }
                }
                    break;
                }
            }

        }
REPONSE_END:
        ackDoc.setObject(ackJsonObj);
        return res;
    }

    bool st_clientNodeAppLayer::receiveFileData(BD_TRANSFER_FILE_DATA_S &body)
    {
        bool res = false;
        if(m_uploadFileHandle == nullptr)
            return res;
        qint64 wb;
        if(body.currently_sent_bytes > sizeof (body.file_data))
            return res;
        wb = m_uploadFileHandle->write((char*)body.file_data,body.currently_sent_bytes);
        if(wb == body.currently_sent_bytes)
        {
            m_receivedBytes += body.currently_sent_bytes;
            if(m_receivedBytes == m_uploadFileInfo.fileSize && body.file_size == body.sent_bytes) //接收完成
            {
                closeUploadFile();
                res = true;
            }
            else if(m_receivedBytes > m_uploadFileInfo.fileSize || m_receivedBytes > body.sent_bytes) //传输发生错误
            {

            }
            else
                res = true;

        }
        return res;
    }

    void st_clientNodeAppLayer::closeUploadFile()
    {
        if(m_uploadFileHandle != nullptr)
        {
            if(m_uploadFileHandle->isOpen())
                m_uploadFileHandle->close();
            delete m_uploadFileHandle;
            m_uploadFileHandle = nullptr;
        }
    }

    bool st_clientNodeAppLayer::openUploadFile(const QString &fileName)
    {
        bool res = false,bExists = false;
        closeUploadFile();
        m_uploadFileHandle = new QFile(fileName);
        if(m_uploadFileHandle->exists())
        {
            res = m_uploadFileHandle->remove();
            bExists = true;
        }
        if(res || bExists == false)
        {
            if(m_uploadFileHandle->open(QIODevice::WriteOnly | QIODevice::Truncate))
                res = true;
        }
        if(!res)
            closeUploadFile();
        return res;
    }

    bool st_clientNodeAppLayer::sendFileData(BD_TMN_DOWNLOAD_FILE_DATA_S &body,BD_TRANSFER_FILE_DATA_S &tBody)
    {
        bool res = false;
        if(m_downloadFileHandle == nullptr)
            return res;
        if(body.file_size == m_downloadFileHandle->size() && m_sentBytes == body.received_bytes && body.request_receive_bytes > 0)
        {
            qint64 rb = m_downloadFileHandle->read((char*)tBody.file_data,body.request_receive_bytes);
            if(rb == body.request_receive_bytes)
            {
                m_sentBytes += rb;
                tBody.currently_sent_bytes = rb;
                tBody.file_size = body.file_size;
                tBody.sent_bytes = m_sentBytes;
                tBody.not_sent_bytes = body.file_size - m_sentBytes;
                if(m_sentBytes == body.file_size)
                    closeDownloadFile();
                res = true;
            }
        }

        if(!res)
            closeDownloadFile();
        return res;
    }

    bool st_clientNodeAppLayer::openDownloadFile(const QString &fileName)
    {
        bool res = false;
        closeDownloadFile();
        m_downloadFileHandle = new QFile(fileName);

        if(m_downloadFileHandle->exists())
        {
            if(m_downloadFileHandle->open(QIODevice::ReadOnly))
                res = true;
        }
        if(!res)
            closeDownloadFile();
        return res;
    }

    void st_clientNodeAppLayer::closeDownloadFile()
    {
        if(m_downloadFileHandle != nullptr)
        {
            if(m_downloadFileHandle->isOpen())
                m_downloadFileHandle->close();
            delete m_downloadFileHandle;
            m_downloadFileHandle = nullptr;
        }
    }

    /**
     * @brief mkMutiDir  创建多级目录
     * @param path	     未创建的目录
     * @return 	     返回已创建的目录
     */
    QString st_clientNodeAppLayer::mkMutiDir(const QString path){
        QDir dir(path);
        if ( dir.exists(path)){
            return path;
        }
        QString parentDir = mkMutiDir(path.mid(0,path.lastIndexOf('/')));
        QString dirname = path.mid(path.lastIndexOf('/') + 1);
        QDir parentPath(parentDir);
        if ( !dirname.isEmpty() )
            parentPath.mkpath(dirname);
        return parentDir + "/" + dirname;
    }

    bool st_clientNodeAppLayer::moveFile(const QString srcFullPath,const QString dstFullPath)
    {
        bool res = false;
        QFile srcFile(srcFullPath);
        QFile dstFile(dstFullPath);
        if(srcFile.exists())
        {
            QFileInfo fileInfo(dstFullPath);
            QDir dir(fileInfo.path());
            if(!dir.exists())
                mkMutiDir(fileInfo.path());
            if(dstFile.exists())
                dstFile.remove();
            res = srcFile.rename(dstFullPath);
        }
        return res;
    }

    //获取系统时间
    time_t st_clientNodeAppLayer::getSysTime(uint8_t *datetime,uint8_t len)
    {
        if(len < 6 || datetime == NULL)
            return 0;
        time_t rawtime;
        struct tm *info;
        time(&rawtime);
        //rawtime = CBdNetService::GetUTCTime(rawtime);
        info = gmtime(&rawtime );
        //tminfo = localtime(&rawTime);
        datetime[0] = (info->tm_year+1900) % 100;
        datetime[1] = info->tm_mon+1;
        datetime[2] = info->tm_mday;
        datetime[3] = info->tm_hour;
        datetime[4] = info->tm_min;
        datetime[5] = info->tm_sec;

        //printf("GetSysTime-----sec:%ld---------%02d/%02d/%02d %02d:%02d:%02d\n",rawtime, datetime[0],datetime[1],datetime[2],datetime[3],datetime[4],datetime[5]);
        return rawtime;
    }

}
