/*
 * net_client.cpp —— 网络客户端（RX 侧）
 * 原厂对应：demo.cpp 的 connect_tcp_thread / XMDataCallBack / XMClientEventCallBack
 */

#include "net/net_client.h"
#include "app_config.h"
#include "disp/disp_mdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "xm_middleware_api.h"
#include "xm_middleware_network.h"
#include "cJson/cJSON.h"
#include "frame_header.h"

#define LOGI(fmt, ...) printf("[net] " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[net][ERR] " fmt "\n", ##__VA_ARGS__)

static const int kChnum = 0;

static int64_t NowMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

NetClient *NetClient::instance_ = NULL;

NetClient *NetClient::Instance()
{
    if (NULL == instance_)
    {
        instance_ = new NetClient();
    }
    return instance_;
}

NetClient::NetClient()
    : signal_port_(0), media_port_(0),
      thread_tid_(0), thread_running_(false), connected_(false),
      last_request_idr_ms_(0),
      last_seq_(0), got_first_frame_(false)
{
    memset(ip_, 0, sizeof(ip_));
}

NetClient::~NetClient()
{
}

int NetClient::Start(const char *ip, int signal_port, int media_port)
{
    if (thread_running_)
    {
        LOGE("already running");
        return -1;
    }

    snprintf(ip_, sizeof(ip_), "%s", ip);
    signal_port_ = signal_port;
    media_port_ = media_port;
    LOGI("Start: connect to %s, signal=%d, media=%d", ip_, signal_port_, media_port_);
    XM_Middleware_Network_Client_Startup();

    XM_Middleware_Network_Client_SetDataCallback(kChnum, OnData);
    XM_Middleware_Network_Client_SetEventCallback(kChnum, OnEvent);
    XM_Middleware_Network_Client_SetHeartBeatTimeOut(kChnum, HEARTBEAT_TIMEOUT_MS);
    LOGI("callbacks registered, heartbeat timeout = %d ms", HEARTBEAT_TIMEOUT_MS);

    thread_running_ = true;
    int ret = pthread_create(&thread_tid_, NULL, ConnectThreadEntry, this);
    if (0 != ret)
    {
        LOGE("pthread_create failed, ret=%d", ret);
        thread_running_ = false;
        return -1;
    }
    LOGI("connect thread created");
    return 0;
}

void *NetClient::ConnectThreadEntry(void *arg)
{
    NetClient *self = static_cast<NetClient *>(arg);
    self->ConnectLoop();
    return NULL;
}

void NetClient::ConnectLoop()
{
    prctl(PR_SET_NAME, "net_connect");
    LOGI("connect loop start");

    while (thread_running_)
    {
        if (connected_)
        {
            usleep(500 * 1000);
            continue;
        }
        LOGI("connecting to %s:%d/%d ...", ip_, signal_port_, media_port_);

        XM_Middleware_Network_Client_StopClient(kChnum);
        int ret = XM_Middleware_Network_Client_StartClient(kChnum, ip_,
                                                           signal_port_, media_port_);
        if (ret < 0)
        {
            LOGE("StartClient failed, ret=%d, retry in %d ms", ret, RECONNECT_INTERVAL_MS);
            usleep(RECONNECT_INTERVAL_MS * 1000);
            continue;
        }
        connected_ = true;
        LOGI("connected");
        SendRealPlay();
    }
    LOGI("connect loop exit");
    return;
}

int NetClient::Stop()
{
    if (!thread_running_)
    {
        return 0;
    }

    thread_running_ = false;
    pthread_join(thread_tid_, NULL);
    thread_tid_ = 0;

    XM_Middleware_Network_Client_StopClient(kChnum);
    connected_ = false;

    LOGI("stopped");
    return 0;
}

int NetClient::SendRealPlay()
{
    cJSON *params = cJSON_CreateObject();
    cJSON_AddNumberToObject(params, "sdkversion", atof(XM_Middleware_GetVersion()));
    cJSON_AddNumberToObject(params, "HOR_RES", VIDEO_HOR_RES);
    cJSON_AddNumberToObject(params, "VER_RES", VIDEO_VER_RES);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "version", "1.0");
    cJSON_AddStringToObject(root, "type", "request");
    cJSON_AddStringToObject(root, "op", "realplay");
    cJSON_AddItemToObject(root, "param", params);
    char *body = cJSON_Print(root);
    cJSON_Delete(root);

    if (NULL == body)
    {
        LOGE("cJSON_Print failed");
        return -1;
    }
    int ret = XM_Middleware_Network_Client_SendData(kChnum, (uint8_t)XM_DATA_STRING,
                                                    body, strlen(body));
    LOGI("send realplay: %s", body);
    free(body);

    if (ret < 0)
    {
        LOGE("SendData failed, ret=%d", ret);
        return -1;
    }
    return 0;
}

