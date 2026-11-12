#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <ctype.h>
#include <strings.h>
#include <sys/stat.h>
#include "app_config.h"
#include "net/wifi_sync_mdl.h"
#include "periphery/sig100_mdl.h"
#include "cJson/cJSON.h"
#include <arpa/inet.h>
#include "net/net_client.h"

int WifiSyncMdl::ReadApInfo(char *ssid, int ssid_len, char *password, int password_len)
{
    FILE *fp = fopen(AP_CONF_PATH, "r");
    if (NULL == fp)
    {
        printf("[wifisync] open %s FAILED\n", AP_CONF_PATH);
        return -1;
    }

    ssid[0] = '\0';
    password[0] = '\0';

    char line[256];
    while (NULL != fgets(line, sizeof(line), fp))
    {
        if ('#' == line[0])
            continue;

        line[strcspn(line, "\r\n")] = '\0';

        if (0 == strncmp(line, "ssid=", 5))
            snprintf(ssid, ssid_len, "%s", line + 5);
        else if (0 == strncmp(line, "wpa_passphrase=", 15))
            snprintf(password, password_len, "%s", line + 15);
    }
    fclose(fp);

    if ('\0' == ssid[0])
    {
        printf("[wifisync] no ssid in %s\n", AP_CONF_PATH);
        return -1;
    }
    return 0;
}

int WifiSyncMdl::BuildMsg(const char *ssid, const char *password, char *buf, int buf_len)
{
    cJSON *param = cJSON_CreateObject();
    cJSON_AddStringToObject(param, "ssid", ssid);
    cJSON_AddStringToObject(param, "password", password);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "op", WIFI_SYNC_OP);
    cJSON_AddItemToObject(root, "param", param);
    char *body = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (NULL == body)
    {
        printf("[wifisync] cJSON_PrintUnformatted FAILED\n");
        return -1;
    }

    int len = snprintf(buf, buf_len, "%s\n", body);
    free(body);

    if (len < 0 || len >= buf_len)
    {
        printf("[wifisync] msg too long, len=%d\n", len);
        return -1;
    }
    return len;
}

WifiSyncMdl *WifiSyncMdl::Instance()
{
    static WifiSyncMdl inst;
    return &inst;
}

WifiSyncMdl::WifiSyncMdl()
    : m_tid(0), m_running(false), m_line_len(0)
{
    m_line[0] = '\0';
    m_last_mac[0] = '\0';
}

WifiSyncMdl::~WifiSyncMdl()
{
    Stop();
}

int WifiSyncMdl::Start()
{
    if (m_running)
    {
        printf("[wifisync] already started\n");
        return 0;
    }
    EnsureApPassword();

    m_running = true;
    if (0 != pthread_create(&m_tid, NULL, SendThreadEntry, this))
    {
        printf("[wifisync] pthread_create FAILED\n");
        m_running = false;
        return -1;
    }

    printf("[wifisync] started\n");
    return 0;
}

void WifiSyncMdl::Stop()
{
    if (!m_running)
        return;

    m_running = false;
    pthread_join(m_tid, NULL);
    printf("[wifisync] stopped\n");
}

void *WifiSyncMdl::SendThreadEntry(void *arg)
{
    WifiSyncMdl *self = (WifiSyncMdl *)arg;
    self->SendLoop();
    return NULL;
}


static bool IsValidMac(const char *mac)
{
    if (17 != strlen(mac))
        return false;
    for (int i = 0; i < 17; i++)
    {
        if (2 == i % 3)
        {
            if (':' != mac[i])
                return false;
        }
        else if (!isxdigit((unsigned char)mac[i]))
        {
            return false;
        }
    }
    return true;
}

void WifiSyncMdl::OnPlcData(const unsigned char *data, int len, void *user)
{
    WifiSyncMdl *self = (WifiSyncMdl *)user;
    if (NULL == self)
        return;
    self->Feed(data, len);
}

void WifiSyncMdl::Feed(const unsigned char *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        char c = (char)data[i];

        if ('\n' == c)
        {
            m_line[m_line_len] = '\0';
            HandleLine(m_line);
            m_line_len = 0;
            continue;
        }

        if (m_line_len >= WIFI_SYNC_MSG_MAX_LEN - 1)
        {
            printf("[wifisync] line too long, drop %d bytes\n", m_line_len);
            m_line_len = 0;
        }
        m_line[m_line_len++] = c;
    }
}

