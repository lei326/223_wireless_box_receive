#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "xm_middleware_api.h"
#include "app_config.h"
#include "disp/disp_mdl.h"
#include "net/net_client.h"
#include "stat/play_stat.h"
#include "net/net_client.h"
#include "net/hostapd_mdl.h"
#include <sys/select.h>
#include "periphery/sig100_mdl.h"
#include <string.h>
#include "net/wifi_sync_mdl.h"

// 热点信道文件里不是 2.4G 合法信道（1~13）就改成 AP_CHANNEL 并重启 */
static void CheckApChannelFile(void)
{
    FILE *fp = fopen(AP_CHANNEL_FILE, "r");
    if (NULL == fp)
    {
        printf("[main] %s open failed\n", AP_CHANNEL_FILE);
        return;
    }

    char buf[4] = {0};
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    int channel = atoi(buf);
    printf("[main] current_channel = %d\n", channel);
    if (channel >= 1 && channel <= 13)
        return;

    fp = fopen(AP_CHANNEL_FILE, "w");
    if (NULL == fp)
    {
        printf("[main] %s write failed, NOT reboot\n", AP_CHANNEL_FILE);
        return;
    }
    fprintf(fp, "%d", AP_CHANNEL);
    fclose(fp);
    sync();

    printf("[main] ap channel %d invalid, set to %d, reboot\n", channel, AP_CHANNEL);
    system("/sbin/reboot");
}

int main(int argc, char *argv[])
{
    prctl(PR_SET_NAME, "main");

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("========================================\n");
    printf("  video : %dx%d @%dfps (must match TX)\n",
           VIDEO_HOR_RES, VIDEO_VER_RES, VIDEO_FRAME_RATE);
    printf("  screen: %dx%d\n", SCREEN_HOR_RES, SCREEN_VER_RES);
    printf("========================================\n");
    CheckApChannelFile();

    XM_Middleware_Init();
    printf("[init] XM_Middleware_Init done\n");
    //if (0 != HostapdMdl::Instance()->Start())
    //{
    //    printf("[main] HostapdMdl Start FAILED\n");
   //}
    printf("[init] sdk version = %s\n", XM_Middleware_GetVersion());

    if (XM_SUCCESS != DispMdl::Instance()->Start())
    {
        printf("[main] DispMdl Start FAILED\n");
        return -1;
    }

    if (0 != NetClient::Instance()->Start("", SIGNAL_PORT, MEDIA_PORT))
    {
        printf("[main] NetClient Start FAILED\n");
        return -1;
    }

    if (0 != Sig100Mdl::Instance()->Start(WifiSyncMdl::OnPlcData, WifiSyncMdl::Instance()))
    {
        printf("[main] Sig100Mdl Start FAILED\n");
    }
    else if (0 != WifiSyncMdl::Instance()->Start())
    {
        printf("[main] WifiSyncMdl Start FAILED\n");
    }

    int n = 0;
    bool stdin_ok = true;   /* 开机脚本后台启动时 stdin 是 /dev/null，读到 EOF 后不再监听 */
    while (1)
    {
        struct timeval tv;
        fd_set rfds;
        FD_ZERO(&rfds);
        if (stdin_ok)
            FD_SET(0, &rfds);
        tv.tv_sec  = 5;
        tv.tv_usec = 0;

        if (select(stdin_ok ? 1 : 0, &rfds, NULL, NULL, &tv) > 0)
        {
            char buf[64];
            if (fgets(buf, sizeof(buf), stdin))
            {
                int v = 0;
                if (1 == sscanf(buf, "bright %d", &v))
                    NetClient::Instance()->SendColorCmd("Brightness", v);
                else if (1 == sscanf(buf, "contrast %d", &v))
                    NetClient::Instance()->SendColorCmd("contrast_ratio", v);
                else if (1 == sscanf(buf, "hue %d", &v))
                    NetClient::Instance()->SendColorCmd("hue", v);
                else
                    printf("usage: bright|contrast|hue <0-14>\n");
            }
            else
            {
                stdin_ok = false;
                printf("[main] stdin EOF, stop reading commands\n");
            }
            continue;
        }

        NetClient *net = NetClient::Instance();

        PlayInfo info;
        if (0 != PlayStat::Instance()->GetPlayInfo(&info))
        {
            printf("[stat] #%d net=%s | (no data)\n",
                   n++, net->IsConnected() ? "OK" : "DOWN");
            continue;
        }

        printf("[stat] #%d net=%s | recv %d fps, %d kbps, interval=%dms, total=%lld\n",
               n++,
               net->IsConnected() ? "OK" : "DOWN",
               info.fps,
               info.bitrate_bps / 1000,
               info.avg_interval_ms,
               (long long)info.total_frames);

        printf("[stat]     peer: %dx%d @%dfps %dkbps | wifi=%d (%d dBm)\n",
               net->GetPeerWidth(),
               net->GetPeerHeight(),
               net->GetPeerFps(),
               net->GetPeerBitrate(),
               net->GetWifiLevel(),
               net->GetWifiDbm());
    }
    return 0;
}