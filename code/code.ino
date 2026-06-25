#include "globals.h"
#include "wifi_config.h"
#include "config.h"
#include "web_handlers.h"
#include "web_handlers.h"
#include "modem.h"
#include "web_handlers.h"
#include "push.h"
#include "web_handlers.h"
#include "sms_process.h"
#include "web_handlers.h"
#include "esim.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  // 缩短初始化延时，WiFi连接会处理自己的超时
  delay(200);
  Serial1.begin(115200, SERIAL_8N1, RXD, TXD);
  Serial1.setRxBufferSize(SERIAL_BUFFER_SIZE);
  while (Serial1.available()) Serial1.read();
  modemPowerCycle();
  while (Serial1.available()) Serial1.read();
  initConcatBuffer();
  loadConfig();
  configValid = isConfigValid();

// ---- WiFi 连接优化与智能配网 ----
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // 关闭 Modem Sleep，提高连接响应速度
  WiFi.setAutoReconnect(true); // 断线后自动重连
  WiFi.setScanMethod(WIFI_FAST_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);

  // 1. 优先读取你的 config 结构体里保存的 WiFi（如果你的配置类已经实现读取）
  // 如果你的 config 还没有这两个变量，可以用系统的 preferences 或者先用默认宏定义测试
  String targetSsid = config.wifiSsid; 
  String targetPass = config.wifiPass; 
  if (targetSsid.length() == 0) {
    targetSsid = WIFI_SSID;
    targetPass = WIFI_PASS;
  }

  WiFi.begin(targetSsid.c_str(), targetPass.c_str());
  logCaptureLn(String("正在连接wifi: ") + targetSsid);

  // 2. 带超时的等待连接
  unsigned long wifiStart = millis();
  const unsigned long WIFI_TIMEOUT = 15000; // 缩短到 15 秒超时，免得等太久
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_TIMEOUT) {
    blink_short(200);
  }

  // 3. 判定连接状态
  if (WiFi.status() == WL_CONNECTED) {
    logCaptureLn(String("wifi已连接"));
    logCapture(String("IP地址: "));
    logCaptureLn(WiFi.localIP().toString());
    logCapture(String("信号强度(RSSI): "));
    logCaptureLn(String(WiFi.RSSI()) + " dBm");
  } else {
    // 4. 【核心改动】连不上不重启！立刻变成 AP 热点，让手机能连上它进后台
    logCaptureLn(String("WiFi连接超时，正在启动临时配网热点..."));
    
    WiFi.mode(WIFI_AP);
    // 启动一个叫 ESP32C3_SETUP 的无密码热点
    WiFi.softAP("ESP32C3_SETUP"); 
    
    logCaptureLn(String("✓ 临时热点已启动！本机AP IP: ") + WiFi.softAPIP().toString());
    logCaptureLn(String("请用手机连接 WiFi [ESP32C3_SETUP]，然后访问 192.168.4.1 打开后台配网！"));
  }

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/tools", handleRoot);
  server.on("/sms", handleRoot);
  server.on("/sendsms", HTTP_POST, handleSendSms);
  server.on("/ping", HTTP_POST, handlePing);
  server.on("/query", handleQuery);
  server.on("/flight", handleFlightMode);
  server.on("/at", handleATCommand);
  server.on("/log", handleLog);
  server.on("/modem", handleModem);
  server.on("/wifi", handleWifi);
  server.on("/esim", handleESim);
  server.begin();
  logCaptureLn(String("HTTP服务器已启动"));

  // 1. 优先初始化 4G 模组，让它在后台开始注网、分配 IP
  logCaptureLn(String("正在初始化模组并等待网络稳定..."));
  modemInit(); 

  // ---- eSIM初始化 ----
  logCaptureLn(String("初始化eSIM..."));
  if (esimInit()) {
    logCaptureLn(String("eSIM初始化成功"));
    char eid[40];
    if (esimGetEID(eid, sizeof(eid))) {
      logCapture(String("EID: "));
      logCaptureLn(eid);
    }
  } else {
    logCaptureLn(String("eSIM初始化失败或未检测到eUICC芯片"));
  }
  logCaptureLn(String("正在从SIM卡自动获取本机号码..."));
String cnumResp = sendATCommand("AT+CNUM", 1500);
int firstQuote = cnumResp.indexOf("\",\"");
if (firstQuote != -1) {
  int secondQuote = cnumResp.indexOf("\"", firstQuote + 3);
  if (secondQuote != -1) {
    localPhoneNumber = cnumResp.substring(firstQuote + 3, secondQuote);
  }
}
localPhoneNumber.replace("NonNull", "");
localPhoneNumber.trim();
if (localPhoneNumber.length() > 0 && localPhoneNumber != "未知号码") {
  logCaptureLn(String("✓ 成功获取并固化本机号码: ") + localPhoneNumber);
} else {
  localPhoneNumber = "未知号码";
  logCaptureLn(String("⚠️ SIM卡内未写入号码"));
}
  // 2. 给模组 5-8 秒的注网和获取 IP 的缓冲时间
  for (int i = 0; i < 8; i++) {
    server.handleClient(); // 保持网页能正常访问
    delay(1000);
  }

  // 3. 此时模组网络基本已经通了，再调用你原本的 NTP 同步
  logCaptureLn(String("正在同步NTP时间..."));
  // 注入国内的高速 NTP 服务器
  configTime(8 * 3600, 0, "ntp.aliyun.com", "ntp.ntsc.ac.cn", "pool.ntp.org"); 
  
  int ntpRetry = 0;
  // 通过 time(nullptr) 是否大于 100000 来判断网络时间是否真的同步成功
  while (time(nullptr) < 100000 && ntpRetry < 15) { 
    server.handleClient();
    delay(1000);
    ntpRetry++;
  }

  if (time(nullptr) >= 100000) {
    timeSynced = true;
    logCaptureLn(String("NTP时间同步成功！"));
  } else {
    logCaptureLn(String("NTP时间同步超时，将使用设备时间"));
  }

  ssl_client.setInsecure();
  digitalWrite(LED_BUILTIN, LOW);

  // ---- 启动通知 ----
  if (configValid) {
    logCaptureLn(String("配置有效，发送启动通知..."));
  String subject = "短信转发器已启动 [" + localPhoneNumber + "]";
  String body = "设备已启动\n";
  body += "本机号码：" + localPhoneNumber + "\n";
  body += "设备地址：" + getDeviceUrl();
    sendEmailNotification(subject.c_str(), body.c_str());
  }

}

void loop() {
  server.handleClient();
  if (!configValid) {
    if (millis() - lastPrintTime >= 1000) {
      lastPrintTime = millis();
      logCaptureLn(String("⚠️ 请访问 " + getDeviceUrl() + " 配置系统参数"));
    }
  }
  checkConcatTimeout();
  handleSerialConsole();
  checkSerial1URC();
}