void WifiSyncMdl::HandleLine(char *line)
{
    char *json = strchr(line, '{');
    if (NULL == json)
        return;

    cJSON *root = cJSON_Parse(json);
    if (NULL == root)
    {
        printf("[wifisync] bad line, drop: %s\n", json);
        return;
    }

    cJSON *op = cJSON_GetObjectItem(root, "op");
    cJSON *param = cJSON_GetObjectItem(root, "param");
    cJSON *mac = (NULL != param) ? cJSON_GetObjectItem(param, "mac") : NULL;
    if (NULL == op || !cJSON_IsString(op) || 0 != strcmp(op->valuestring, MAC_SYNC_OP) ||
        NULL == mac || !cJSON_IsString(mac) || !IsValidMac(mac->valuestring))
    {
        printf("[wifisync] not a valid %s msg, drop: %s\n", MAC_SYNC_OP, json);
        cJSON_Delete(root);
        return;
    }

    if (0 != strcasecmp(mac->valuestring, m_last_mac))
    {
        if (0 == SetPairedTx(mac->valuestring))
            snprintf(m_last_mac, sizeof(m_last_mac), "%s", mac->valuestring);
    }
      cJSON *ip = cJSON_GetObjectItem(param, "ip");
    if (NULL != ip && cJSON_IsString(ip))
        SetPeerIp(ip->valuestring);

    cJSON_Delete(root);
}

void WifiSyncMdl::SendLoop()
{
    prctl(PR_SET_NAME, "wifi_sync");

    char ssid[WIFI_SSID_MAX_LEN + 1];
    char password[WIFI_PASSWORD_MAX_LEN + 1];
    char msg[WIFI_SYNC_MSG_MAX_LEN];

    while (m_running)
    {
        if (0 == ReadApInfo(ssid, sizeof(ssid), password, sizeof(password)))
        {
            int len = BuildMsg(ssid, password, msg, sizeof(msg));
            if (len > 0)
                Sig100Mdl::Instance()->Send((const unsigned char *)msg, len);
        }

        usleep(WIFI_SYNC_INTERVAL_MS * 1000);
    }

    printf("[wifisync] send thread exit\n");
}

static bool RunCli(const char *args, char *out, int out_len)
{
    char cmd[160];
    snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s %s 2>&1", AP_CTRL_DIR, args);

    if (NULL != out && out_len > 0)
        out[0] = '\0';

    FILE *pp = popen(cmd, "r");
    if (NULL == pp)
        return false;

    bool ok = false;
    int used = 0;
    char line[128];
    while (NULL != fgets(line, sizeof(line), pp))
    {
        if (0 == strncmp(line, "OK", 2) || 0 == strncmp(line, "PONG", 4))
            ok = true;
        if (NULL != out)
        {
            int left = out_len - used;
            if (left > 1)
            {
                int n = snprintf(out + used, left, "%s", line);
                used += (n < left) ? n : left - 1;
            }
        }
    }
    pclose(pp);
    return ok;
}

static int GenPassword(char *pw, int len)
{
    static const char kChars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    unsigned char rnd[64];
    int n = len - 1;
    if (n <= 0 || n > (int)sizeof(rnd))
        return -1;

    FILE *fp = fopen("/dev/urandom", "r");
    if (NULL == fp)
        return -1;
    int got = fread(rnd, 1, n, fp);
    fclose(fp);
    if (got != n)
        return -1;

    for (int i = 0; i < n; i++)
        pw[i] = kChars[rnd[i] % (sizeof(kChars) - 1)];
    pw[n] = '\0';
    return 0;
}

static int WriteApPassword(const char *pw)
{
    FILE *in = fopen(AP_CONF_PATH, "r");
    if (NULL == in)
    {
        printf("[wifisync] open %s FAILED\n", AP_CONF_PATH);
        return -1;
    }

    char tmp_path[128];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", AP_CONF_PATH);
    FILE *out = fopen(tmp_path, "w");
    if (NULL == out)
    {
        fclose(in);
        printf("[wifisync] open %s FAILED\n", tmp_path);
        return -1;
    }

    char line[256];
    while (NULL != fgets(line, sizeof(line), in))
    {
        if (0 == strncmp(line, "wpa", 3) || 0 == strncmp(line, "rsn_pairwise=", 13))
            continue;
        fputs(line, out);
        if (NULL == strchr(line, '\n'))
            fputc('\n', out);
    }
    fclose(in);

    fprintf(out, "wpa=2\n");
    fprintf(out, "wpa_passphrase=%s\n", pw);
    fprintf(out, "wpa_key_mgmt=WPA-PSK\n");
    fprintf(out, "rsn_pairwise=CCMP\n");
    fflush(out);
    fsync(fileno(out));
    fclose(out);

    if (0 != rename(tmp_path, AP_CONF_PATH))
    {
        printf("[wifisync] rename %s FAILED\n", tmp_path);
        unlink(tmp_path);
        return -1;
    }
    sync();
    return 0;
}

