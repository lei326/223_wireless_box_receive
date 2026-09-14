#pragma once

#include <pthread.h>

class HostapdMdl
{
public:
    static HostapdMdl *Instance();

    int Start();
    void Stop();

    const char *GetSsid();
    const char *GetPsk();
    int FindPeerIp(char *ip, int ip_len);

private:
    HostapdMdl();
    ~HostapdMdl();
    HostapdMdl(const HostapdMdl &);
    HostapdMdl &operator=(const HostapdMdl &);

    int BuildIdentity();
    int WriteConf();
    int KillOld();
    int StartDaemon();
    int SetLocalIp();

    char m_ssid[33];
    char m_psk[17];
    bool m_running;
};