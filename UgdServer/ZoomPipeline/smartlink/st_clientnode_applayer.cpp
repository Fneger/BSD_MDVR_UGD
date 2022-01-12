#include "st_clientnode_applayer.h"
#include "st_client_table.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
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

        m_fileTool.reset(new st_client_file);
        m_fileTool->addAuthUser(userInfo);

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
        updateActiveTime();
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
            if(m_fileTool->loginClient(loginInfo,m_userInfo))
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
             //msg << "BdUgdServer Receive BdTerminalJson_E\n";
//             QByteArray json;
//             json.append((char*)msgdata,strlen((char*)msgdata)+1);
//             //strcpy(json,(char*)msgdata);
             //msg << QString::fromUtf8((char*)msgdata) + "\n";
             //qDebug() << "Terminal Json" << QString::fromUtf8((char*)msgdata);
             QJsonDocument ackDoc;
             reponseRequest((char*)msgdata,ackDoc);
             //qDebug() << "Ack Json" << ackDoc;
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
        //qDebug() << msg;
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
                        && requestCode != BdRequestSetEventStatus_E
                        && requestCode != BdRequestAddLog_E)
                {
                    ackJsonObj["code"] = BdUserNotLogged_E;
                    ackJsonObj["msg"] = "The user is not logged in!";
                    goto RESPONSE_END;
                }
                switch (requestCode) {
                case BdRequestAddNewMcuVersion_E:
                case BdRequestAddMcuType_E:
                {
                    QJsonObject mcuObj = jsonObj["data"].toObject();
                    if(!mcuObj.contains("ProductName") || !mcuObj.contains("McuTypeName"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Mcu version information parsing error!";
                        goto RESPONSE_END;
                    }
                    BODY_MCU_VERSION_INFO_S info;
                    info.productName = mcuObj["ProductName"].toString();
                    info.mcuTypeName = mcuObj["McuTypeName"].toString();
                    if(mcuObj.contains("Name") && mcuObj.contains("Date") && mcuObj.contains("IsDefault")
                            && mcuObj.contains("Message") && mcuObj.contains("FileName") && mcuObj.contains("FileSize"))
                    {
                        info.name = mcuObj["Name"].toString();
                        info.date = mcuObj["Date"].toString();
                        info.isDefaultVersion = mcuObj["IsDefault"].toBool();
                        info.message = mcuObj["Message"].toString();
                        info.fileName = mcuObj["FileName"].toString();
                        info.fileSize = mcuObj["FileSize"].toString().toUInt();
                    }
                    else
                    {
                        info.name = "";
                        info.date = "";
                        info.isDefaultVersion = false;
                        info.message = "";
                        info.fileName = "";
                        info.fileSize = 0;
                    }

                    GS_DB_BODY_C body = info;
                    if(m_fileTool->addDb(TB_MCU_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add mcu version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Add mcu version failed!";
                    }
                }
                    break;
                case BdRequestRemoveMcuType_E:
                case BdRequestRemoveMcuVersion_E:
                {
                    QJsonObject mcuObj = jsonObj["data"].toObject();
                    if(!mcuObj.contains("ProductName") || !mcuObj.contains("McuTypeName"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Mcu version information parsing error!";
                        goto RESPONSE_END;
                    }
                    BODY_MCU_VERSION_INFO_S info;
                    info.productName = mcuObj["ProductName"].toString();
                    info.mcuTypeName = mcuObj["McuTypeName"].toString();
                    if(mcuObj.contains("Name"))
                    {
                        info.name = mcuObj["Name"].toString();
                    }

                    GS_DB_BODY_C body = info;
                    if(m_fileTool->removeDb(TB_MCU_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove mcu version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Remove mcu version failed!";
                    }
                }
                    break;
                case BdRequestSetDefaultMcuVersion_E:
                {
                    QJsonObject mcuObj = jsonObj["data"].toObject();
                    if(!mcuObj.contains("ProductName") || !mcuObj.contains("McuTypeName")
                            || !mcuObj.contains("Name") || !mcuObj.contains("Date") || !mcuObj.contains("IsDefault")
                            || !mcuObj.contains("Message") || !mcuObj.contains("FileName") || !mcuObj.contains("FileSize"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Mcu version information parsing error!";
                        goto RESPONSE_END;
                    }
                    BODY_MCU_VERSION_INFO_S info;
                    info.productName = mcuObj["ProductName"].toString();
                    info.mcuTypeName = mcuObj["McuTypeName"].toString();
                    info.name = mcuObj["Name"].toString();
                    info.date = mcuObj["Date"].toString();
                    info.isDefaultVersion = mcuObj["IsDefault"].toBool();
                    info.message = mcuObj["Message"].toString();
                    info.fileName = mcuObj["FileName"].toString();
                    info.fileSize = mcuObj["FileSize"].toString().toUInt();
                    GS_DB_BODY_C body = info;
                    if(!m_fileTool->queryDb(TB_MCU_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Mcu version do not exists!";
                        goto RESPONSE_END;
                    }
                    info = boost::any_cast<BODY_MCU_VERSION_INFO_S>(body);
                    info.isDefaultVersion = true;

                    QList<BODY_MCU_VERSION_INFO_S> infos;
                    body = infos;
                    QString queryStr = QString("product_name = \"%1\" AND mcu_type_name = \"%2\" AND is_default_version = TRUE").arg(info.productName).arg(info.mcuTypeName);
                    if(m_fileTool->queryDb(TB_MCU_VERSION_INFO_E, body, queryStr))
                    {
                        infos = boost::any_cast<QList<BODY_MCU_VERSION_INFO_S>>(body);
                        foreach (BODY_MCU_VERSION_INFO_S info1, infos) {
                            if(info1.isDefaultVersion)
                            {qDebug() << __FUNCTION__ << __LINE__ << info1.mcuTypeName << info1.name;
                                info1.isDefaultVersion = false;
                                body = info1;
                                m_fileTool->updateDb(TB_MCU_VERSION_INFO_E, body);
                            }
                        }
                    }

                    body = info;
                    if(m_fileTool->updateDb(TB_MCU_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Set mcu default version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Set mcu default version failed!";
                    }
                }
                    break;
                case BdRequestAddNewCrcVersion_E:
                {
                    QJsonObject crcObj = jsonObj["data"].toObject();
                    if(!crcObj.contains("ProductName") || !crcObj.contains("Name") || !crcObj.contains("Date") || !crcObj.contains("IsDefault")
                            || !crcObj.contains("Message") || !crcObj.contains("FileName") || !crcObj.contains("FileSize"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Crc version information parsing error!";
                        goto RESPONSE_END;
                    }
                    BODY_CRC_VERSION_INFO_S info;
                    info.productName = crcObj["ProductName"].toString();
                    info.name = crcObj["Name"].toString();
                    info.date = crcObj["Date"].toString();
                    info.isDefaultVersion = crcObj["IsDefault"].toBool();
                    info.message = crcObj["Message"].toString();
                    info.fileName = crcObj["FileName"].toString();
                    info.fileSize = crcObj["FileSize"].toString().toUInt();
                    if(crcObj.contains("Custom"))
                        info.custom = crcObj["Custom"].toString();
                    GS_DB_BODY_C body = info;
                    if(m_fileTool->addDb(TB_CRC_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add crc version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Add crc version failed!";
                    }
                }
                    break;
                case BdRequestRemoveCrcVersion_E:
                {
                    QJsonObject crcObj = jsonObj["data"].toObject();
                    if(!crcObj.contains("ProductName") || !crcObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Crc version information parsing error!";
                        goto RESPONSE_END;
                    }
                    BODY_CRC_VERSION_INFO_S info;
                    info.productName = crcObj["ProductName"].toString();
                    info.name = crcObj["Name"].toString();
                    GS_DB_BODY_C body = info;
                    if(m_fileTool->removeDb(TB_CRC_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Remove crc version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Remove crc version failed!";
                    }
                }
                    break;
                case BdRequestSetDefaultCrcVersion_E:
                {
                    QJsonObject crcObj = jsonObj["data"].toObject();
                    if(!crcObj.contains("ProductName") || !crcObj.contains("Name") || !crcObj.contains("Date") || !crcObj.contains("IsDefault")
                            || !crcObj.contains("Message") || !crcObj.contains("FileName") || !crcObj.contains("FileSize"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Crc version information parsing error!";
                        goto RESPONSE_END;
                    }
                    BODY_CRC_VERSION_INFO_S info;
                    info.productName = crcObj["ProductName"].toString();
                    info.name = crcObj["Name"].toString();
                    info.date = crcObj["Date"].toString();
                    info.isDefaultVersion = crcObj["IsDefault"].toBool();
                    info.message = crcObj["Message"].toString();
                    info.fileName = crcObj["FileName"].toString();
                    info.fileSize = crcObj["FileSize"].toString().toUInt();
                    if(crcObj.contains("Custom"))
                        info.custom = crcObj["Custom"].toString();

                    GS_DB_BODY_C body = info;
                    if(!m_fileTool->queryDb(TB_CRC_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Crc version do not exists!";
                        goto RESPONSE_END;
                    }

                    info = boost::any_cast<BODY_CRC_VERSION_INFO_S>(body);
                    info.custom = GetCustom(info.name);
                    info.isDefaultVersion = true;

                    QList<BODY_CRC_VERSION_INFO_S> infos;
                    body = infos;
                    QString queryStr = QString("product_name = \"%1\" AND is_default_version = TRUE").arg(info.productName);
                    if(m_fileTool->queryDb(TB_CRC_VERSION_INFO_E, body, queryStr)) //查到默认版本，根绝定制信息确认是否需要去掉默认
                    {
                        QList<BODY_CRC_VERSION_INFO_S> infos = boost::any_cast<QList<BODY_CRC_VERSION_INFO_S>>(body);
                        foreach (BODY_CRC_VERSION_INFO_S info1, infos) {
                            QString custom = GetCustom(info1.name);
                            if(custom == info.custom)
                            {
                                info1.isDefaultVersion = false;
                                body = info1;
                                m_fileTool->updateDb(TB_CRC_VERSION_INFO_E, body);
                            }
                        }
                    }


                    body = info;
                    if(m_fileTool->updateDb(TB_CRC_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Set crc default version successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Set crc default version failed!";
                    }
                }
                    break;
                case BdRequestAddFileToGroupFile_E:
                case BdRequestAddGroupFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("IsDefault")
                            || !dataObj.contains("Message") || !dataObj.contains("Name") || !dataObj.contains("SavePath") || !dataObj.contains("Date"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Add new group file parsing error!";
                        goto RESPONSE_END;
                    }
                    QJsonDocument productDoc;
                    QString productName = dataObj["ProductName"].toString();
                    BODY_PRODUCT_INFO_S info1;
                    info1.name = productName;
                    GS_DB_BODY_C body = info1;
                    if(!m_fileTool->queryDb(TB_PRODUCTS_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }
                    QString name = dataObj["Name"].toString();

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
                        res = false;
                        BODY_GROUP_VERSION_INFO_S info2;
                        info2.productName = productName;
                        info2.name = dataObj["Name"].toString();
                        info2.date = dataObj["Date"].toString();
                        info2.isDefaultVersion = dataObj["IsDefault"].toBool();
                        info2.message = dataObj["Message"].toString();
                        info2.savePath = dataObj["SavePath"].toString();
                        if(dataObj.contains("FileInfo")) //添加文件到文件夹中
                        {
                            QJsonObject fileObj = dataObj["FileInfo"].toObject();
                            info2.subFileDate = fileObj["Date"].toString();
                            info2.subFileName = fileObj["Name"].toString();
                            info2.subFileSize = fileObj["FileSize"].toString().toUInt();
                            info2.subFileMessage = fileObj["Message"].toString();
                            info2.subFilePath = fileObj["FileName"].toString();
                        }
                        else
                        {
                            info2.subFileDate = "";
                            info2.subFileName = "";
                            info2.subFileSize = 0;
                            info2.subFileMessage = "";
                            info2.subFilePath = "";
                        }
                        body = info2;
                        res = m_fileTool->addDb(TB_GROUP_VERSION_INFO_E, body);
                    }
                    if(!res)
                    {
                        dir.setPath(savePath);
                        dir.removeRecursively();
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Add group file failed!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Succeeded to add group file!";
                    }
                }
                    break;
                case BdRequestRemoveGroupFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Remove group file parsing error!";
                        goto RESPONSE_END;
                    }

                    QString productName = dataObj["ProductName"].toString();
                    QString name = dataObj["Name"].toString();
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
                    if(!res)
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Remove group file dir failed!";
                    }
                    BODY_GROUP_VERSION_INFO_S info;
                    info.productName = dataObj["ProductName"].toString();
                    info.name = dataObj["Name"].toString();
                    GS_DB_BODY_C body = info;
                    if(m_fileTool->removeDb(TB_GROUP_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Succeeded remove a group file!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Remove group file failed!";
                    }

                }
                    break;
                case BdRequestRemoveFilesFromGroupFile_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("Name") || !dataObj.contains("SubFileName"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Remove file from group file parsing error!";
                        goto RESPONSE_END;
                    }

                    BODY_GROUP_VERSION_INFO_S info;
                    info.productName = dataObj["ProductName"].toString();
                    info.name = dataObj["Name"].toString();
                    info.subFileName = dataObj["SubFileName"].toString();
                    GS_DB_BODY_C body = info;
                    if(m_fileTool->removeDb(TB_GROUP_VERSION_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Succeeded in remove file from group file!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Remove file from group file failed!";
                    }
                }
                    break;
                case BdRequestSetDefaultGroupFileVersion_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("Name"))
                    {
                        ackJsonObj["code"] = BdGroupFileInfoParseError_E;
                        ackJsonObj["msg"] ="Remove group file parsing error!";
                        goto RESPONSE_END;
                    }

                    QString productName = dataObj["ProductName"].toString();
                    QString name = dataObj["Name"].toString();

                    QList<BODY_GROUP_VERSION_INFO_S> infos;
                    GS_DB_BODY_C bodys = infos;
                    QString queryStr = QString("product_name = \"%1\" AND name = \"%2\"").arg(productName).arg(name);
                    if(!m_fileTool->queryDb(TB_GROUP_VERSION_INFO_E, bodys, queryStr))
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Group version do not exists!";
                        goto RESPONSE_END;
                    }

                    infos = boost::any_cast<QList<BODY_GROUP_VERSION_INFO_S>>(bodys);

                    QList<BODY_GROUP_VERSION_INFO_S> clearInfos;
                    bodys = clearInfos;
                    queryStr = QString("product_name = \"%1\" AND is_default_version = TRUE").arg(productName);
                    if(m_fileTool->queryDb(TB_GROUP_VERSION_INFO_E, bodys, queryStr))
                    {
                        clearInfos = boost::any_cast<QList<BODY_GROUP_VERSION_INFO_S>>(bodys);
                        foreach (BODY_GROUP_VERSION_INFO_S info, clearInfos) {
                            info.isDefaultVersion = false;
                            GS_DB_BODY_C body = info;
                            m_fileTool->updateDb(TB_GROUP_VERSION_INFO_E, body);
                        }
                    }

                    foreach (BODY_GROUP_VERSION_INFO_S info, infos) {
                        info.isDefaultVersion = true;
                        qDebug() << __FUNCTION__ << __LINE__ << info.name << info.subFileName;
                        GS_DB_BODY_C body = info;
                        m_fileTool->updateDb(TB_GROUP_VERSION_INFO_E, body);
                    }
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Succeeded in set default version of group file!";
                }
                    break;
                case BdRequestAddProductInfo_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("ProductTypeId") || !dataObj.contains("Date")
                            || !dataObj.contains("ProductDesp") || !dataObj.contains("SaveFilePath"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    QString productName = dataObj["ProductName"].toString();
                    QString saveFilePath = getProductSavePath(productName);
                    BODY_PRODUCT_INFO_S info;
                    info.name = productName;
                    info.desp = dataObj["ProductDesp"].toString();
                    info.addDate = dataObj["Date"].toString();
                    info.modifyDate = dataObj["Date"].toString();
                    info.savePath = saveFilePath;
                    GS_DB_BODY_C body = info;
                    if(m_fileTool->queryDb(TB_PRODUCTS_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Product already exists!";
                        goto RESPONSE_END;
                    }


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
                        BODY_PRODUCT_INFO_S info;
                        info.name = productName;
                        info.desp = dataObj["ProductDesp"].toString();
                        info.addDate = dataObj["Date"].toString();
                        info.modifyDate = dataObj["Date"].toString();
                        info.savePath = saveFilePath;
                        GS_DB_BODY_C body = info;
                        if(m_fileTool->addDb(TB_PRODUCTS_INFO_E, body))
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
                    if(!newProductObj.contains("ProductName"))
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    QString saveFilePath = newProductObj["SaveFilePath"].toString();
                    QDir dir;
                    dir.setPath(saveFilePath);
                    bool res = dir.removeRecursively();
                    if(!res)
                    {
                        ackJsonObj["code"] = BdDeleteProductInfoFailed_E;
                        ackJsonObj["msg"] = "Remove product save path failed!";
                        goto RESPONSE_END;
                    }

                    if(m_fileTool->removeProductDb(newProductObj["ProductName"].toString()))
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
                    QJsonDocument productDoc;
                    if(m_fileTool->openProductDb(productName,productDoc))
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
                        if(!m_fileTool->addAuthUser(user))
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
                        if(!m_fileTool->removeAuthUser(user))
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
                        if(!m_fileTool->editAuthUser(user))
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
                    if(m_fileTool->openAuthUsersDb(userDoc))
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

                    BODY_PRODUCT_INFO_S info1;
                    info1.name = productName;
                    GS_DB_BODY_C body = info1;

                    if(!m_fileTool->queryDb(TB_PRODUCTS_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }

                    info1 = boost::any_cast<BODY_PRODUCT_INFO_S>(body);

                    QString fileType = dataObj["FileType"].toString();
                    QString fileSubType = dataObj["FileSubType"].toString();
                    QString versionName =  dataObj["VersionName"].toString();

                    if(info1.savePath.isEmpty() || info1.savePath == "")
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }
                    bool finded = false;
                    bool firstFinded = false;
                    if(fileType == "CRC")
                    {
                        QString custom;
                        if(dataObj.contains("Custom"))
                            custom = dataObj["Custom"].toString();
                        else if(productName == "ZH510")
                            custom = "BD";
                        else
                            custom = "STD";
                        BODY_CRC_VERSION_INFO_S info2;
                        info2.productName = productName;
                        info2.name = versionName;
                        body = info2;
                        if(m_fileTool->queryDb(TB_CRC_VERSION_INFO_E, body))
                        {
                            info2 = boost::any_cast<BODY_CRC_VERSION_INFO_S>(body);
                            if(custom == info2.custom)
                            {
                                finded = true;
                                custom = info2.custom;
                                QJsonObject dataObj;
                                QJsonObject crcObj;
                                m_fileTool->putInfo2Json(TB_CRC_VERSION_INFO_E, body, crcObj);
                                dataObj["SaveFilePath"] = info1.savePath;
                                dataObj["VersionInfo"] = crcObj;
                                dataObj["Custom"] = custom;
                                ackJsonObj["data"] = dataObj;
                            }
                        }
                        else //全词匹配方式未找到，使用模糊查找
                        {
                            QList<BODY_CRC_VERSION_INFO_S> infos;
                            body = infos;
                            QString queryStr = QString("product_name = \"%1\" AND name LIKE \"%%2%\"").arg(productName).arg(versionName);
                            if(m_fileTool->queryDb(TB_CRC_VERSION_INFO_E, body, queryStr))
                            {
                                infos = boost::any_cast<QList<BODY_CRC_VERSION_INFO_S>>(body);
                                QJsonObject crcObj;
                                QJsonArray verArray;

                                foreach (BODY_CRC_VERSION_INFO_S info3, infos) {
                                    if(custom == info3.custom)
                                    {
                                        finded = true;
                                        custom = info3.custom;
                                    }
                                    if(finded && firstFinded == false)
                                    {
                                        body = info3;
                                        m_fileTool->putInfo2Json(TB_CRC_VERSION_INFO_E, body, crcObj);
                                        dataObj["VersionInfo"] = crcObj;
                                        firstFinded = true;
                                    }
                                    verArray << info3.name;
                                }
                                dataObj["SaveFilePath"] = info1.savePath;
                                dataObj["VersionInfos"] = verArray;
                                dataObj["Custom"] = custom;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                            }
                        }
                    }
                    else if(fileType == "MCU")
                    {
                        BODY_MCU_VERSION_INFO_S info2;
                        info2.productName = productName;
                        info2.mcuTypeName = fileSubType;
                        info2.name = versionName;
                        body = info2;
                        if(m_fileTool->queryDb(TB_MCU_VERSION_INFO_E, body))
                        {
                            QJsonObject dataObj;
                            QJsonObject mcuObj;
                            m_fileTool->putInfo2Json(TB_MCU_VERSION_INFO_E, body, mcuObj);
                            dataObj["SaveFilePath"] = info1.savePath;
                            dataObj["VersionInfo"] = mcuObj;
                            ackJsonObj["data"] = dataObj;
                            finded = true;
                        }
                        else //全词匹配方式未找到，使用模糊查找
                        {
                            QList<BODY_MCU_VERSION_INFO_S> infos;
                            body = infos;
                            QString queryStr = QString("product_name = \"%1\" AND mcu_type_name = \"%2\" AND name LIKE \"%%3%\"").arg(productName).arg(fileSubType).arg(versionName);
                            if(m_fileTool->queryDb(TB_MCU_VERSION_INFO_E, body, queryStr))
                            {
                                infos = boost::any_cast<QList<BODY_MCU_VERSION_INFO_S>>(body);
                                QJsonObject mcuObj;
                                QJsonArray verArray;
                                foreach (BODY_MCU_VERSION_INFO_S info3, infos) {
                                    verArray << info3.name;
                                    if(firstFinded == false)
                                    {
                                        body = info3;
                                        m_fileTool->putInfo2Json(TB_MCU_VERSION_INFO_E, body, mcuObj);
                                        dataObj["VersionInfo"] = mcuObj;
                                        firstFinded = true;
                                    }
                                }
                                dataObj["SaveFilePath"] = info1.savePath;
                                dataObj["VersionInfos"] = verArray;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                            }
                        }
                    }
                    else if(fileType == "GROUP")
                    {
                        QList<BODY_GROUP_VERSION_INFO_S> infos2;
                        body = infos2;
                        QString queryStr = QString("product_name = \"%1\" AND name = \"%2\"").arg(productName).arg(versionName);
                        if(m_fileTool->queryDb(TB_GROUP_VERSION_INFO_E, body, queryStr))
                        {
                            QJsonObject infosObj;
                            m_fileTool->putInfos2Json(TB_GROUP_VERSION_INFO_E, body, infosObj);
                            if(infosObj.contains(versionName))
                            {
                                QJsonObject dataObj;
                                dataObj["SaveFilePath"] =info1.savePath;
                                dataObj["VersionInfo"] = infosObj[versionName].toObject();
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                            }
                        }
                        else //全词匹配方式未找到，使用模糊查找
                        {
                            QList<BODY_GROUP_VERSION_INFO_S> infos;
                            body = infos;
                            QString queryStr = QString("product_name = \"%1\" AND name LIKE \"%%3%\"").arg(productName).arg(versionName);
                            if(m_fileTool->queryDb(TB_GROUP_VERSION_INFO_E, body, queryStr))
                            {
                                infos = boost::any_cast<QList<BODY_GROUP_VERSION_INFO_S>>(body);
                                QJsonObject infosObj;
                                m_fileTool->putInfos2Json(TB_GROUP_VERSION_INFO_E, body, infosObj);
                                if(infosObj.size() > 0)
                                {
                                    dataObj["VersionInfo"] = infosObj[infosObj.keys().first()].toObject();
                                }
                                QJsonArray verArray;
                                foreach (QJsonValue value, infosObj) {
                                    QJsonObject obj = value.toObject();
                                    verArray << obj["Name"].toString();
                                }

                                dataObj["SaveFilePath"] = info1.savePath;
                                dataObj["VersionInfos"] = verArray;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
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
                    BODY_PRODUCT_INFO_S info1;
                    info1.name = productName;
                    GS_DB_BODY_C body = info1;

                    if(!m_fileTool->queryDb(TB_PRODUCTS_INFO_E, body))
                    {
                        ackJsonObj["code"] = BdProductDontExist_E;
                        ackJsonObj["msg"] ="The product does not exist!";
                        goto RESPONSE_END;
                    }


                    QString fileType = dataObj["FileType"].toString();
                    QString fileSubType = dataObj["FileSubType"].toString();
                    info1 = boost::any_cast<BODY_PRODUCT_INFO_S>(body);

                    if(info1.savePath.isEmpty() || info1.savePath == "")
                    {
                        ackJsonObj["code"] = BdProductInfoParseError_E;
                        ackJsonObj["msg"] ="Product information parsing error!";
                        goto RESPONSE_END;
                    }

                    bool finded = false;
                    if(fileType == "CRC")
                    {

                        QList<BODY_CRC_VERSION_INFO_S> infos2;
                        body = infos2;
                        QString queryStr = QString("product_name = \"%1\" AND is_default_version = TRUE").arg(productName);
                        if(m_fileTool->queryDb(TB_CRC_VERSION_INFO_E, body, queryStr))
                        {
                            infos2 = boost::any_cast<QList<BODY_CRC_VERSION_INFO_S>>(body);

                            if(productName == "ZH510" && !dataObj.contains("Custom")) //防止旧版ZH510 BD版本升级为标准版
                                dataObj["Custom"] = "BD";

                            if(dataObj.contains("Custom"))
                            {
                                QString custom = dataObj["Custom"].toString();
                                if(custom == "" || custom.isEmpty() || custom == "STD")
                                {
                                    foreach (BODY_CRC_VERSION_INFO_S info, infos2) {
                                        if(info.custom == "" || info.custom.isEmpty() || info.custom == "STD") //定制类型匹配
                                        {
                                            QJsonObject crcObj;
                                            body = info;
                                            m_fileTool->putInfo2Json(TB_CRC_VERSION_INFO_E, body, crcObj);
                                            QJsonObject dataObj;
                                            dataObj["SaveFilePath"] = info1.savePath;
                                            dataObj["VersionInfo"] = crcObj;
                                            dataObj["Custom"] = "STD";
                                            ackJsonObj["data"] = dataObj;
                                            finded = true;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    foreach (BODY_CRC_VERSION_INFO_S info, infos2) {
                                        if(info.custom == custom) //找到指定版本
                                        {
                                            QJsonObject crcObj;
                                            body = info;
                                            m_fileTool->putInfo2Json(TB_CRC_VERSION_INFO_E, body, crcObj);
                                            QJsonObject dataObj;
                                            dataObj["SaveFilePath"] = info1.savePath;
                                            dataObj["VersionInfo"] = crcObj;
                                            dataObj["Custom"] = custom;
                                            ackJsonObj["data"] = dataObj;
                                            finded = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            else //兼容旧版本
                            {
                                foreach (BODY_CRC_VERSION_INFO_S info, infos2) {
                                    if(info.custom == "" || info.custom.isEmpty() || info.custom == "STD") //定制类型匹配
                                    {
                                        QJsonObject crcObj;
                                        body = info;
                                        m_fileTool->putInfo2Json(TB_CRC_VERSION_INFO_E, body, crcObj);
                                        QJsonObject dataObj;
                                        dataObj["SaveFilePath"] = info1.savePath;
                                        dataObj["VersionInfo"] = crcObj;
                                        dataObj["Custom"] = "STD";
                                        ackJsonObj["data"] = dataObj;
                                        finded = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else if(fileType == "MCU")
                    {
                        QList<BODY_MCU_VERSION_INFO_S> infos2;
                        body = infos2;
                        QString queryStr = QString("product_name = \"%1\" AND is_default_version = TRUE").arg(productName);
                        if(m_fileTool->queryDb(TB_MCU_VERSION_INFO_E, body, queryStr))
                        {
                            infos2 = boost::any_cast<QList<BODY_MCU_VERSION_INFO_S>>(body);
                            foreach (BODY_MCU_VERSION_INFO_S info, infos2) {
                                body = info;
                                QJsonObject mcuObj;
                                m_fileTool->putInfo2Json(TB_MCU_VERSION_INFO_E, body, mcuObj);
                                dataObj["SaveFilePath"] = info1.savePath;
                                dataObj["VersionInfo"] = mcuObj;
                                ackJsonObj["data"] = dataObj;
                                finded = true;
                                break;
                            }
                        }
                    }
                    else if(fileType == "GROUP")
                    {
                        QList<BODY_GROUP_VERSION_INFO_S> infos2;
                        body = infos2;
                        QString queryStr = QString("product_name = \"%1\" AND is_default_version = TRUE").arg(productName);
                        if(m_fileTool->queryDb(TB_GROUP_VERSION_INFO_E, body, queryStr))
                        {
                            QJsonObject infosObj;
                            m_fileTool->putInfos2Json(TB_GROUP_VERSION_INFO_E, body, infosObj);
                            if(infosObj.size() > 0)
                            {
                                QJsonObject dataObj;
                                dataObj["SaveFilePath"] =info1.savePath;
                                dataObj["VersionInfo"] = infosObj[infosObj.keys().first()].toObject();
                                ackJsonObj["data"] = dataObj;
                                finded = true;
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
                case BdRequestAddLog_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("DevNumber") || !dataObj.contains("ImeiNumber") || !dataObj.contains("FwVersion")
                            || !dataObj.contains("Type") || !dataObj.contains("Subtype")
                            || !dataObj.contains("Result") || !dataObj.contains("DateTime") || !dataObj.contains("Message"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }


                    BODY_LOG_INFO_S info;
                    info.productName = dataObj["ProductName"].toString();
                    info.devNumber = dataObj["DevNumber"].toString();
                    info.imeiNumber = dataObj["ImeiNumber"].toString();
                    info.fwVersion = dataObj["FwVersion"].toString();
                    info.type = (GS_LOG_TYPE)dataObj["Type"].toInt();
                    if(dataObj.contains("Subtype"))
                        info.subtype = dataObj["Subtype"].toInt();
                    else
                        info.subtype = 0;
                    info.result = (GS_LOG_RESULT)dataObj["Result"].toInt();
                    info.dateTime = QDateTime::fromString(dataObj["DateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                    info.serverDateTime = QDateTime::currentDateTime();
                    info.message = dataObj["Message"].toString();
                    if(m_fileTool->addLogDb(info))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add log successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Add log failed!";
                    }
                }
                    break;
                case BdRequestQueryLog_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("DevNumber") || !dataObj.contains("ImeiNumber") || !dataObj.contains("FwVersion")
                            || !dataObj.contains("Type")
                            || !dataObj.contains("Result") || !dataObj.contains("StartDateTime")
                            || !dataObj.contains("FuzzySearch") || !dataObj.contains("EndDateTime")
                            || !dataObj.contains("PageNo") || !dataObj.contains("PageSize"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }

                    BD_REQUEST_LOG_S info;
                    info.productName = dataObj["ProductName"].toString();
                    info.devNumber = dataObj["DevNumber"].toString();
                    info.imeiNumber = dataObj["ImeiNumber"].toString();
                    info.fwVersion = dataObj["FwVersion"].toString();
                    info.type = (GS_LOG_TYPE)dataObj["Type"].toInt();
                    if(dataObj.contains("Subtype"))
                        info.subtype = dataObj["Subtype"].toInt();
                    else
                        info.subtype = 0;
                    info.result = (GS_LOG_RESULT)dataObj["Result"].toInt();
                    info.startDateTime = QDateTime::fromString(dataObj["StartDateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                    info.endDateTime = QDateTime::fromString(dataObj["EndDateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                    info.fuzzySearch = dataObj["FuzzySearch"].toBool();
                    info.pageNo = dataObj["PageNo"].toInt();
                    info.pageSize = dataObj["PageSize"].toInt();

                    QString queryStr;
                    QString matchCondition1;
                    QString specifyType;
                    QString specifySubType;
                    QString specifyResult;
                    QString specifyDevNumber;
                    QString specifyImeiNumber;
                    QString specifyCurrentVersion;
                    QString specifyDateTime;
                    QString specifyServerDateTime;
                    int subtypeMax;
                    int resultMax;
                    if(info.type == LOG_UPGRADE_E)
                        subtypeMax = UPGRADE_TYPE_MAX;
                    else if(info.type == LOG_DEV_MODULE_E)
                        subtypeMax = DEV_MODULE_TYPE_MAX;
                    else
                        subtypeMax = 0;
                    if(info.type == LOG_DEV_MODULE_E)
                        resultMax = DEV_MODULE_STATUS_MAX;
                    else
                        resultMax = LOG_RESULT_MAX;
                    if(info.type < LOG_TYPE_MAX)
                        specifyType = QString(" type=%1 ").arg(info.type);
                    if(info.subtype < subtypeMax)
                        specifySubType = QString(" subtype=%1 ").arg(info.subtype);
                    if(info.result < resultMax)
                        specifyResult = QString(" result=%1 ").arg(info.result);

                    if(info.devNumber.size() > 0)
                    {
                        if(info.fuzzySearch)
                            specifyDevNumber = QString(" dev_number LIKE \"%%1%\" ").arg(info.devNumber);
                        else
                            specifyDevNumber = QString(" dev_number = \"%1\" ").arg(info.devNumber);
                    }

                    if(info.imeiNumber.size() > 0)
                    {
                        if(info.fuzzySearch)
                            specifyImeiNumber =  QString(" imei_number LIKE \"%%1%\" ").arg(info.imeiNumber);
                        else
                            specifyImeiNumber = QString(" imei_number = \"%1\" ").arg(info.imeiNumber);
                    }

                    if(info.fwVersion.size() > 0)
                    {
                        if(info.fuzzySearch)
                            specifyCurrentVersion =  QString(" fw_version LIKE \"%%1%\" ").arg(info.fwVersion);
                        else
                            specifyCurrentVersion = QString(" fw_version = \"%1\" ").arg(info.fwVersion);
                    }
                    if(info.productName == "ALL")
                    {
                        specifyDateTime = QString(" date_time >= '%1' AND date_time <= '%2' ")
                                .arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                        specifyServerDateTime = QString(" server_date_time >= '%1' AND server_date_time <= '%2' ")
                                .arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                    }
                    else
                    {
                        specifyDateTime = QString(" product_name = \"%1\" AND date_time >= '%2' AND date_time <= '%3' ")
                                .arg(info.productName).arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                        specifyServerDateTime = QString(" product_name = \"%1\" AND server_date_time >= '%2' AND server_date_time <= '%3' ")
                                .arg(info.productName).arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                    }

                    if(specifyType.size() > 0)
                        matchCondition1 += specifyType;
                    if(specifyImeiNumber.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyImeiNumber;
                        else
                            matchCondition1 += specifyImeiNumber;
                    }
                    if(specifyCurrentVersion.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyCurrentVersion;
                        else
                            matchCondition1 += specifyCurrentVersion;
                    }
                    if(specifySubType.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifySubType;
                        else
                            matchCondition1 += specifySubType;
                    }
                    if(specifyResult.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyResult;
                        else
                            matchCondition1 += specifyResult;
                    }

                    if(specifyDevNumber.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyDevNumber;
                        else
                            matchCondition1 += specifyDevNumber;
                    }

                    if(matchCondition1.size() > 0)
                        queryStr = matchCondition1 + "AND" + specifyDateTime;
                    else
                        queryStr = specifyDateTime;
                    qDebug() << __FUNCTION__ <<queryStr;
                    QJsonObject ackObj;
                    if(info.pageNo == 1)
                    {
                        int count = m_fileTool->queryLogMatchCountDb(info.devNumber, queryStr);
                        if(count > 0)
                        {
                            ackObj["TotalCount"] = count;
                        }
                        if(count <= 0)
                        {
                            qWarning() << "If the time of the matching device is not queried, change the server time to match!";
                            if(matchCondition1.size() > 0)
                                queryStr = matchCondition1 + "AND" + specifyServerDateTime;
                            else
                                queryStr = specifyServerDateTime;
                            qDebug() << __FUNCTION__ <<queryStr;
                            count = m_fileTool->queryLogMatchCountDb(info.devNumber, queryStr);
                            if(count > 0)
                            {
                                ackObj["TotalCount"] = count;
                            }
                        }
                        if(count <= 0)
                        {
                            ackJsonObj["code"] = BdExecutionFailed_E;
                            ackJsonObj["msg"] = "No data found!";
                            goto RESPONSE_END;
                        }
                    }



                    QList<BODY_LOG_INFO_S> infos;
                    GS_DB_BODY_C body = infos;
                    res = m_fileTool->queryLogDb(info.devNumber, body, queryStr, (info.pageNo-1) * info.pageSize, info.pageSize);
                    if(!res)
                    {
                        qWarning() << "If the time of the matching device is not queried, change the server time to match!";
                        if(matchCondition1.size() > 0)
                            queryStr = matchCondition1 + "AND" + specifyServerDateTime;
                        else
                            queryStr = specifyServerDateTime;
                        qDebug() << __FUNCTION__ <<queryStr;
                    }
                    if(!m_fileTool->queryLogDb(info.devNumber, body, queryStr, (info.pageNo-1) * info.pageSize, info.pageSize))
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "No data found!";
                        goto RESPONSE_END;
                    }
                    QJsonArray infoArray;
                    infos = boost::any_cast<QList<BODY_LOG_INFO_S>>(body);
                    foreach (BODY_LOG_INFO_S info1, infos) {
                        QJsonObject infoObj;
                        infoObj["ProductName"] = info1.productName;
                        infoObj["DevNumber"] = info1.devNumber;
                        infoObj["ImeiNumber"] = info1.imeiNumber;
                        infoObj["FwVersion"] = info1.fwVersion;
                        infoObj["Type"] = info1.type;
                        infoObj["Subtype"] = info1.subtype;
                        infoObj["Result"] = info1.result;
                        infoObj["DateTime"] = info1.dateTime.toString("yyyy-MM-dd hh:mm:ss");
                        infoObj["ServerDateTime"] = info1.serverDateTime.toString("yyyy-MM-dd hh:mm:ss");
                        infoObj["Message"] = info1.message;
                        infoArray << infoObj;
                    }
                    ackObj["Logs"] = infoArray;
                    ackJsonObj["data"] = ackObj;
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Search log successfully!";
                }
                    break;
                case BdRequestSetTerminalInfo_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("DeviceNum") || !dataObj.contains("PhoneNumber") || !dataObj.contains("LicenseNum")
                            || !dataObj.contains("VideoId") || !dataObj.contains("TerminalId")
                            || !dataObj.contains("ImeiNumber") || !dataObj.contains("CrcVersion") || !dataObj.contains("McuVersion")
                            || !dataObj.contains("AuthorizationStatus") || !dataObj.contains("ServerInfos")
                            || !dataObj.contains("DateTime") || !dataObj.contains("ServerDateTime"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }

                    BODY_TMN_INFO_S info;
                    info.productName = dataObj["ProductName"].toString();
                    info.deviceNum = dataObj["DeviceNum"].toString();
                    info.phoneNumber = dataObj["PhoneNumber"].toString();
                    info.licenseNum = dataObj["LicenseNum"].toString();
                    info.videoId = dataObj["VideoId"].toString();
                    info.terminalId = dataObj["TerminalId"].toString();
                    info.imeiNumber = dataObj["ImeiNumber"].toString();
                    info.crcVersion = dataObj["CrcVersion"].toString();
                    info.mcuVersion = dataObj["McuVersion"].toString();
                    info.authorizationStatus = dataObj["AuthorizationStatus"].toInt();
                    QJsonArray serverInfos = dataObj["ServerInfos"].toArray();
                    foreach (QJsonValue val, serverInfos) {
                        QJsonObject serverInfoObj = val.toObject();
                        if(!serverInfoObj.contains("IpAddr") || !serverInfoObj.contains("Port"))
                            break;
                        BODY_SERVER_INFO_S serverInfo;
                        serverInfo.ipAddr = serverInfoObj["IpAddr"].toString();
                        serverInfo.port = serverInfoObj["Port"].toInt();
                        info.serverInfos << serverInfo;
                    }

                    info.dateTime = QDateTime::fromString(dataObj["DateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                    info.serverDateTime = QDateTime::currentDateTime();

                    if(m_fileTool->addTmnInfoLogDb(info))
                    {
                        ackJsonObj["code"] = BdExecutionSucceeded_E;
                        ackJsonObj["msg"] = "Add log successfully!";
                    }
                    else
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "Add log failed!";
                    }
                }
                    break;
                case BdRequestQueryTerminalInfoLog_E:
                {
                    QJsonObject dataObj = jsonObj["data"].toObject();
                    if(!dataObj.contains("ProductName") || !dataObj.contains("DevNumber") || !dataObj.contains("ImeiNumber") || !dataObj.contains("FwVersion")
                            || !dataObj.contains("Type")
                            || !dataObj.contains("Result") || !dataObj.contains("StartDateTime")
                            || !dataObj.contains("FuzzySearch") || !dataObj.contains("EndDateTime")
                            || !dataObj.contains("PageNo") || !dataObj.contains("PageSize"))
                    {
                        ackJsonObj["code"] = BdJsonParseError_E;
                        ackJsonObj["msg"] ="Json parse error!";
                        goto RESPONSE_END;
                    }

                    BD_REQUEST_LOG_S info;
                    info.productName = dataObj["ProductName"].toString();
                    info.devNumber = dataObj["DevNumber"].toString();
                    info.imeiNumber = dataObj["ImeiNumber"].toString();
                    info.fwVersion = dataObj["FwVersion"].toString();
                    info.type = (GS_LOG_TYPE)dataObj["Type"].toInt();
                    if(dataObj.contains("Subtype"))
                        info.subtype = dataObj["Subtype"].toInt();
                    else
                        info.subtype = 0;
                    info.result = (GS_LOG_RESULT)dataObj["Result"].toInt();
                    info.startDateTime = QDateTime::fromString(dataObj["StartDateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                    info.endDateTime = QDateTime::fromString(dataObj["EndDateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                    info.fuzzySearch = dataObj["FuzzySearch"].toBool();
                    info.pageNo = dataObj["PageNo"].toInt();
                    info.pageSize = dataObj["PageSize"].toInt();

                    QString queryStr;
                    QString matchCondition1;
                    QString specifyType;
                    QString specifySubType;
                    QString specifyResult;
                    QString specifyDevNumber;
                    QString specifyImeiNumber;
                    QString specifyCurrentVersion;
                    QString specifyDateTime;
                    QString specifyServerDateTime;
                    int subtypeMax;
                    int resultMax;
                    if(info.type == LOG_UPGRADE_E)
                        subtypeMax = UPGRADE_TYPE_MAX;
                    else if(info.type == LOG_DEV_MODULE_E)
                        subtypeMax = DEV_MODULE_TYPE_MAX;
                    else
                        subtypeMax = 0;
                    if(info.type == LOG_DEV_MODULE_E)
                        resultMax = DEV_MODULE_STATUS_MAX;
                    else
                        resultMax = LOG_RESULT_MAX;
                    if(info.type < LOG_TYPE_MAX)
                        specifyType = QString(" type=%1 ").arg(info.type);
                    if(info.subtype < subtypeMax)
                        specifySubType = QString(" subtype=%1 ").arg(info.subtype);
                    if(info.result < resultMax)
                        specifyResult = QString(" result=%1 ").arg(info.result);

                    if(info.devNumber.size() > 0)
                    {
                        if(info.fuzzySearch)
                            specifyDevNumber = QString(" dev_number LIKE \"%%1%\" ").arg(info.devNumber);
                        else
                            specifyDevNumber = QString(" dev_number = \"%1\" ").arg(info.devNumber);
                    }

                    if(info.imeiNumber.size() > 0)
                    {
                        if(info.fuzzySearch)
                            specifyImeiNumber =  QString(" imei_number LIKE \"%%1%\" ").arg(info.imeiNumber);
                        else
                            specifyImeiNumber = QString(" imei_number = \"%1\" ").arg(info.imeiNumber);
                    }

                    if(info.fwVersion.size() > 0)
                    {
                        if(info.fuzzySearch)
                            specifyCurrentVersion =  QString(" fw_version LIKE \"%%1%\" ").arg(info.fwVersion);
                        else
                            specifyCurrentVersion = QString(" fw_version = \"%1\" ").arg(info.fwVersion);
                    }
                    if(info.productName == "ALL")
                    {
                        specifyDateTime = QString(" date_time >= '%1' AND date_time <= '%2' ")
                                .arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                        specifyServerDateTime = QString(" server_date_time >= '%1' AND server_date_time <= '%2' ")
                                .arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                    }
                    else
                    {
                        specifyDateTime = QString(" product_name = \"%1\" AND date_time >= '%2' AND date_time <= '%3' ")
                                .arg(info.productName).arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                        specifyServerDateTime = QString(" product_name = \"%1\" AND server_date_time >= '%2' AND server_date_time <= '%3' ")
                                .arg(info.productName).arg(dataObj["StartDateTime"].toString()).arg(dataObj["EndDateTime"].toString());
                    }

                    if(specifyType.size() > 0)
                        matchCondition1 += specifyType;
                    if(specifyImeiNumber.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyImeiNumber;
                        else
                            matchCondition1 += specifyImeiNumber;
                    }
                    if(specifyCurrentVersion.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyCurrentVersion;
                        else
                            matchCondition1 += specifyCurrentVersion;
                    }
                    if(specifySubType.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifySubType;
                        else
                            matchCondition1 += specifySubType;
                    }
                    if(specifyResult.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyResult;
                        else
                            matchCondition1 += specifyResult;
                    }

                    if(specifyDevNumber.size() > 0)
                    {
                        if(matchCondition1.size() > 0)
                            matchCondition1 += "AND" + specifyDevNumber;
                        else
                            matchCondition1 += specifyDevNumber;
                    }

                    if(matchCondition1.size() > 0)
                        queryStr = matchCondition1 + "AND" + specifyDateTime;
                    else
                        queryStr = specifyDateTime;
                    qDebug() << __FUNCTION__ <<queryStr;
                    QJsonObject ackObj;
                    if(info.pageNo == 1)
                    {
                        int count = m_fileTool->queryLogMatchCountDb(info.devNumber, queryStr);
                        if(count > 0)
                        {
                            ackObj["TotalCount"] = count;
                        }
                        if(count <= 0)
                        {
                            qWarning() << "If the time of the matching device is not queried, change the server time to match!";
                            if(matchCondition1.size() > 0)
                                queryStr = matchCondition1 + "AND" + specifyServerDateTime;
                            else
                                queryStr = specifyServerDateTime;
                            qDebug() << __FUNCTION__ <<queryStr;
                            count = m_fileTool->queryLogMatchCountDb(info.devNumber, queryStr);
                            if(count > 0)
                            {
                                ackObj["TotalCount"] = count;
                            }
                        }
                        if(count <= 0)
                        {
                            ackJsonObj["code"] = BdExecutionFailed_E;
                            ackJsonObj["msg"] = "No data found!";
                            goto RESPONSE_END;
                        }
                    }



                    QList<BODY_LOG_INFO_S> infos;
                    GS_DB_BODY_C body = infos;
                    res = m_fileTool->queryLogDb(info.devNumber, body, queryStr, (info.pageNo-1) * info.pageSize, info.pageSize);
                    if(!res)
                    {
                        qWarning() << "If the time of the matching device is not queried, change the server time to match!";
                        if(matchCondition1.size() > 0)
                            queryStr = matchCondition1 + "AND" + specifyServerDateTime;
                        else
                            queryStr = specifyServerDateTime;
                        qDebug() << __FUNCTION__ <<queryStr;
                    }
                    if(!m_fileTool->queryLogDb(info.devNumber, body, queryStr, (info.pageNo-1) * info.pageSize, info.pageSize))
                    {
                        ackJsonObj["code"] = BdExecutionFailed_E;
                        ackJsonObj["msg"] = "No data found!";
                        goto RESPONSE_END;
                    }
                    QJsonArray infoArray;
                    infos = boost::any_cast<QList<BODY_LOG_INFO_S>>(body);
                    foreach (BODY_LOG_INFO_S info1, infos) {
                        QJsonObject infoObj;
                        infoObj["ProductName"] = info1.productName;
                        infoObj["DevNumber"] = info1.devNumber;
                        infoObj["ImeiNumber"] = info1.imeiNumber;
                        infoObj["FwVersion"] = info1.fwVersion;
                        infoObj["Type"] = info1.type;
                        infoObj["Subtype"] = info1.subtype;
                        infoObj["Result"] = info1.result;
                        infoObj["DateTime"] = info1.dateTime.toString("yyyy-MM-dd hh:mm:ss");
                        infoObj["ServerDateTime"] = info1.serverDateTime.toString("yyyy-MM-dd hh:mm:ss");
                        infoObj["Message"] = info1.message;
                        infoArray << infoObj;
                    }
                    ackObj["Logs"] = infoArray;
                    ackJsonObj["data"] = ackObj;
                    ackJsonObj["code"] = BdExecutionSucceeded_E;
                    ackJsonObj["msg"] = "Search log successfully!";
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
        uint32_t fseek_pos = body.sent_bytes - body.currently_sent_bytes;
        if(fseek_pos < m_receivedBytes)
        {
            qDebug() << QString("receiveFileData   uuid:%1 re-upload   %2/%3").arg(m_uuid).arg(body.sent_bytes).arg(body.file_size);
            m_uploadFileHandle->seek(fseek_pos);
            m_receivedBytes = fseek_pos;
        }
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
        if(body.received_bytes != m_sentBytes && body.request_receive_bytes > 0)
        {
            qDebug() << QString("sendFileData   uuid:%1 re-download   %2/%3").arg(m_uuid).arg(body.received_bytes).arg(body.file_size);
            m_downloadFileHandle->seek(body.received_bytes);
            m_sentBytes = body.received_bytes;
        }
        if(m_sentBytes == body.received_bytes && body.request_receive_bytes > 0)
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