static int RestartHostapd()
{
    int r = system("killall hostapd 2>/dev/null");
    (void)r;
    sleep(1);

    if (0 != access(AP_ACCEPT_PATH, F_OK))
    {
        FILE *fp = fopen(AP_ACCEPT_PATH, "w");
        if (NULL != fp)
            fclose(fp);
        sync();
        printf("[wifisync] %s not found, create empty\n", AP_ACCEPT_PATH);
    }

    if (0 != system("hostapd " AP_CONF_PATH " -B &"))
    {
        printf("[wifisync] start hostapd FAILED\n");
        return -1;
    }

    for (int i = 0; i < 10; i++)
    {
        usleep(500 * 1000);
        if (RunCli("ping", NULL, 0))
        {
            char cfg[512];
            RunCli("get_config", cfg, sizeof(cfg));
            printf("[wifisync] hostapd restarted, %s\n",
                   NULL != strstr(cfg, "key_mgmt=WPA-PSK") ? "WPA2 on" : "WPA2 NOT on");
            return 0;
        }
    }
    printf("[wifisync] hostapd not responding after restart\n");
    return -1;
}

static int ReadStoredPsk(char *pw, int len)
{
    FILE *fp = fopen(AP_PSK_FILE, "r");
    if (NULL == fp)
        return -1;

    char line[80];
    char *got = fgets(line, sizeof(line), fp);
    fclose(fp);
    if (NULL == got)
        return -1;

    line[strcspn(line, "\r\n")] = '\0';
    int n = strlen(line);
    if (n < WIFI_PASSWORD_MIN_LEN || n > WIFI_PASSWORD_MAX_LEN)
        return -1;

    snprintf(pw, len, "%s", line);
    return 0;
}

static int SaveStoredPsk(const char *pw)
{
    char tmp_path[128];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", AP_PSK_FILE);
    FILE *fp = fopen(tmp_path, "w");
    if (NULL == fp)
    {
        printf("[wifisync] open %s FAILED\n", tmp_path);
        return -1;
    }
    fchmod(fileno(fp), 0600);
    fprintf(fp, "%s\n", pw);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    if (0 != rename(tmp_path, AP_PSK_FILE))
    {
        printf("[wifisync] rename %s FAILED\n", tmp_path);
        unlink(tmp_path);
        return -1;
    }
    sync();
    return 0;
}

static bool IsHostapdWpa()
{
    char cfg[512];
    RunCli("get_config", cfg, sizeof(cfg));
    return NULL != strstr(cfg, "key_mgmt=WPA-PSK");
}

int WifiSyncMdl::EnsureApPassword()
{
    char ssid[WIFI_SSID_MAX_LEN + 1];
    char conf_pw[WIFI_PASSWORD_MAX_LEN + 1];
    if (0 != ReadApInfo(ssid, sizeof(ssid), conf_pw, sizeof(conf_pw)))
        return -1;

    char pw[WIFI_PASSWORD_MAX_LEN + 1];
    if (0 != ReadStoredPsk(pw, sizeof(pw)))
    {
        if ((int)strlen(conf_pw) >= WIFI_PASSWORD_MIN_LEN)
        {
            snprintf(pw, sizeof(pw), "%s", conf_pw);
        }
        else if (0 != GenPassword(pw, AP_PSK_GEN_LEN + 1))
        {
            printf("[wifisync] gen password FAILED\n");
            return -1;
        }
        if (0 != SaveStoredPsk(pw))
            return -1;
        printf("[wifisync] ap password saved (%d chars)\n", (int)strlen(pw));
    }

    if (0 == strcmp(conf_pw, pw) && IsHostapdWpa())
    {
        printf("[wifisync] ap password ok (%d chars)\n", (int)strlen(pw));
        return 0;
    }

    if (0 != WriteApPassword(pw))
        return -1;
    printf("[wifisync] ap password applied (%d chars), restart hostapd\n", (int)strlen(pw));
    return RestartHostapd();
}

