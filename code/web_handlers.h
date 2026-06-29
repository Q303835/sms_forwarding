#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include "globals.h"

#define LOG_BUF_SIZE 120

extern String logBuffer[LOG_BUF_SIZE];
extern int logBufIdx;
extern int logBufCount;

void logCapture(const String& msg);
void logCapture(const char* msg);
void logCaptureF(const char* fmt, ...);
void logCaptureLn(const String& msg);
void logCaptureLn(const char* msg);

bool checkAuth();
void handleRoot();
void handleToolsPage();
void handleSave();
void handleQuery();
void handleFlightMode();
void handleATCommand();
void handleSendSms();
void handlePing();
void handleLog();
void handleModem();
void handleWifi();
void handleESim();
void handleSetAutoSms();
void handleESimPassthrough();// 处理进入透传模式的请求
void handleESimPassthroughStop();// 处理退出透传模式的请求
void handleESimPassthroughLogs();// 处理获取透传模式日志的请求
#endif
