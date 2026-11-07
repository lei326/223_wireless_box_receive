#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>

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
    : m_tid(0), m_running(false)
{
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