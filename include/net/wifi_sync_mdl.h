#pragma once

#include <pthread.h>

class WifiSyncMdl
{
public:
    static WifiSyncMdl *Instance();

    int Start();
    void Stop();

private:
    WifiSyncMdl();
    ~WifiSyncMdl();
    WifiSyncMdl(const WifiSyncMdl &);
    WifiSyncMdl &operator=(const WifiSyncMdl &);

    int ReadApInfo(char *ssid, int ssid_len, char *password, int password_len);
    int BuildMsg(const char *ssid, const char *password, char *buf, int buf_len);

    static void *SendThreadEntry(void *arg);
    void SendLoop();

    pthread_t m_tid;
    bool m_running;
};