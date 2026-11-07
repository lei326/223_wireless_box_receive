#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "app_config.h"
#include "periphery/sig100_mdl.h"
#include "periphery/io.h"
#include "periphery/setgpio.h"

#include <errno.h>
#include <sys/select.h>
#include <sys/prctl.h>
#include <string.h>

Sig100Mdl *Sig100Mdl::Instance()
{
    static Sig100Mdl inst;
    return &inst;
}

Sig100Mdl::Sig100Mdl()
    : m_fd(-1), m_tid(0), m_running(false), m_cb(NULL), m_user(NULL)
{
}

Sig100Mdl::~Sig100Mdl()
{
    Stop();
}

int Sig100Mdl::SetHdcPin(int level)
{
    int ret = writel(SIG100_HDC_REG, level ? HIGH : LOW);
    printf("[sig100] HDC -> %s (%s)\n", level ? "HIGH" : "LOW", ret == 0 ? "ok" : "fail");
    return ret;
}

int Sig100Mdl::SetPinMux()
{
    if (0 != writel(SIG100_PINMUX_REG0, SIG100_PINMUX_VAL) ||
        0 != writel(SIG100_PINMUX_REG1, SIG100_PINMUX_VAL))
    {
        printf("[sig100] pinmux FAILED\n");
        return -1;
    }
    return 0;
}

int Sig100Mdl::OpenUart()
{
    m_fd = open(SIG100_UART_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fd < 0)
    {
        printf("[sig100] open %s FAILED\n", SIG100_UART_DEV);
        return -1;
    }

    struct termios newtio;
    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = CS8 | CLOCAL | CREAD;
    newtio.c_iflag = 0;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 5;

    cfsetospeed(&newtio, SIG100_BAUDRATE);
    cfsetispeed(&newtio, SIG100_BAUDRATE);

    tcflush(m_fd, TCIOFLUSH);
    if (tcsetattr(m_fd, TCSANOW, &newtio) != 0)
    {
        printf("[sig100] tcsetattr FAILED\n");
        close(m_fd);
        m_fd = -1;
        return -1;
    }

    if (fcntl(m_fd, F_SETFL, 0) < 0)
    {
        printf("[sig100] fcntl FAILED\n");
        close(m_fd);
        m_fd = -1;
        return -1;
    }

    printf("[sig100] %s opened\n", SIG100_UART_DEV);
    return 0;
}

int Sig100Mdl::Start(RecvCallback cb, void *user)
{
    if (m_fd >= 0)
    {
        printf("[sig100] already started\n");
        return 0;
    }

    m_cb = cb;
    m_user = user;

    if (0 != SetHdcPin(1))      
        return -1;

    if (0 != SetPinMux())     
        return -1;

    if (0 != OpenUart())       
        return -1;

    m_running = true;
        if (0 != pthread_create(&m_tid, NULL, RecvThreadEntry, this))
    {
        printf("[sig100] pthread_create FAILED\n");
        m_running = false;
        close(m_fd);
        m_fd = -1;
        return -1;
    }

    printf("[sig100] started\n");
    return 0;
}

void Sig100Mdl::Stop()
{
    if (m_running)
    {
        m_running = false;          
        pthread_join(m_tid, NULL);   
    }

    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
        printf("[sig100] stopped\n");
    }
}

void *Sig100Mdl::RecvThreadEntry(void *arg)
{
    Sig100Mdl *self = (Sig100Mdl *)arg;
    self->RecvLoop();
    return NULL;
}

void Sig100Mdl::RecvLoop()
{
    prctl(PR_SET_NAME, "sig100_recv");

    unsigned char buf[30];        

    while (m_running)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_fd, &rfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500 * 1000;  

        int ret = select(m_fd + 1, &rfds, NULL, NULL, &tv);
        if (ret == 0)
            continue;     
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;           
            printf("[sig100] select error, errno=%d\n", errno);
            usleep(100 * 1000);
            continue;
        }

        int n = read(m_fd, buf, sizeof(buf));
        if (n <= 0)
        {
            printf("[sig100] read error, n=%d errno=%d\n", n, errno);
            usleep(100 * 1000);
            continue;
        }

        printf("[sig100] recv %d bytes:", n);
        for (int i = 0; i < n; i++)
            printf(" %02x", buf[i]);
        printf("\n");

        if (m_cb)
            m_cb(buf, n, m_user);
    }

    printf("[sig100] recv thread exit\n");
}