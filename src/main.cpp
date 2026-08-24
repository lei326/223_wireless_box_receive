#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/prctl.h> 
#include "xm_middleware_api.h"

int main(int argc, char* argv[])
{
    prctl(PR_SET_NAME, "main");
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("========================================\n");
    printf("  wireless_box_receive  (RX / XM650)\n");
    printf("========================================\n");

    XM_Middleware_Init();
    printf("[init] XM_Middleware_Init done\n");
    printf("[init] sdk version = %s\n", XM_Middleware_GetVersion());

    int n = 0;
    while (1) {
        printf("[alive] %d\n", n++);
        sleep(5);
    }
    return 0;
}