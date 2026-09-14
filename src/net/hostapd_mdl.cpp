#include "net/hostapd_mdl.h"
#include "app_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>

#define AP_LOG(fmt, ...) printf("[ap] " fmt "\n", ##__VA_ARGS__)

static int GetLocalMac(const char *ifname, unsigned char *mac)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    int ret = -1;
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
    {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        ret = 0;
    }
    close(fd);
    return ret;
}

/* 读 MAC，拼出 SSID 和密码：
 *   MAC  = 2c:cc:7a:b7:25:34
 *   SSID = cardvr_wifi_2ccc7ab72534
 *   PSK  = 7ab72534          (SSID 末尾 8 位，即 MAC 后 4 字节)
 */
int HostapdMdl::BuildIdentity()
{
    unsigned char mac[6];
    if (GetLocalMac(AP_IFNAME, mac) != 0)
    {
        AP_LOG("GetLocalMac failed");
        return -1;
    }
    char mac_str[13];
    snprintf(mac_str, sizeof(mac_str), "%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(m_ssid, sizeof(m_ssid), "%s%s", AP_SSID_PREFIX, mac_str);
    int mac_len = strlen(mac_str);
    int off = mac_len - AP_PSK_LEN;
    if (off < 0)
        off = 0;
    snprintf(m_psk, sizeof(m_psk), "%s", mac_str + off);
    AP_LOG("identity: ssid=%s  psk=%s", m_ssid, m_psk);
    return 0;
}

int HostapdMdl::WriteConf()
{
    FILE *fp = fopen(AP_CONF_PATH, "w");
    if (!fp)
    {
        AP_LOG("open %s failed: %s", AP_CONF_PATH, strerror(errno));
        return -1;
    }

    fprintf(fp, "interface=%s\n", AP_IFNAME);
    fprintf(fp, "ctrl_interface=/var/run/hostapd\n");
    fprintf(fp, "ctrl_interface_group=0\n");
    fprintf(fp, "ssid=%s\n", m_ssid);
    fprintf(fp, "hw_mode=g\n");
    fprintf(fp, "channel=%d\n", AP_CHANNEL);
    fprintf(fp, "ap_max_inactivity=5\n");
    fprintf(fp, "ignore_broadcast_ssid=0\n");
    fprintf(fp, "ieee80211n=1\n");

    fprintf(fp, "wpa=2\n");
    fprintf(fp, "wpa_passphrase=%s\n", m_psk);
    fprintf(fp, "wpa_key_mgmt=WPA-PSK\n");
    fprintf(fp, "rsn_pairwise=CCMP\n");

    fclose(fp);

    AP_LOG("conf written: ssid=%s channel=%d WPA2", m_ssid, AP_CHANNEL);
    return 0;
}

int HostapdMdl::KillOld()
{
    int r;

    r = system("killall -9 hostapd 2>/dev/null");
    (void)r;
    usleep(300 * 1000);

    r = system("iw dev mon." AP_IFNAME " del 2>/dev/null");
    (void)r;

    r = system("ifconfig " AP_IFNAME " down 2>/dev/null");
    (void)r;
    r = system("iw dev " AP_IFNAME " set type managed 2>/dev/null");
    (void)r;
    r = system("ifconfig " AP_IFNAME " up 2>/dev/null");
    (void)r;

    unlink("/var/run/hostapd/" AP_IFNAME);
    usleep(500 * 1000);
    return 0;
}

int HostapdMdl::StartDaemon()
{
    char cmd[256];

    KillOld();
    int r = system("iw dev p2p0 del 2>/dev/null");
    (void)r;
    snprintf(cmd, sizeof(cmd), "hostapd %s -B", AP_CONF_PATH);
    AP_LOG("exec: %s", cmd);

    if (system(cmd) != 0)
    {
        AP_LOG("hostapd start FAILED");
        return -1;
    }

    usleep(AP_START_WAIT_MS * 1000);
    return 0;
}

int HostapdMdl::SetLocalIp()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        AP_LOG("socket failed: %s", strerror(errno));
        return -1;
    }

    struct ifreq ifr;
    struct sockaddr_in *sin;

    /* IP */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, AP_IFNAME, IFNAMSIZ - 1);
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, AP_LOCAL_IP, &sin->sin_addr) != 1 ||
        ioctl(fd, SIOCSIFADDR, &ifr) < 0)
    {
        AP_LOG("set ip failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, AP_IFNAME, IFNAMSIZ - 1);
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, AP_NETMASK, &sin->sin_addr) != 1 ||
        ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
        AP_LOG("set netmask failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, AP_IFNAME, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)
    {
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
        ioctl(fd, SIOCSIFFLAGS, &ifr);
    }

    close(fd);
    AP_LOG("ip configured: %s/%s", AP_LOCAL_IP, AP_NETMASK);
    return 0;
}

HostapdMdl *HostapdMdl::Instance()
{
    static HostapdMdl s_inst;
    return &s_inst;
}

HostapdMdl::HostapdMdl() : m_running(false)
{
    memset(m_ssid, 0, sizeof(m_ssid));
    memset(m_psk, 0, sizeof(m_psk));
}

HostapdMdl::~HostapdMdl()
{
    Stop();
}

int HostapdMdl::Start()
{
    if (m_running)
    {
        AP_LOG("already started");
        return 0;
    }

    FILE *fp = fopen("/proc/sys/kernel/printk", "w");
    if (fp)
    {
        fputs("1\n", fp);
        fclose(fp);
    }

    if (BuildIdentity() != 0)
        return -1;

    if (WriteConf() != 0)
        return -1;

    if (StartDaemon() != 0) 
        return -1;

    if (SetLocalIp() != 0)
        return -1;

    m_running = true;
    AP_LOG("=== AP READY: %s @ %s ===", m_ssid, AP_LOCAL_IP);
    return 0;
}

void HostapdMdl::Stop()
{
    if (!m_running)
        return;
    KillOld();
    m_running = false;
    AP_LOG("stopped");
}

const char *HostapdMdl::GetSsid() { return m_ssid; }
const char *HostapdMdl::GetPsk() { return m_psk; }

static int GetFirstStaMac(char *buf, int buf_len)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "iw dev %s station dump 2>/dev/null", AP_IFNAME);

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return -1;

    int ret = -1;
    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "Station ", 8) != 0)
            continue;
        if (strlen(line + 8) < 17)
            continue;

        snprintf(buf, buf_len, "%.17s", line + 8);
        ret = 0;
        break;
    }

    pclose(fp);
    return ret;
}

