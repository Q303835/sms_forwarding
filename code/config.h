#ifndef CONFIG_H
#define CONFIG_H

#include "globals.h"

void saveConfig();//将当前配置保存到 NVS Flash 闪存
void loadConfig();//从NVS Flash闪存加载配置
bool isPushChannelValid(const PushChannel& ch);//检查推送通道配置是否有效
bool isConfigValid();//检查配置是否有效（主要检查Web登录凭据和至少一个推送通道是否启用）
String getDeviceUrl();//获取设备访问地址

#endif