void NetClient::OnData(int chnum, int engineId, int connId,
                       uint8_t type, char *data, int len)
{
    NetClient *self = Instance();

    if (XM_DATA_STREAM == type)
    {
        self->HandleStream(data, len);
    }
    else if (XM_DATA_STRING == type)
    {
        self->HandleJson(data);
    }
    else
    {
        LOGI("ignore data type=%d, len=%d", (int)type, len);
    }
}

void NetClient::OnEvent(int chnum, int engineId, int connId,
                        XMIPEventType type, char *data, int len)
{
    NetClient *self = Instance();

    if (XM_EVENT_DISCONNECT == type)
    {
        LOGI("DISCONNECT (engineId=%d, connId=%d)", engineId, connId);

        self->connected_ = false;

        DispMdl::Instance()->ResetDecoder();
    }
    else
    {
        LOGI("event type=%d (engineId=%d, connId=%d)", (int)type, engineId, connId);
    }
}

void NetClient::HandleJson(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (NULL == root)
    {
        LOGE("json parse error");
        return;
    }

    cJSON *op_item = cJSON_GetObjectItem(root, "op");
    if (NULL == op_item || !cJSON_IsString(op_item))
    {
        LOGE("no valid 'op' field");
        cJSON_Delete(root);
        return;
    }

    LOGI("recv reply: op=%s", op_item->valuestring);

    /* TODO(下一轮): realplay 响应 / heartbeat 应答 */

    cJSON_Delete(root);
}

void NetClient::HandleStream(const char *data, int len)
{
    const uint8_t *p = (const uint8_t *)data;
    if (len < 4)
    {
        LOGE("stream too short: len=%d", len);
        return;
    }
    if (p[0] != FRAME_START_CODE_0 || p[1] != FRAME_START_CODE_1 || p[2] != FRAME_START_CODE_2)
    {
        LOGE("bad start code: %02x %02x %02x", p[0], p[1], p[2]);
        return;
    }
    bool key_frame = false;
    int hdr_len = 0;
    uint8_t seq = 0;
    uint32_t data_len = 0;
    if (p[3] == FRAME_TYPE_I_FRAME)
    {
        key_frame = true;
        hdr_len = FRAME_LEN_I_FRAME;
        if (len < hdr_len)
        {
            LOGE("I frame header truncated: len=%d", len);
            return;
        }
        seq = p[FRAME_I_OFF_SEQ];
        data_len = FRAME_RD_U32(p, FRAME_I_OFF_DATA_LEN);
    }
    else if (p[3] == FRAME_TYPE_P_FRAME)
    {
        key_frame = false;
        hdr_len = FRAME_LEN_P_FRAME;
        if (len < hdr_len)
        {
            LOGE("P frame header truncated: len=%d", len);
            return;
        }
        seq = p[FRAME_P_OFF_SEQ];
        data_len = FRAME_RD_U32(p, FRAME_P_OFF_DATA_LEN);
    }
    else
    {
        /* 0xFA 音频目前阶段不处理 */
        return;
    }

    if ((int)(data_len + hdr_len) != len)
    {
        LOGE("frame len mismatch: hdr says %u + %d, actual %d", data_len, hdr_len, len);
        RequestIFrame();
        return;
    }

    if (!key_frame && got_first_frame_)
    {
        uint8_t expect = (uint8_t)(last_seq_ + 1);
        if (seq != expect)
        {
            LOGE("frame seq mismatch: expect %u, got %u", expect, seq);
            RequestIFrame();
        }
    }
    last_seq_ = seq;
    got_first_frame_ = true;
    const unsigned char *payload = (const unsigned char *)(data + hdr_len);
    int ret = DispMdl::Instance()->SendFrame(payload, data_len, key_frame);
    if (ret < 0)
    {
        RequestIFrame();
    }
}
void NetClient::RequestIFrame()
{
    int64_t now = NowMs();
    if (now - last_request_idr_ms_ < REQUEST_IDR_INTERVAL_MS)
    {
        return;
    }
    last_request_idr_ms_ = now;

    LOGI("request I frame from TX");
    XM_Middleware_Network_Client_processIFrame(kChnum);
}