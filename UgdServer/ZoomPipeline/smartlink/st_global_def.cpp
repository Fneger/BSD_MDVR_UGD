#include "st_global_def.h"

namespace GsGlobalDef {

const QString S_USERS_INFO_TABLE_NAME = "users_info_tbl";
const QString S_PRODUCTS_INFO_TABLE_NAME = "products_info_tbl";
const QString S_CRC_VERSION_INFO_TABLE_NAME = "crc_version_info_tbl";
const QString S_MCU_VERSION_INFO_TABLE_NAME = "mcu_version_info_tbl";
const QString S_GROUP_VERSION_INFO_TABLE_NAME = "group_version_info_tbl";
const QString S_LOG_INFO_TABLE_NAME = "log_info_tbl";

const int S_QUERY_ITEM_NUM_MAX = 1000;
const int S_LOG_TABLE_DEV_MAX = 50; //单个表单存储50个设备的日志
const int S_LOG_TABLE_ITEM_MAX = 5000000; //单个表单最大存储条数
const int S_LOG_TABLE_DEV_ID_MAX = 1000000;//设备号最大值

}

st_global_def::st_global_def()
{

}
