#include "config.h"
#include "web_handlers.h"

// 保存配置到NVS
void saveConfig() {
  preferences.begin("sms_config", false);
  preferences.putString("smtpServer", config.smtpServer);
  preferences.putInt("smtpPort", config.smtpPort);
  preferences.putString("smtpUser", config.smtpUser);
  preferences.putString("smtpPass", config.smtpPass);
  preferences.putString("smtpSendTo", config.smtpSendTo);
  preferences.putString("adminPhone", config.adminPhone);
  preferences.putString("webUser", config.webUser);
  preferences.putString("webPass", config.webPass);
  preferences.putString("numBlkList", config.numberBlackList);
  preferences.putString("wifiSsid", config.wifiSsid);
  preferences.putString("wifiPass", config.wifiPass);
  preferences.putInt("esimProxyMode", config.esimProxyMode);
  preferences.putString("esimProxyUrl", config.esimProxyUrl);
  preferences.putString("esimLpacTarget", config.esimLpacTarget);
  // 保存自动保号配置 (键名严格限制在15字符以内)
  preferences.putBool("as_en", config.autoSms.enabled);
  preferences.putUInt("as_time", config.autoSms.lastSentTime);
  preferences.putUShort("as_days", config.autoSms.intervalDays);
  
  // 因为你在结构体里定义的是 char 数组，保存时要转成 String
  preferences.putString("as_tgt", String(config.autoSms.targetNumber));
  preferences.putString("as_msg", String(config.autoSms.message));
  // 保存推送通道配置
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String prefix = "push" + String(i);
    preferences.putBool((prefix + "en").c_str(), config.pushChannels[i].enabled);
    preferences.putUChar((prefix + "type").c_str(), (uint8_t)config.pushChannels[i].type);
    preferences.putString((prefix + "url").c_str(), config.pushChannels[i].url);
    preferences.putString((prefix + "name").c_str(), config.pushChannels[i].name);
    preferences.putString((prefix + "k1").c_str(), config.pushChannels[i].key1);
    preferences.putString((prefix + "k2").c_str(), config.pushChannels[i].key2);
    preferences.putString((prefix + "body").c_str(), config.pushChannels[i].customBody);
  }
  
  preferences.end();
  logCaptureLn(String("配置已保存"));
}

// 从NVS加载配置
void loadConfig() {
  preferences.begin("sms_config", true);
  config.smtpServer = preferences.getString("smtpServer", "");
  config.smtpPort = preferences.getInt("smtpPort", 465);
  config.smtpUser = preferences.getString("smtpUser", "");
  config.smtpPass = preferences.getString("smtpPass", "");
  config.smtpSendTo = preferences.getString("smtpSendTo", "");
  config.adminPhone = preferences.getString("adminPhone", "");
  config.webUser = preferences.getString("webUser", DEFAULT_WEB_USER);
  config.webPass = preferences.getString("webPass", DEFAULT_WEB_PASS);
  config.numberBlackList = preferences.getString("numBlkList", "");
  config.wifiSsid = preferences.getString("wifiSsid", ""); // 默认读宏定义
  config.wifiPass = preferences.getString("wifiPass", "");
  config.esimProxyMode = preferences.getInt("esimProxyMode", 1); 
  config.esimProxyUrl = preferences.getString("esimProxyUrl", "");
  config.esimLpacTarget = preferences.getString("esimLpacTarget", "");
  // 加载推送通道配置
  // 读取自动保号配置，逗号后面的是默认值（如果找不到数据就用默认值）
    config.autoSms.enabled = preferences.getBool("as_en", false);
    config.autoSms.lastSentTime = preferences.getUInt("as_time", 0);
    config.autoSms.intervalDays = preferences.getUShort("as_days", 80);
    
    // 读取字符串并安全复制回你的 char 数组中
    String target = preferences.getString("as_tgt", "");
    strncpy(config.autoSms.targetNumber, target.c_str(), sizeof(config.autoSms.targetNumber) - 1);
    
    String msg = preferences.getString("as_msg", "Keepalive");
    strncpy(config.autoSms.message, msg.c_str(), sizeof(config.autoSms.message) - 1);
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String prefix = "push" + String(i);
    config.pushChannels[i].enabled = preferences.getBool((prefix + "en").c_str(), false);
    config.pushChannels[i].type = (PushType)preferences.getUChar((prefix + "type").c_str(), PUSH_TYPE_POST_JSON);
    config.pushChannels[i].url = preferences.getString((prefix + "url").c_str(), "");
    config.pushChannels[i].name = preferences.getString((prefix + "name").c_str(), "通道" + String(i + 1));
    config.pushChannels[i].key1 = preferences.getString((prefix + "k1").c_str(), "");
    config.pushChannels[i].key2 = preferences.getString((prefix + "k2").c_str(), "");
    config.pushChannels[i].customBody = preferences.getString((prefix + "body").c_str(), "");
    if (config.pushChannels[i].type < PUSH_TYPE_POST_JSON ||
        config.pushChannels[i].type > PUSH_TYPE_TELEGRAM) {
      config.pushChannels[i].type = PUSH_TYPE_POST_JSON;
    }
  }
  
  // 兼容旧配置：如果有旧的httpUrl配置，迁移到第一个通道
  String oldHttpUrl = preferences.getString("httpUrl", "");
  if (oldHttpUrl.length() > 0 && !config.pushChannels[0].enabled) {
    config.pushChannels[0].enabled = true;
    config.pushChannels[0].url = oldHttpUrl;
    config.pushChannels[0].type = preferences.getUChar("barkMode", 0) != 0 ? PUSH_TYPE_BARK : PUSH_TYPE_POST_JSON;
    config.pushChannels[0].name = "迁移通道";
    logCaptureLn(String("已迁移旧HTTP配置到推送通道1"));
  }
  
  preferences.end();
  logCaptureLn(String("配置已加载"));
}

// 检查推送通道是否有效配置
bool isPushChannelValid(const PushChannel& ch) {
  if (!ch.enabled) return false;
  
  switch (ch.type) {
    case PUSH_TYPE_POST_JSON:
    case PUSH_TYPE_BARK:
    case PUSH_TYPE_GET:
    case PUSH_TYPE_DINGTALK:
    case PUSH_TYPE_FEISHU:
    case PUSH_TYPE_CUSTOM:
      return ch.url.length() > 0;
    case PUSH_TYPE_PUSHPLUS:
    case PUSH_TYPE_SERVERCHAN:
      return ch.key1.length() > 0;  // 这两个主要靠key1（token/sendkey）
    case PUSH_TYPE_GOTIFY:
      return ch.url.length() > 0 && ch.key1.length() > 0;  // 需要URL和Token
    case PUSH_TYPE_TELEGRAM:
      return ch.key1.length() > 0 && ch.key2.length() > 0; // 需要Chat ID和Token
    default:
      return false;
  }
}

// 检查配置是否有效（至少配置了邮件或任一推送通道）
bool isConfigValid() {
  bool emailValid = config.smtpServer.length() > 0 && 
                    config.smtpUser.length() > 0 && 
                    config.smtpPass.length() > 0 && 
                    config.smtpSendTo.length() > 0;
  
  bool pushValid = false;
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    if (isPushChannelValid(config.pushChannels[i])) {
      pushValid = true;
      break;
    }
  }
  
  return emailValid || pushValid;
}

// 获取当前设备URL
String getDeviceUrl() {
  return "http://" + WiFi.localIP().toString() + "/";
}