static int FindIpByMac(const char *mac, char *ip, int ip_len)
{
    FILE *fp = fopen("/proc/net/arp", "r");
    if (!fp)
        return -1;

    char line[256];
    int ret = -1;

    if (!fgets(line, sizeof(line), fp))
    {
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp))
    {
        char f_ip[64], f_hw[64], f_flag[64], f_mac[64], f_mask[64], f_dev[64];

        if (sscanf(line, "%63s %63s %63s %63s %63s %63s",
                   f_ip, f_hw, f_flag, f_mac, f_mask, f_dev) != 6)
            continue;

        if (strcmp(f_dev, AP_IFNAME) != 0) 
            continue;
        if (strcmp(f_flag, "0x2") != 0) 
            continue;
        if (strcasecmp(f_mac, mac) != 0) 
            continue;

        snprintf(ip, ip_len, "%s", f_ip);
        ret = 0;
        break;
    }

    fclose(fp);
    return ret;
}

int HostapdMdl::FindPeerIp(char *ip, int ip_len)
{
    char mac[18];

    if (GetFirstStaMac(mac, sizeof(mac)) != 0)
        return -1;

    if (FindIpByMac(mac, ip, ip_len) != 0)
    {
        AP_LOG("sta %s associated but no arp entry yet", mac);
        return -1;
    }

    AP_LOG("peer found: %s [%s]", ip, mac);
    return 0;
}