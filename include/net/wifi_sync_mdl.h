#pragma once

#include <pthread.h>
#include "app_config.h"

class WifiSyncMdl
{
public:
    static WifiSyncMdl *Instance();

    int Start();
    void Stop();
    static void OnPlcData(const unsigned char *data, int len, void *user);

private:
    WifiSyncMdl();
    ~WifiSyncMdl();
    WifiSyncMdl(const WifiSyncMdl &);
    WifiSyncMdl &operator=(const WifiSyncMdl &);

    int ReadApInfo(char *ssid, int ssid_len, char *password, int password_len);
    int BuildMsg(const char *ssid, const char *password, char *buf, int buf_len);

    static void *SendThreadEntry(void *arg);
    void SendLoop();
    
    void Feed(const unsigned char *data, int len);
    void HandleLine(char *line);
    int SetPairedTx(const char *mac);
    int SetPeerIp(const char *ip);
    int EnsureApPassword();

    pthread_t m_tid;
    bool m_running;
    
    char m_line[WIFI_SYNC_MSG_MAX_LEN];
    int m_line_len;
    char m_last_mac[18];    
};