static bool IsOnlyLine(const char *path, const char *expect)
{
    FILE *fp = fopen(path, "r");
    if (NULL == fp)
        return false;

    char buf[64];
    int count = 0;
    bool match = false;
    while (NULL != fgets(buf, sizeof(buf), fp))
    {
        buf[strcspn(buf, "\r\n")] = '\0';
        if ('\0' == buf[0])
            continue;
        count++;
        match = (0 == strcasecmp(buf, expect));
    }
    fclose(fp);
    return 1 == count && match;
}

int WifiSyncMdl::SetPairedTx(const char *mac)
{
    char upper[18];
    for (int i = 0; i < 17; i++)
        upper[i] = toupper((unsigned char)mac[i]);
    upper[17] = '\0';

    char args[64];
    snprintf(args, sizeof(args), "accept_acl ADD_MAC %s", upper);
    if (!RunCli(args, NULL, 0))
    {
        printf("[wifisync] hostapd ADD_MAC %s FAILED\n", upper);
        return -1;
    }

    char list[512];
    RunCli("accept_acl SHOW", list, sizeof(list));
    for (char *p = strtok(list, "\n"); NULL != p; p = strtok(NULL, "\n"))
    {
        if (strlen(p) < 17)
            continue;
        char other[18];
        memcpy(other, p, 17);
        other[17] = '\0';
        if (!IsValidMac(other) || 0 == strcasecmp(other, upper))
            continue;
        snprintf(args, sizeof(args), "accept_acl DEL_MAC %s", other);
        RunCli(args, NULL, 0);
        printf("[wifisync] removed old mac %s\n", other);
    }

    char expect[32];
    snprintf(expect, sizeof(expect), "0:%s", upper);
    if (IsOnlyLine(AP_ACCEPT_PATH, expect))
    {
        printf("[wifisync] mac %s already paired\n", upper);
        return 0;
    }

    char tmp_path[128];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", AP_ACCEPT_PATH);
    FILE *fp = fopen(tmp_path, "w");
    if (NULL == fp)
    {
        printf("[wifisync] open %s FAILED\n", tmp_path);
        return -1;
    }
    fprintf(fp, "%s\n", expect);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    if (0 != rename(tmp_path, AP_ACCEPT_PATH))
    {
        printf("[wifisync] rename %s FAILED\n", tmp_path);
        unlink(tmp_path);
        return -1;
    }
    sync();

    printf("[wifisync] mac %s paired, whitelist now only this one\n", upper);
    return 0;
}

static int ReadPeerIpCfg(char *ip, int ip_len)
{
    FILE *fp = fopen(PEER_IP_CFG_PATH, "r");
    if (NULL == fp)
        return -1;
    char buf[512] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[n] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (NULL == root)
        return -1;
    cJSON *ch1 = cJSON_GetObjectItem(root, "ch1");
    int ret = -1;
    if (NULL != ch1 && cJSON_IsString(ch1))
    {
        snprintf(ip, ip_len, "%s", ch1->valuestring);
        ret = 0;
    }
    cJSON_Delete(root);
    return ret;
}

static int WritePeerIpCfg(const char *ip)
{
    char tmp_path[128];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", PEER_IP_CFG_PATH);
    FILE *fp = fopen(tmp_path, "w");
    if (NULL == fp)
    {
        printf("[wifisync] open %s FAILED\n", tmp_path);
        return -1;
    }
    fprintf(fp, "{\"ch1\": \"%s\", \"ch2\": \"%s\", \"ch3\": \"%s\", \"ch4\": \"%s\"}\n",
            ip, PEER_IP_DEFAULT, PEER_IP_DEFAULT, PEER_IP_DEFAULT);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    if (0 != rename(tmp_path, PEER_IP_CFG_PATH))
    {
        printf("[wifisync] rename %s FAILED\n", tmp_path);
        unlink(tmp_path);
        return -1;
    }
    sync();
    return 0;
}

int WifiSyncMdl::SetPeerIp(const char *ip)
{
    struct in_addr addr;
    if (1 != inet_pton(AF_INET, ip, &addr))
    {
        printf("[wifisync] bad peer ip, drop: %s\n", ip);
        return -1;
    }

    char cfg_ip[64] = {0};
    if (0 != ReadPeerIpCfg(cfg_ip, sizeof(cfg_ip)) || 0 != strcmp(cfg_ip, ip))
    {
        if (0 != WritePeerIpCfg(ip))
            return -1;
        printf("[wifisync] peer ip %s saved to %s\n", ip, PEER_IP_CFG_PATH);
    }

    NetClient::Instance()->UpdatePeerIp(ip);
    return 0;
}