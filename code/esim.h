#ifndef ESIM_H
#define ESIM_H

#include "globals.h"

#define ESIM_MAX_ICCID_LEN 21
#define ESIM_MAX_ISDP_AID_LEN 33
#define ESIM_MAX_NAME_LEN 64

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
bool esimEnableProfile(const char* iccidOrAid);
bool esimDisableProfile(const char* iccidOrAid);
bool esimDeleteProfile(const char* iccidOrAid);
bool esimSwitchProfile(const char* iccidOrAid);
bool esimGetNotificationCount(int* count);
const char* esimGetLastError();

bool handleSerialConsole();
bool handleESimSerialCommand(const String& command);

#endif
