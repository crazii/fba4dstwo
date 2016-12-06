#ifndef __pspadhoc_h__
#define __pspadhoc_h__

#define WIFI_CMD_RESET 0xFFFFFFFE
#define WIFI_CMD_SYNC_REQ 0xFFFFFFFD
#define WIFI_CMD_SYNC_RESP 0xFFFFFFFC

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned int inputKeys[3][3];
int adhocLoadDrivers();
int adhocTerm();
int adhocInit(char* netWorkName);
int wifiSend(unsigned int wifiCMD);
int wifiRecv();
void sendSyncGame();
void clearMacRecvCount();
void removePspFromList();

#ifdef __cplusplus
}
#endif

#endif
