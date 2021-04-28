#ifndef ST_MESSAGE_H
#define ST_MESSAGE_H
#include <qglobal.h>
#include <QString>
#include <QStringList>
#include <QMap>

#define DATA_BUFF_LEN_MAX 4*1024*1024
#define DATA_PAYLOAD 40*1024
using namespace std;
namespace BdUgdServer {

    //远程版本管理协议由英国客户八代空中协议衍生而来
    //数据包封包格式可参考该协议
    typedef enum BD_PACKET_FUNC_ID_E{
        /************************ active report ************************/
        BdTerminalResponse_E = 0x0001,        //Terminal universal response
        BdPlatformResponse_E = 0x8001,        //Platform  universal response
        BdTerminalRptsHeartbeat_E = 0x0002,   //Terminal  heartbeat report
        BdTerminalAnswerDFI_E = 0x00EE,       //The  terminal answer downstream format is incorrect
        BdTerminalLogin_E = 0x0102,           //Terminal login
                                            //Every time the GPRS off the line, send instruction
                                            //first and wait for the answer and then send other data.
        BdTerminalLoginResponse_E = 0x8102,   //Terminal Login Response (downlink)

        BdRemoteUpgrade_E = 0x8500,           //远程升级（downlink）
        BdTerminalJson_E = 0x0003,              //终端发送到服务器Json
        BdTerminalUploadFileData_E = 0x0004,  //终端上传文件数据
        BdTerminalDownloadFileData_E = 0x0005,  //终端下载文件数据
        BdTerminalClientTest_E = 0x0006,        //终端客户端测试数据包
        BdTerminalStartClientTest_E = 0x0007,   //终端开始测试

        BdTerminalAckJson_E = 0x0006,         //终端反馈到服务器Json
        BdPlatformAckJson_E = 0x8301,           //服务器反馈到终端Json
        BdPlatformJson_E = 0x8304,              //服务器发送到终端Json
        BdPlarformSendFileData_E = 0x8305,     //服务器发送数据
    }BD_PACKET_FUNC_ID_E;

#pragma  pack (push,1)

//终端响应服务器反馈信息(5 Bytes)
typedef struct BD_TMN_RESPONSE_S{
    uint16_t  downlink_msn; //Corresponding to the downlink message serial number
    uint16_t downlink_fid;//the downlink setting function ID
    //downlink setting returns result
    //0x00 setting successful,0x01 Setting failed,0XFF wrong format
    uint8_t downlink_res;
}BD_TMN_RESPONSE_S;

//服务器响应终端反馈信息(5 Bytes)
typedef struct BD_PLATFORM_RESPONSE_S{
    uint16_t  uplink_msn; //Corresponding to the uplink message serial number
    uint16_t uplink_fid;//the uplink setting function ID
    //uplink setting returns result
    //0x00 setting successful,0x01 Setting failed
    uint8_t uplink_res;
}BD_PLATFORM_RESPONSE_S;

//登录信息(103 Bytes)
typedef struct BD_TMN_LOGIN_INFO_S{
    uint8_t tsft_version[14]; //终端软件版本号
    uint8_t stf_date[10];   //终端软件版本发布日期
    uint8_t cpu_id[12]; //CPU ID 号
    uint8_t terminal_type;  //终端类型0xaa:PC端版本控制APP
    char user_name[33]; //用户名
    char password[33];   //密码
}BD_TMN_LOGIN_INFO_S;

//终端登录服务器反馈信息(12 Bytes)
typedef struct BD_TMN_LOGIN_RESPONSE_S{
    uint8_t platform_time[6];//服务器平台时间
    uint16_t models_id;//型号编号
    uint16_t displacement;//Unit ml
    uint8_t needUpgrade;//0x55终端软件需要升级，其他值则不需要
    uint8_t auth;       //0xaa:登录成功，其他登录失败
}BD_TMN_LOGIN_RESPONSE_S;

//传输文件数据字段定义(4112 Bytes)
typedef struct BD_TRANSFER_FILE_DATA{
    uint32_t currently_sent_bytes; //当前发送字节
    uint32_t sent_bytes;     //已发送字节
    uint32_t not_sent_bytes; //未发送字节
    uint32_t file_size;      //当前传输文件大小
    uint8_t file_data[32768];       //文件数据,未填满填充0
}BD_TRANSFER_FILE_DATA_S;

//下载文件数据字段定义
typedef struct BD_TMN_DOWNLOAD_FILE_DATA{
    uint32_t request_receive_bytes; //请求接收字节
    uint32_t received_bytes;     //已接收字节
    uint32_t not_received_bytes; //未接收字节
    uint32_t file_size;      //当前传输文件大小
}BD_TMN_DOWNLOAD_FILE_DATA_S;

//终端测试字段定义
typedef struct BD_TMN_CLIENT_TEST{
    uint32_t currently_sent_bytes; //当前发送字节
    uint64_t total_sent_bytes;  //当前测试总发送字节
    uint8_t test_data[32768];    //测试数据
}BD_TMN_CLIENT_TEST_S;


typedef struct BD_MSG_HEADER{
    uint8_t start_code; //数据包开始0xE7
    uint16_t function_id; //数据包功能ID
    uint16_t message_attr; //消息属性，bit15：0,常规消息 bit0-9消息体长度；1,长消息主要用于文件上传下载，bit0-14消息体长度
    uint8_t device_id[7]; //BCD 码， 高字节在前， 低字节在后
    uint16_t massage_serial_num; //消息流水号
}BD_MSG_HEADER_S;

typedef struct BD_MSG_TAIL{
    uint8_t massage_xor;//从第一个消息标识后开始， 到校验前所有字节异或校验值,即：功能 ID、 消息属性、 设备 ID、 消息流水号、 数据包的异或值
    uint8_t end_code;   //数据包结束0xE7
}BD_MSG_TAIL_S;

typedef struct BD_MSG_HEARTBEATING{
    BD_MSG_HEADER_S msg_header;
    BD_MSG_TAIL_S msg_tail;
}BD_MSG_HEARTBEATING_S;

typedef enum tag_User_Auth{
    BdUserAuthGeneral_E, //普通用户
    BdUserAuthAdmin_E,  //管理员
}USER_AUTH_E;

//用户信息
typedef struct tag_User_Info{
    QString name;   //名称
    QString password; //密码
    int auth;   //权限 1:管理员，0：普通用户
    QString date;   //添加日期
    QString message; //信息
    QStringList products; //可以访问产品列表
}USER_INFO_S;

typedef struct tag_Version_Info{
    QString name;   //名称
    QString date;   //添加日期
    bool isDefault; //是否为默认版本
    QString message; //信息
    QString fileName;   //升级文件全路径
}VERSION_INFO_S;

typedef struct tag_Mcu_Info{
    QString mcuTypeName; //MCU类型名称
    quint64 mcuTypeId;    //MCU型号Id，为支持同一产品不同型号MCU
    QString mcuDefaultVersion;
    QList<VERSION_INFO_S> mcuVersionList; //Mcu版本列表
}MCU_INFO_S;
typedef struct tag_Product_Info{
    QString productName; //设备类型名称
    quint64 productTypeId;   //设备类型ID，由设备类型名称转BCD码所得
    QString addedDate;    //添加时间
    QString productDesp; //设备类型描述
    QString crcDafaultVersion;  //Crc文件默认版本
    QString saveFilePath;//服务器该产品管理文件存放路径
    QList<VERSION_INFO_S> crcVersionList; //Crc版本列表
    QMap<QString,MCU_INFO_S> mcuInfos;
    QMap<QString,MCU_INFO_S> fileInfos;
}PRODUCT_INFO_S;

//请求码定义,在Json中使用,只能向后添加
typedef enum BD_REQUEST_CODE {
    BdRequestSaveProductInfo_E, //请求保存产品信息
    BdRequestDeleteProductInfo_E, //删除产品
    BdRequestUploadFile_E,      //请求上传文件
    BdRequestDownloadFile_E,    //请求下载文件
    BdRequestQueryProductList_E, //请求查询产品列表
    BdRequestQueryProductInfo_E,     //请求查询指定产品型号信息
    BdRequestStopUploadFile_E,      //停止上传文件数据
    BdRequestStopDownloadFile_E,    //停止下载文件
    BdRequestAddMcuType_E,      //添加MCU新类型
    BdRequestRemoveMcuType_E,      //移除MCU类型
    BdRequestAddNewCrcVersion_E,        //添加Crc新版本
    BdRequestRemoveCrcVersion_E,  //移除Crc版本
    BdRequestSetDefaultCrcVersion_E,    //设置CRC默认版本
    BdRequestAddNewMcuVersion_E,        //添加Mcu新版本
    BdRequestRemoveMcuVersion_E,  //移除Mcu版本
    BdRequestSetDefaultMcuVersion_E,    //设置MCU默认版本
    BdRequestAddOtherFileVersion_E,      //添加其他文件版本
    BdRequestRemoveOtherFileVersion_E,      //移除其他文件版本
    BdRequestGetUserList_E,   //获取用户列表
    BdRequestAddUser_E,         //添加用户
    BdRequestRemoveUser_E,  //移除用户
    BdRequestEditUser_E,    //编辑用户信息
}BD_JSON_REQUEST_CODE_E;

//执行码定义
typedef enum BD_EXECUTION_CODE{
    BdExecutionSucceeded_E,     //请求执行成功
    BdJsonParseError_E,     //Json解析错误
    BdUserNotLogged_E,      //用户未登录
    BdOpenProductInfoFileFailed_E,  //打开产品信息文件失败
    BdSaveProductInfoFileFailed_E,  //保存产品信息文件失败
    BdDeleteProductInfoFailed_E,    //删除产品信息失败
    BdJsonFormatError_E,    //Json格式错误
    BdProductDontExist_E,   //产品不存在
    BdCreateFileFailed_E,       //创建文件失败
    BdUploadFileRequiredInfoError_E,    //上传文件所需信息错误
    BdDataTransmissionError_E,  //数据传输错误
    BdOpenFileFialed_E, //打开文件失败
    BdProductInfoParseError_E,     //产品信息解析错误
    BdPushHistoryJsonParseError_E, //软件加入历史版本Json解析错误
    BdMoveFileFailed_E,     //移动文件失败
    BdCrcHistoryPathParameterError_E,    //Crc历史版本文件存放路径参数错误
    BdAddNewMcuTypeJsonParseError_E,    //添加新MCU类型Json解析错误
    BdAddNewCrcVersionJsonParseError_E, //添加新版本CRC Json解析错误
    BdRemoveCrcVersionJsonParseError_E,//移除CRC版本 Json解析错误
    BdMcuTypeExists_E,              //MCU类型已存在
    BdMcuTypeInfoParseError_E,          //MCU类型信息解析错误
    BdMcuTypeDontExists_E,       //该MCU类型不存在
    BdMcuVersionInfoParseError_E,   //MCU版本信息解析错误
    BdMcuHistoryPathParameterError_E,    //Mcu历史版本文件存放路径参数错误
    BdCrcVersionDontExists_E,       //CRC版本不存在
    BdProductMcuInfoParseError_E,  //产品信息 MCU信息解析错误
    BdFileTypeDontExists_E,       //该FILE类型不存在
    BdAddUserFailed_E,  //添加用户失败
    BdRemoveUserFailed_E,  //添加用户失败
    BdEditUserFailed_E,  //编辑用户信息失败
    BdGetUserListFailed_E,  //获取用户列表失败
}BD_EXECUTION_CODE_E;

//请求结果Json字段定义
typedef struct BD_JSON_ACK{
    BD_EXECUTION_CODE_E code;  //执行码 0：成功，其他失败
    char massage[2048]; //相关提示信息
}BD_JSON_ACK_S;

//请求上传文件定义
typedef struct BD_REQUEST_UPLOAD_FILE{
    QString remoteFilePath; //远程路径
    QString remoteFileName; //远程文件名
    uint32_t fileSize;
}BD_REQUEST_UPLOAD_FILE_S;

//请求上传文件定义
typedef struct BD_REQUEST_DOWNLOAD_FILE{
    QString remoteFileFullPath; //远程文件全路径，包括文件名
}BD_REQUEST_DOWNLOAD_FILE_S;

//请求上传文件定义返回消息
typedef struct BD_REQUEST_DOWNLOAD_FILE_ACK{
    uint32_t fileSize;
}BD_REQUEST_DOWNLOAD_FILE_ACK_S;

//终止操作原因编码定义
typedef enum STOP_OPERATION_REASON{
    ReasonFileOpenFialed_E,    //文件打开失败

}STOP_OPERATION_REASON_E;

//停止上传文件字段定义
typedef struct BD_REQUEST_STOP_UPLOAD_FILE{
    STOP_OPERATION_REASON_E reason;
    QString msg;
}BD_REQUEST_STOP_UPLOAD_FILE_S;

//Crc文件加入历史版本
typedef struct BD_REQUEST_CRC_PUSH_HISTORY{
    QString productName;   //产品名称
    QString softwareVersion;
}BD_REQUEST_CRC_PUSH_HISTORY_S;

//Mcu Hex文件加入历史版本
typedef struct BD_REQUEST_MCU_PUSH_HISTORY{
    QString productName;   //产品名称
    QString mcuTypeName;    //mcu类型名称
    QString softwareVersion;
}BD_REQUEST_MCU_PUSH_HISTORY_S;

//添加新MCU类型
typedef struct BD_REQUEST_ADD_NEW_MCU{
    QString productName;   //产品名称
    MCU_INFO_S mcuInfo;
}BD_REQUEST_ADD_NEW_MCU_S;

//移除MCU类型
typedef struct BD_REQUEST_REMOVE_MCU{
    QString productName;   //产品名称
    QString mcuTypeName;    //mcu类型名称
}BD_REQUEST_REMOVE_MCU_S;

//添加CRC新版本
typedef struct BD_REQUEST_ADD_NEW_CRC_VERSION{
    QString productName;   //产品名称
    VERSION_INFO_S versionInfo; //版本信息
}BD_REQUEST_ADD_NEW_CRC_VERSION_S;

//添加MCU新版本
typedef struct  BD_REQUEST_ADD_NEW_MCU_VERSION{
    QString productName;   //产品名称
    QString mcuTypeName;    //mcu类型名称
    VERSION_INFO_S versionInfo; //版本信息
}BD_REQUEST_ADD_NEW_MCU_VERSION_S;

#pragma pack(pop)
}
namespace ExampleServer{



#pragma  pack (push,1)

#pragma pack(pop)
}
#endif // ST_MESSAGE_H
