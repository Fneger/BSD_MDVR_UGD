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
#include "st_client_event.h"

namespace ExampleServer{

    QString st_clientNodeAppLayer::S_sSaveRootPath = "./version/";
    QString st_clientNodeAppLayer::S_sProductFile = "ProductInfo.json";
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

        m_uploadFileHandle.reset(new QFile);
        m_downloadFileHandle.reset(new QFile);
    }

    st_clientNodeAppLayer::~st_clientNodeAppLayer()
    {
        closeUploadFile();
        closeDownloadFile();
    }

	//!deal current message
    int st_clientNodeAppLayer::deal_current_message_block()
    {
        if(st_clientNode_baseTrans::deal_current_message_block() < 0)
            return -1;
        qDebug() << "Test......0";
        uint8_t *msgdata = (uint8_t*)&m_currentBlock.data()[14];
        qDebug() << "Test......1";
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
            if(body.currently_sent_bytes > datalen)
                sendPacket(BdPlatformResponse_E,NULL,BdDataTransmissionError_E);
            else
            {
                memcpy(body.file_data,&msgdata[msgpos],body.currently_sent_bytes);
                msgpos += body.currently_sent_bytes;
                if(receiveFileData(body))
                    sendPacket(BdPlatformResponse_E,NULL,0);
                else
                    sendPacket(BdPlatformResponse_E,NULL,BdDataTransmissionError_E);
            }
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
        case BdTerminalAckJson_E:
        {
            msg << "BdUgdServer Receive BdTerminalAckJson_E\n";
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
                if(m_userInfo.auth < 0 && requestCode != BdRequestStopDownloadFile_E
                        && requestCode != BdRequestUploadFile_E
                        && requestCode != BdRequestDownloadFile_E
                        && requestCode != BdRequestQuerySpecifyVersionInfo_E
                        && requestCode != BdRequestQueryDefaultVersionInfo_E
                        && requestCode != BdRequestQueryDeviceClientEvents_E
                        && requestCode != BdRequestSetEventStatus_E)
                {
                    ackJsonObj["code"] = BdUserNotLogged_E;
                    ackJsonObj["msg"] = "The user is not logged in!";
                    goto RESPONSE_END;
                }
                switch (requestCode) {
                case BdRequestAddMcuType_E:
                case BdRequestRemoveMcuVersion_E:
                case BdRequestAddNewMcuVersion_E:
                case BdRequestRemoveCrcVersion_E:
                case BdRequestAddNewCrcVersion_E:
                case BdRequestSetDefaultCrcVersion_E:
                case BdRequestAddOtherFileVersion_E:
                case BdRequestSetDefaultMcuVersion_E:
                case BdRequestSaveProductInfo_E: //添加产品
                {
                    QJsonObject newProductObj = jsonObj["data"].toObject();
                    if(!newProductObj.contains("ProductName") || !newProductObj.contains("ProductTypeId") || !newProductObj.contains("Date")
                            || !newProductObj.contains("ProductDesp") || !newProductObj.contains("CrcDefaultVersion") || !newProductObj.contains("SaveFilePath")
                            || !newProductObj.contains("McuInfos") || !newProductObj.contains("GroupFileDefaultVersion") || !newProductObj.contains("GroupFileInfos"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    QString saveFilePath = newProductObj["SaveFilePath"].toString();
                    QString editProduct = newProductObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(editProduct);
                    QJsonDocument productDoc;
                    if(st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        QJsonObject productObj = productDoc.object();


                        productObj[editProduct] = newProductObj;
                        //判断是否有新版本加入到历史版本中
                        //......
                        productDoc.setObject(productObj);
                        if(st_client_file::Instance()->saveProductJson(jsonFile,productDoc))
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
                case BdRequestAddProductInfo_E:
                {
                    QJsonObject newProductObj = jsonObj["data"].toObject();
                    if(!newProductObj.contains("ProductName") || !newProductObj.contains("ProductTypeId") || !newProductObj.contains("Date")
                            || !newProductObj.contains("ProductDesp") || !newProductObj.contains("CrcDefaultVersion") || !newProductObj.contains("SaveFilePath")
                            || !newProductObj.contains("McuInfos") || !newProductObj.contains("GroupFileDefaultVersion") || !newProductObj.contains("GroupFileInfos"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    QString productName = newProductObj["ProductName"].toString();
                    QString saveFilePath = getProductSavePath(productName);
                    QJsonObject productObj;
                    productObj[productName] = newProductObj;
                    QJsonDocument productDoc;
                    productDoc.setObject(productObj);
                    QDir dir;
                    dir.setPath(saveFilePath);
                    bool res = false;
                    if(dir.exists())
                        res = true;
                    else
                    {
                        dir.setPath(S_sSaveRootPath);
                        res = dir.mkdir("./" + productName);
                    }
                    if(res)
                    {
                        QString jsonFile = getProductInfoFileName(productName);
                        if(st_client_file::Instance()->saveProductJson(jsonFile,productDoc))
                        {
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] = "Add product successfully!";
                        }
                        else
                            res = false;
                    }
                    if(!res)
                    {
                        dir.setPath(saveFilePath);
                        dir.removeRecursively();
                        ackJsonObj["code"] = BdAddProductInfoFalied_E;
                        ackJsonObj["msg"] = "Add product failed!";
                    }
                }
                    break;
                case BdRequestDeleteProductInfo_E:
                {
                    QJsonObject newProductObj = jsonObj["data"].toObject();
                    if(!newProductObj.contains("ProductName") || !newProductObj.contains("ProductTypeId") || !newProductObj.contains("Date")
                            || !newProductObj.contains("ProductDesp") || !newProductObj.contains("CrcDefaultVersion") || !newProductObj.contains("SaveFilePath")
                            || !newProductObj.contains("McuInfos") || !newProductObj.contains("GroupFileDefaultVersion") || !newProductObj.contains("GroupFileInfos"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    QString saveFilePath = newProductObj["SaveFilePath"].toString();
                    QDir dir;
                    dir.setPath(saveFilePath);
                    bool res = dir.removeRecursively();
                    if(res)
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove product successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdDeleteProductInfoFailed_E;
                        ackJsonObj["msg"] = "Remove product failed!";
                    }
                }
                    break;
                case BdRequestQueryProductList_E:
                {
                    if(m_userInfo.auth == BdUserAuthAdmin_E)
                    {
                        QDir dir(S_sSaveRootPath);
                        QStringList nameFilters;
                        QStringList products = dir.entryList(nameFilters, QDir::Dirs, QDir::Name);
                        QJsonDocument productDoc;
                        QJsonArray jsonArray;
                        foreach (QString key, products) {
                            if(key != "." && key != "..")
                                jsonArray << key;
                        }
                        ackJsonObj["data"] = jsonArray;
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] ="Query the product list successfully!";
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
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    QString productName = dataObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(productName);
                    QJsonDocument productDoc;
                    if(st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        QJsonObject productObj = productDoc.object();
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
                case BdRequestRemoveMcuType_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("McuTypeName"))
                    {
                        ackJsonObj["code"] = BdAddNewMcuTypeJsonParseError_E;
                        ackJsonObj["msg"] ="Add new type MCUJson parsing error!";
                        goto RESPONSE_END;
                    }

                    QString productName = dataObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(productName);
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto RESPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }
                    QJsonObject currObj = productObj[productName].toObject();
                    if(!currObj.contains("McuInfos"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
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
                                if(st_client_file::Instance()->saveProductJson(jsonFile,productDoc))
                                {
                                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                                    ackJsonObj["msg"] = "Succeeded in adding a new MCU type!";
                                }
                                else
                                {
                                    ackJsonObj["code"] = BdSaveProductInfoFileFailed_E;
                                    ackJsonObj["msg"] ="Saving failed, Failed to save product information file!";
                                    goto RESPONSE_END;
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
                    break;
                case BdRequestAddGroupFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("GroupFileInfo"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Add new group file parsing error!";
                        goto RESPONSE_END;
                    }
                    QString productName = dataObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(productName);
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto RESPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }

                    QJsonObject currObj = productObj[productName].toObject();
                    QJsonArray groupFileArray;
                    if(currObj.contains("GroupFileInfos"))
                        groupFileArray = currObj["GroupFileInfos"].toArray();

                    QJsonObject groupFileObj = dataObj["GroupFileInfo"].toObject();
                    if(!groupFileObj.contains("Name") || !groupFileObj.contains("Date") || !groupFileObj.contains("SavePath") || !groupFileObj.contains("Message")
                            || !groupFileObj.contains("IsDefault") || !groupFileObj.contains("FileInfos"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Add new group file parsing error!";
                        goto RESPONSE_END;
                    }

                    groupFileArray << groupFileObj;
                    currObj["GroupFileInfos"] = groupFileArray;
                    productObj[productName] = currObj;
                    productDoc.setObject(productObj);
                    QString name = groupFileObj["Name"].toString();
                    QString savePath = getProductSavePath(productName) + name + "/";
                    QDir dir;
                    dir.setPath(savePath);
                    bool res = false;
                    if(dir.exists())
                        res = true;
                    else
                    {
                        dir.setPath(getProductSavePath(productName));
                        res = dir.mkdir("./" + name);
                    }
                    if(res)
                    {
                        if(st_client_file::Instance()->saveProductJson(jsonFile,productDoc))
                        {
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] = "Succeeded in adding a group file!";
                        }
                        else
                            res = false;
                    }
                    if(!res)
                    {
                        dir.setPath(savePath);
                        dir.removeRecursively();
                        ackJsonObj["code"] = BdAddProductInfoFalied_E;
                        ackJsonObj["msg"] = "Add group file failed!";
                    }
                }
                    break;
                case BdRequestRemoveGroupFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("GroupFileInfo"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Add new group file parsing error!";
                        goto RESPONSE_END;
                    }
                    QString productName = dataObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(productName);
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto RESPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();
                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }

                    QJsonObject currObj = productObj[productName].toObject();
                    QJsonArray groupFileArray;
                    if(currObj.contains("GroupFileInfos"))
                        groupFileArray = currObj["GroupFileInfos"].toArray();

                    QJsonObject groupFileObj = dataObj["GroupFileInfo"].toObject();
                    if(!groupFileObj.contains("Name") || !groupFileObj.contains("Date") || !groupFileObj.contains("SavePath") || !groupFileObj.contains("Message")
                            || !groupFileObj.contains("IsDefault"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Add new group file parsing error!";
                        goto RESPONSE_END;
                    }
                    QString name = groupFileObj["Name"].toString();
                    QString savePath = getProductSavePath(productName) + name + "/";
                    QDir dir;
                    dir.setPath(savePath);
                    bool res = false;
                    if(dir.exists())
                    {
                        res = dir.removeRecursively();
                    }
                    else
                        res = true;
                    if(res)
                    {
                        int index = 0;
                        foreach (QJsonValue groupFileValue, groupFileArray) {
                            QJsonObject groupFileObj = groupFileValue.toObject();
                            if(!groupFileObj.contains("Name"))
                                continue;
                            if(groupFileObj["Name"].toString() == name)
                            {
                                groupFileArray.removeAt(index);
                                break;
                            }
                            index++;
                        }
                        currObj["GroupFileInfos"] = groupFileArray;
                        productObj[productName] = currObj;
                        productDoc.setObject(productObj);
                        if(st_client_file::Instance()->saveProductJson(jsonFile,productDoc))
                        {
                            ackJsonObj["code"] = BdExecutionSucceeded_E;
                            ackJsonObj["msg"] = "Succeeded in remove a group file!";
                        }
                        else
                            res = false;
                    }

                    if(!res)
                    {
                        ackJsonObj["code"] = BdAddProductInfoFalied_E;
                        ackJsonObj["msg"] = "Remove group file failed!";
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
                            goto RESPONSE_END;
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
                            goto RESPONSE_END;
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
                            goto RESPONSE_END;
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
                case BdRequestQuerySpecifyVersionInfo_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("FileType") || !dataObj.contains("FileSubType") || !dataObj.contains("VersionName"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    QString productName = dataObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(productName);
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto RESPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();

                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }
                    QString fileType = dataObj["FileType"].toString();
                    QString fileSubType = dataObj["FileSubType"].toString();
                    QString versionName =  dataObj["VersionName"].toString();

                    QJsonObject currObj = productObj[productName].toObject();
                    if(!currObj.contains("SaveFilePath"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    bool finded = false;
                    if(fileType == "CRC")
                    {
                        if(!currObj.contains("CrcVersionList"))
                        {
                            ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                            ackJsonObj["msg"] ="CRC version Json parsing error!";
                            goto RESPONSE_END;
                        }
                        QJsonArray crcArray;
                        crcArray = currObj["CrcVersionList"].toArray();
                        foreach (QJsonValue crcValue, crcArray) {
                            QJsonObject crcObj = crcValue.toObject();
                            if(!crcObj.contains("Name"))
                                continue;
                            if(crcObj["Name"].toString() == versionName)
                            {
                                QJsonObject dataObj;
                                dataObj["SaveFilePath"] = currObj["SaveFilePath"];
                                dataObj["VersionInfo"] = crcObj;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                                break;
                            }
                        }
                    }
                    else if(fileType == "MCU")
                    {
                        if(!currObj.contains("McuInfos"))
                        {
                            ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                            ackJsonObj["msg"] = "The MCU information parsing error in the product information!";
                            goto RESPONSE_END;
                        }
                        QJsonArray mcuTypeArray = currObj["McuInfos"].toArray();
                        foreach (QJsonValue mcuTypeVal, mcuTypeArray) {
                            QJsonObject mcuTypeObj = mcuTypeVal.toObject();
                            if(!mcuTypeObj.contains("McuTypeName"))
                                continue;
                            if(mcuTypeObj["McuTypeName"].toString() == fileSubType)
                            {
                                if(!mcuTypeObj.contains("McuVersionList"))
                                {
                                    ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                                    ackJsonObj["msg"] = "The MCU information parsing error in the product information!";
                                    goto RESPONSE_END;
                                }
                                QJsonArray mcuArray = mcuTypeObj["McuVersionList"].toArray();
                                foreach (QJsonValue mcuVal, mcuArray) {
                                    QJsonObject mcuObj = mcuVal.toObject();
                                    if(!mcuObj.contains("Name"))
                                        continue;
                                    if(mcuObj["Name"].toString() == versionName)
                                    {
                                        QJsonObject dataObj;
                                        dataObj["SaveFilePath"] = currObj["SaveFilePath"];
                                        dataObj["VersionInfo"] = mcuObj;
                                        ackJsonObj["data"] = dataObj;
                                        finded = true;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                    else if(fileType == "GROUP")
                    {
                        if(!currObj.contains("GroupFileInfos"))
                        {
                            ackJsonObj["code"] = BdJsonParseError_E;
                            ackJsonObj["msg"] = "The group file information parsing error in the product information!";
                            goto RESPONSE_END;
                        }
                        QJsonArray groupFileArray = currObj["GroupFileInfos"].toArray();
                        foreach (QJsonValue groupFileVal, groupFileArray) {
                            QJsonObject groupFileObj = groupFileVal.toObject();
                            if(!groupFileObj.contains("Name"))
                                continue;
                            if(groupFileObj["Name"].toString() == versionName)
                            {
                                QJsonObject dataObj;
                                dataObj["SaveFilePath"] = currObj["SaveFilePath"];
                                dataObj["VersionInfo"] = groupFileObj;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                                break;
                            }
                        }
                    }
                    if(finded)
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Query version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdCrcVersionDontExists_E;
                        ackJsonObj["msg"] = "The version do not exists!";
                    }
                }
                    break;
                case BdRequestQueryDefaultVersionInfo_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("FileType") || !dataObj.contains("FileSubType"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    QString productName = dataObj["ProductName"].toString();
                    QString jsonFile = getProductInfoFileName(productName);
                    QJsonDocument productDoc;
                    if(!st_client_file::Instance()->openProductJson(jsonFile,productDoc))
                    {
                        ackJsonObj["code"] = BdOpenProductInfoFileFailed_E;
                        ackJsonObj["msg"] ="Failed to open product information file!";
                        goto RESPONSE_END;
                    }
                    QJsonObject productObj = productDoc.object();

                    if(!productObj.contains(productName))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }
                    QString fileType = dataObj["FileType"].toString();
                    QString fileSubType = dataObj["FileSubType"].toString();

                    QJsonObject currObj = productObj[productName].toObject();
                    if(!currObj.contains("SaveFilePath"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    bool finded = false;
                    if(fileType == "CRC")
                    {
                        if(!currObj.contains("CrcDefaultVersion"))
                        {
                            ackJsonObj["code"] = BdAddNewCrcVersionJsonParseError_E;
                            ackJsonObj["msg"] ="CRC version Json parsing error!";
                            goto RESPONSE_END;
                        }
                        QJsonObject crcObj = currObj["CrcDefaultVersion"].toObject();
                        QJsonObject dataObj;
                        dataObj["SaveFilePath"] = currObj["SaveFilePath"];
                        dataObj["VersionInfo"] = crcObj;
                        ackJsonObj["data"] = dataObj;
                        finded = true;
                    }
                    else if(fileType == "MCU")
                    {
                        if(!currObj.contains("McuInfos"))
                        {
                            ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                            ackJsonObj["msg"] = "The MCU information parsing error in the product information!";
                            goto RESPONSE_END;
                        }
                        QJsonArray mcuTypeArray = currObj["McuInfos"].toArray();
                        foreach (QJsonValue mcuTypeVal, mcuTypeArray) {
                            QJsonObject mcuTypeObj = mcuTypeVal.toObject();
                            if(!mcuTypeObj.contains("McuTypeName"))
                                continue;
                            if(mcuTypeObj["McuTypeName"].toString() == fileSubType)
                            {
                                if(!mcuTypeObj.contains("McuDefaultVersion"))
                                {
                                    ackJsonObj["code"] = BdProductMcuInfoParseError_E;
                                    ackJsonObj["msg"] = "The MCU information parsing error in the product information!";
                                    goto RESPONSE_END;
                                }
                                QJsonObject mcuObj = mcuTypeObj["McuDefaultVersion"].toObject();
                                dataObj["SaveFilePath"] = currObj["SaveFilePath"];
                                dataObj["VersionInfo"] = mcuObj;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                                break;
                            }
                        }
                    }
                    else if(fileType == "GROUP")
                    {
                        if(!currObj.contains("GroupFileDefaultVersion"))
                        {
                            ackJsonObj["code"] = BdJsonParseError_E;
                            ackJsonObj["msg"] = "The group file information parsing error in the product information!";
                            goto RESPONSE_END;
                        }
                        QJsonObject groupFileObj = currObj["GroupFileDefaultVersion"].toObject();

                        dataObj["SaveFilePath"] = currObj["SaveFilePath"];
                        dataObj["VersionInfo"] = groupFileObj;
                        ackJsonObj["data"] = dataObj;
                        finded = true;
                    }
                    if(finded)
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Query version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdCrcVersionDontExists_E;
                        ackJsonObj["msg"] = "The default version do not exists!";
                    }
                }
                    break;
                case BdRequestDeleteFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("FileFullPath"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    bool res = false;
                    QString fileName = dataObj["FileFullPath"].toString();
                    QFile file(fileName);
                    if(!file.exists())
                        res = true;
                    else
                        res = file.remove();
                    if(res)
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Delete file successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdRemoveFileFailed_E;
                        ackJsonObj["msg"] = "Failed to remove file!";
                    }
                }
                    break;
                case BdRequestAddEvent_E: //PC
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("EventSn") || !dataObj.contains("EventType") || !dataObj.contains("IdType") || !dataObj.contains("Id") || !dataObj.contains("EventPara1"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    EVENT_INFO_S event;
                    event.ownerUuid = m_uuid;
                    event.eventSn = dataObj["EventSn"].toString().toULongLong();
                    event.eventType = dataObj["EventType"].toInt();
                    event.idType = dataObj["IdType"].toInt();
                    event.id = dataObj["Id"].toString();
                    event.eventPara1 = dataObj["EventPara1"].toString();
                    event.status = EventStatusWaitting_E;
                    if(dataObj.contains("EventPara2"))
                        event.eventPara2 = dataObj["EventPara2"].toString();
                    if(dataObj.contains("AckPara1"))
                        event.ackPara1 = dataObj["AckPara1"].toString();
                    if(dataObj.contains("AckPara2"))
                        event.ackPara2 = dataObj["AckPara2"].toString();
                    st_client_event::Instance()->AddClientEvent(event);
                    dataObj["EventSn"] = QString::number(event.eventSn);
                    ackJsonObj["data"] = dataObj;
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Add client event successfully!";
                }
                    break;
                case BdRequestRemoveEvent_E: //PC
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("EventSn"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    quint64 eventSn = dataObj["EventSn"].toString().toULongLong();
                    st_client_event::Instance()->RemoveClientEvent(eventSn);

                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Remove client event successfully!";
                }
                    break;
                case BdRequestQueryPcClientEvents_E:
                {
                    QJsonObject dataObj;
                    QJsonArray eventArray;
                    QMap<quint64,EVENT_INFO_S> events = st_client_event::Instance()->GetUuidEvents(m_uuid);
                    foreach (EVENT_INFO_S event, events) {
                        QJsonObject eventObj;
                        eventObj["EventSn"] = QString::number(event.eventSn);
                        eventObj["EventType"] = event.eventType;
                        eventObj["IdType"] = event.idType;
                        eventObj["Id"] = event.id;
                        eventObj["EventPara1"] = event.eventPara1;
                        eventObj["EventPara2"] = event.eventPara2;
                        eventObj["AckPara1"] = event.ackPara1;
                        eventObj["AckPara2"] = event.ackPara2;
                        eventObj["Status"] = event.status;
                        eventArray << eventObj;
                    }
                    ackJsonObj["data"] = eventArray;
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Query client events successfully!";
                }
                    break;
                case BdRequestSetEventStatus_E: //Device/PC
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("EventSn") || !dataObj.contains("Status") || !dataObj.contains("AckPara1") || !dataObj.contains("AckPara2"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    BD_REQUEST_SET_EVENT_STATUS_S ack;
                    ack.eventSn = dataObj["EventSn"].toString().toULongLong();
                    ack.status = dataObj["Status"].toInt();
                    ack.ackPara1 = dataObj["AckPara1"].toString();
                    ack.ackPara2 = dataObj["AckPara2"].toString();
                    st_client_event::Instance()->SetEventStatus(ack);
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Set client event status successfully!";
                }
                    break;
                case BdRequestQueryEventStatus_E: //PC
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("EventSn") || !dataObj.contains("Status"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    quint64 eventSn = dataObj["EventSn"].toString().toULongLong();
                    int status = st_client_event::Instance()->GetEventStatus(eventSn);
                    dataObj["Status"] = status;
                    ackJsonObj["data"] = dataObj;
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Query client event status successfully!";
                }
                    break;
                case BdRequestQueryDeviceClientEvents_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("DeviceNum") || !dataObj.contains("PhoneNumber") || !dataObj.contains("LicenseNum") || !dataObj.contains("VideoId")
                            || !dataObj.contains("TerminalId"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }
                    m_terminalInfo.deviceNum = dataObj["DeviceNum"].toString();
                    m_terminalInfo.phoneNumber = dataObj["PhoneNumber"].toString();
                    m_terminalInfo.licenseNum = dataObj["LicenseNum"].toString();
                    m_terminalInfo.videoId = dataObj["VideoId"].toString();
                    m_terminalInfo.terminalId = dataObj["TerminalId"].toString();
                    QJsonArray eventArray;
                    QMap<quint64,EVENT_INFO_S> events = st_client_event::Instance()->GetTerminalEvents(m_terminalInfo);
                    foreach (EVENT_INFO_S event, events) {
                        QJsonObject eventObj;
                        eventObj["EventSn"] = QString::number(event.eventSn);
                        eventObj["EventType"] = event.eventType;
                        eventObj["IdType"] = event.idType;
                        eventObj["Id"] = event.id;
                        eventObj["EventPara1"] = event.eventPara1;
                        eventObj["EventPara2"] = event.eventPara2;
                        eventObj["AckPara1"] = event.ackPara1;
                        eventObj["AckPara2"] = event.ackPara2;
                        eventObj["Status"] = event.status;
                        eventArray << eventObj;
                    }
                    ackJsonObj["data"] = eventArray;
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Query client events successfully!";
                }
                    break;
                }
            }
        }
RESPONSE_END:
        ackDoc.setObject(ackJsonObj);
        return res;
    }

    bool st_clientNodeAppLayer::receiveFileData(BD_TRANSFER_FILE_DATA_S &body)
    {
        bool res = false;
        if(!m_uploadFileHandle->isOpen())
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
            else if(m_receivedBytes < m_uploadFileInfo.fileSize)
                res = true;
        }
        if(!res)
            closeUploadFile();
        return res;
    }

    void st_clientNodeAppLayer::closeUploadFile()
    {
        if(!m_uploadFileHandle.isNull())
        {
            if(m_uploadFileHandle->isOpen())
                m_uploadFileHandle->close();
        }
    }

    bool st_clientNodeAppLayer::openUploadFile(const QString &fileName)
    {
        bool res = false,bExists = false;
        closeUploadFile();

        m_uploadFileHandle->setFileName(fileName);
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
        if(!m_downloadFileHandle->isOpen())
            return res;
        if(m_sentBytes == body.received_bytes && body.request_receive_bytes > 0)
        {
            qint64 rb = m_downloadFileHandle->read((char*)tBody.file_data,body.request_receive_bytes);
            if(rb == body.request_receive_bytes)
            {
                qDebug() << "sendFileData" << rb;
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
        m_downloadFileHandle->setFileName(fileName);
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

    QString st_clientNodeAppLayer::getProductInfoFileName(const QString &product)
    {
        return S_sSaveRootPath + product + "/" + S_sProductFile;
    }

    QString st_clientNodeAppLayer::getProductSavePath(const QString &product)
    {
        return S_sSaveRootPath + product + "/";
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
