#pragma once

#include <pthread.h>

/*
 SIG100 的 HDC 脚拉高后进入正常工作模式，是透传：
 本端往串口写什么，对端串口就收到什么。所以这个模块本质上就是
 上电拉高 HDC + 一个串口的收发。
 */
class Sig100Mdl
{
public:
    typedef void (*RecvCallback)(const unsigned char *data, int len, void *user);

    static Sig100Mdl *Instance();

    int Start(RecvCallback cb, void *user); /* 拉高 HDC、切引脚复用、打开串口、起接收线程 */
    void Stop();
    int Send(const unsigned char *data, int len);

private:
    Sig100Mdl();
    ~Sig100Mdl();
    Sig100Mdl(const Sig100Mdl &);
    Sig100Mdl &operator=(const Sig100Mdl &);

    int SetHdcPin(int level);   
    int SetPinMux();           
    int OpenUart();          
    static void *RecvThreadEntry(void *arg);
    void RecvLoop();           

    int m_fd;               
    pthread_t m_tid;
    bool m_running;
    RecvCallback m_cb;
    void *m_user;
};