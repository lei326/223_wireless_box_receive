#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <ctype.h>
#include <strings.h>

#include "app_config.h"
#include "net/wifi_sync_mdl.h"
#include "periphery/sig100_mdl.h"
#include "cJson/cJSON.h"

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
        if (0 == AddToWhitelist(mac->valuestring))
            snprintf(m_last_mac, sizeof(m_last_mac), "%s", mac->valuestring);
    }

    cJSON_Delete(root);
}

int WifiSyncMdl::AddToWhitelist(const char *mac)
{
    char upper[18];
    for (int i = 0; i < 17; i++)
        upper[i] = toupper((unsigned char)mac[i]);
    upper[17] = '\0';

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s accept_acl ADD_MAC %s 2>&1", AP_CTRL_DIR, upper);
    bool ok = false;
    FILE *pp = popen(cmd, "r");
    if (NULL != pp)
    {
        char out[128];
        while (NULL != fgets(out, sizeof(out), pp))
        {
            if (0 == strncmp(out, "OK", 2))
                ok = true;
        }
        pclose(pp);
    }
    if (!ok)
    {
        printf("[wifisync] hostapd ADD_MAC %s FAILED\n", upper);
        return -1;
    }

    int max_index = -1;
    bool ends_with_newline = true;
    FILE *fp = fopen(AP_ACCEPT_PATH, "r");
    if (NULL != fp)
    {
        char line[64];
        while (NULL != fgets(line, sizeof(line), fp))
        {
            ends_with_newline = (NULL != strchr(line, '\n'));
            line[strcspn(line, "\r\n")] = '\0';
            int n = strlen(line);
            if (n < 17)
                continue;
            if (0 == strcasecmp(line + n - 17, upper))
            {
                fclose(fp);
                printf("[wifisync] mac %s already in whitelist\n", upper);
                return 0;
            }
            int index = atoi(line);
            if (index > max_index)
                max_index = index;
        }
        fclose(fp);
    }

    fp = fopen(AP_ACCEPT_PATH, "a");
    if (NULL == fp)
    {
        printf("[wifisync] open %s FAILED\n", AP_ACCEPT_PATH);
        return -1;
    }
    if (!ends_with_newline)
        fputc('\n', fp);
    fprintf(fp, "%d:%s\n", max_index + 1, upper);
    fclose(fp);
    sync();

    printf("[wifisync] mac %s added to whitelist, index %d\n", upper, max_index + 1);
    return 0;
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