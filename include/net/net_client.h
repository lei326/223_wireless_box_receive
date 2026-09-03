#pragma once

#include <stdint.h>
#include <pthread.h>
#include <string>
#include "xm_common.h"
#include "XMIPDef.h"
#include "cJson/cJSON.h"

class NetClient
{
public:
    static NetClient *Instance();

    int Start(const char *ip, int signal_port, int media_port);
    int Stop();

    bool IsConnected() const { return connected_; }
    void RequestIFrame();

    int GetPeerWidth() const { return peer_width_; };
    int GetPeerHeight() const { return peer_height_; };
    int GetPeerFps() const { return peer_fps_; };
    int GetPeerBitrate() const { return peer_bitrate_; };
    int GetWifiLevel() const { return wifi_level_; };
    int GetWifiDbm() const { return wifi_dbm_; };

private:
    NetClient();
    ~NetClient();

    static void OnData(int chnum, int engineId, int connId,
                       uint8_t type, char *data, int len);
    static void OnEvent(int chnum, int engineId, int connId,
                        XMIPEventType type, char *data, int len);

    static void *ConnectThreadEntry(void *arg);
    void ConnectLoop();
    void OnRealPlayReply(cJSON *param);
    void OnHeartbeatReply(cJSON *param);
    void OnResolutionChanged(cJSON *param);
    static int JsonGetInt(cJSON *param, const char *key, int def);
    int SendRealPlay();

    void HandleStream(const char *data, int len); /* ③ 剥私有头 → 送解码 */
    void HandleJson(const char *json);            /* ② TX 的 JSON 回复  */

    static NetClient *instance_;

    char ip_[64];
    int signal_port_;
    int media_port_;

    pthread_t thread_tid_;
    volatile bool thread_running_;
    volatile bool connected_;
    uint8_t last_seq_;
    volatile bool got_first_frame_;
    int64_t last_request_idr_ms_;

    /* 对端编码参数，来自 realplay 响应 / resolutionChanged 通知 */
    int peer_width_;
    int peer_height_;
    int peer_fps_;
    int peer_bitrate_;

    /* TX 报的 WiFi 信号，来自 heartbeat 响应
     */
    int wifi_level_;
    int wifi_dbm_;
};