#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "xm_middleware_api.h"
#include "app_config.h"
#include "disp/disp_mdl.h"
#include "net/net_client.h"

int main(int argc, char *argv[])
{
    prctl(PR_SET_NAME, "main");

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("========================================\n");
    printf("  video : %dx%d @%dfps (must match TX)\n",
           VIDEO_HOR_RES, VIDEO_VER_RES, VIDEO_FRAME_RATE);
    printf("  screen: %dx%d\n", SCREEN_HOR_RES, SCREEN_VER_RES);
    printf("========================================\n");

    XM_Middleware_Init();
    printf("[init] XM_Middleware_Init done\n");
    printf("[init] sdk version = %s\n", XM_Middleware_GetVersion());

    if (XM_SUCCESS != DispMdl::Instance()->Start())
    {
        printf("[main] DispMdl Start FAILED\n");
        return -1;
    }

    if (0 != NetClient::Instance()->Start(TX_IP_ADDR, SIGNAL_PORT, MEDIA_PORT))
    {
        printf("[main] NetClient Start FAILED\n");
        return -1;
    }

    int n = 0;
    while (1)
    {
        sleep(5);
        printf("[alive] %d  net=%s\n", n++,
               NetClient::Instance()->IsConnected() ? "connected" : "disconnected");
    }
    return 0;
}