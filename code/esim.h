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

bool esimInit();
bool esimGetEID(char* eid, size_t bufferSize);
int esimGetProfiles(ESimProfile* profiles, int maxProfiles);
bool esimEnableProfile(const char* iccidOrAid);//启用esim配置文件
bool esimDisableProfile(const char* iccidOrAid);//禁用esim配置文件
bool esimDeleteProfile(const char* iccidOrAid);//删除esim配置文件
bool esimSwitchProfile(const char* iccidOrAid);//切换esim配置文件
bool esimGetNotificationCount(int* count);//获取esim通知数量
bool esimRetrieveNotifications(String& output);//获取esim通知
bool esimClearAllNotifications(int* clearedCount);//清理esim通知
bool esimProcessNotifications(String& outputLog);//一键上报并释放esim
const char* esimGetLastError();

bool handleSerialConsole();
bool handleESimSerialCommand(const String& command);

#endif
