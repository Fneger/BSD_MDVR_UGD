#ifndef ST_GLOBAL_DEF_H
#define ST_GLOBAL_DEF_H

#include <QtCore>
#include <boost/any.hpp>

namespace GsGlobalDef {
typedef boost::any GS_DB_BODY_C;

//数据库表单信息
typedef enum{
    TB_USERS_INFO_E, //用户信息
    TB_PRODUCTS_INFO_E, //产品信息
    TB_CRC_VERSION_INFO_E,  //Crc版本信息
    TB_MCU_VERSION_INFO_E,  //Mcu版本信息
    TB_GROUP_VERSION_INFO_E,    //组合文件信息
    TB_LOG_INFO_E,  //日志信息
    TB_TMN_INFO_LOG_INFO_E,  //终端登录信息日志
    TB_TYPE_MAX,
}GS_BODY_TYPE;

//日志类型
typedef enum{
    LOG_UPGRADE_E,  //产品升级日志
    LOG_DEV_MODULE_E,   //设备模块状态
    LOG_TYPE_MAX    //所有日志类型
}GS_LOG_TYPE;

//升级类型
typedef enum{
    UPGRADE_CRC_E,  //CRC
    UPGRADE_MCU_E,  //MCU
    UPGRADE_GROUP_E,    //GROUP
    UPGRADE_TYPE_MAX
} GS_UPGRADE_TYPE;

//设备模块类型
typedef enum{
    DEV_MODULE_4G_E,   //4G模块
    DEV_MODULE_GPS_E,  //GPS
    DEV_MODULE_DISK_E, //磁盘
    DEV_MODULE_PWR_E,  //电源
    DEV_MODULE_TYPE_MAX
} GS_DEV_MODULE_TYPE;

//打印日志时对应程序执行结果
typedef enum{
    LOG_RESULT_SUCC_E,  //成功
    LOG_RESULT_IN_EXEC_E,   //执行中
    LOG_RESULT_FAILED_E,    //失败
    LOG_RESULT_MAX,     //所有返回结果
}GS_LOG_RESULT;

//设备模块状态
typedef enum{
    DEV_MODULE_NORMAL_E,    //正常
    DEV_MODULE_ABNORMAL_E,  //一样
    DEV_MODULE_STATUS_MAX
}GS_DEV_MODULE_STATUS;

//产品信息
typedef struct {
    int id;
    QString name; //产品名称
    QString desp;   //描述信息
    QString addDate;   //添加日期
    QString modifyDate; //修改日期
    QString savePath;  //文件储存路径
} BODY_PRODUCT_INFO_S;

//Crc版本信息
typedef struct {
    int id;
    QString productName; //产品名称
    QString name;   //版本名称
    QString date;   //添加日期
    bool isDefaultVersion; //是否默认版本
    QString message;    //版本说明
    QString fileName;   //文件所在全路径
    uint32_t fileSize;  //文件大小
    QString custom;     //定制信息 BD、BD_C1、BD_C2
}BODY_CRC_VERSION_INFO_S;

//Mcu版本信息
typedef struct {
    int id;
    QString productName; //产品名称
    QString mcuTypeName;    //mcu类型名称
    QString name;   //版本名称
    QString date;   //添加日期
    bool isDefaultVersion; //是否默认版本
    QString message;    //版本说明
    QString fileName;   //文件所在全路径
    uint32_t fileSize;  //文件大小
}BODY_MCU_VERSION_INFO_S;

//Group版本信息
typedef struct {
    int id;
    QString productName; //产品名称
    QString name;   //组合文件名称
    QString date;   //添加日期
    bool isDefaultVersion; //是否默认版本
    QString message;    //版本说明
    QString savePath;   //组合文件储存路径
    QString subFileName;    //组合文件子文件名称
    QString subFileDate; //组合文件子文件添加日期
    QString subFilePath;   //组合文件子文件所在全路径
    uint32_t subFileSize;    //组合文件子文件大小
    QString subFileMessage; //子文件备注信息
}BODY_GROUP_VERSION_INFO_S;

//日志信息
typedef struct {
    int id;
    QString productName; ////产品名称
    QString devNumber;  //设备号
    QString imeiNumber; //4G模块 IMEI
    QString fwVersion;    //固件版本
    GS_LOG_TYPE type;   //日志类型
    int subtype;   //日志子类型
    GS_LOG_RESULT result; //执行结果
    QDateTime dateTime;   //日志产生时间
    QDateTime serverDateTime;   //服务器本地时间
    QString message;    //日志内容
} BODY_LOG_INFO_S;

//日志表单管理
typedef struct {
    QString tableName;   //数据表名称
    bool isCreated;     //是否已创建
    int startDevNumber; //开始设备号
    int endDevNumber;   //结束设备号
    bool isFull;    //是否已满
    int savedItemNum; //已保存行数
}LOG_TABLE_MAN_S;

//服务器地址信息
typedef struct{
    QString ipAddr;
    int port;
} BODY_SERVER_INFO_S;


//设置终端信息
typedef struct{
    int id;
    QString productName;   //产品名称
    QString deviceNum;		/*机号(范围:000000~999999*/
    QString phoneNumber;   /*手机号*/
    QString licenseNum;    /*车牌号码*/
    QString videoId;       /*视频ID*/
    QString terminalId;    //终端ID
    QString imeiNumber; //4G模块 IMEI
    QString crcVersion;    //crc版本号
    QString mcuVersion;    //mcu版本号
    int authorizationStatus; //授权状态，0：未授权，1：已授权，2：授权失败
    QList<BODY_SERVER_INFO_S> serverInfos; //连接平台信息
    QDateTime dateTime;   //日志产生时间
    QDateTime serverDateTime;   //服务器本地时间
}BODY_TMN_INFO_S;


extern const QString S_USERS_INFO_TABLE_NAME;
extern const QString S_PRODUCTS_INFO_TABLE_NAME;
extern const QString S_CRC_VERSION_INFO_TABLE_NAME;
extern const QString S_MCU_VERSION_INFO_TABLE_NAME;
extern const QString S_GROUP_VERSION_INFO_TABLE_NAME;
extern const QString S_LOG_INFO_TABLE_NAME;
extern const QString S_TMN_INFO_LOG_INFO_TABLE_NAME;
extern const int S_QUERY_ITEM_NUM_MAX;
extern const int S_LOG_TABLE_DEV_MAX;
extern const int S_LOG_TABLE_ITEM_MAX;
extern const int S_LOG_TABLE_DEV_ID_MAX;

}



class st_global_def
{
public:
    st_global_def();
};

#endif // ST_GLOBAL_DEF_H
