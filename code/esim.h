#ifndef ESIM_H
#define ESIM_H

#include "globals.h"

#define ESIM_MAX_ICCID_LEN 21
#define ESIM_MAX_ISDP_AID_LEN 33
#define ESIM_MAX_NAME_LEN 64

#ifndef ESIM_PROFILE_LOG
#define ESIM_PROFILE_LOG 0
#endif

struct ESimProfile {
  char iccid[ESIM_MAX_ICCID_LEN];
  char isdpAid[ESIM_MAX_ISDP_AID_LEN];
  char nickname[ESIM_MAX_NAME_LEN];
  char serviceProviderName[ESIM_MAX_NAME_LEN];
  char profileName[ESIM_MAX_NAME_LEN];
  int state;
  int profileClass;
};

bool esimInit(); //初始化eSIM模块，返回是否成功
bool esimGetEID(char* eid, size_t bufferSize);//获取eSIM EID
int esimGetProfiles(ESimProfile* profiles, int maxProfiles);//获取eSIM配置文件列表，返回实际数量，失败返回负数
bool esimEnableProfile(const char* iccidOrAid);//启用esim配置文件
bool esimDisableProfile(const char* iccidOrAid);//禁用esim配置文件
bool esimDeleteProfile(const char* iccidOrAid);//删除esim配置文件
bool esimSwitchProfile(const char* iccidOrAid);//切换esim配置文件
bool esimGetNotificationCount(int* count);//获取esim通知数量
bool esimRetrieveNotifications(String& output);//获取esim通知
bool esimClearAllNotifications(int* clearedCount);//清理esim通知
bool esimProcessNotifications(String& outputLog);//一键上报并释放esim
const char* esimGetLastError();//获取最近一次操作的错误信息

bool handleSerialConsole();//处理串口输入，返回是否有数据被处理
bool handleESimSerialCommand(const String& command);//处理串口命令
bool esimSetNickname(const char* iccidOrAid, const char* nickname);//设置esim昵称


#endif
