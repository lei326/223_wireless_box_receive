#pragma once

#define VIDEO_HOR_RES 1920
#define VIDEO_VER_RES 1080

#define ALIGN_UP(x, a) (((x) + (a) - 1) / (a) * (a))
#define VIDEO_VER_RES_ALIGNED ALIGN_UP(VIDEO_VER_RES, 32) /* 720 -> 736 */

#define VB_BLK_SIZE (VIDEO_HOR_RES * VIDEO_VER_RES_ALIGNED * 3 / 2)
#define VB_BLK_CNT 3

/* ==================== WiFi 热点（AP 侧） ==================== */
#define AP_IFNAME           "eth2"
#define AP_SSID_PREFIX      "cardvr_wifi_"
#define AP_CHANNEL          3
#define AP_LOCAL_IP         "172.168.10.1"
#define AP_NETMASK          "255.255.0.0"

#define AP_CONF_PATH        "/mnt/mtd/Config/hostapd.conf"
#define AP_PSK_LEN          8             

#define AP_START_WAIT_MS    2000         

#define BACKLIGHT_GPIO_ENABLE 0 /* 确认引脚后改成 1 */
#define GPIO_BACKLIGHT_PWM 0    /* 原厂 A8  */
#define GPIO_DISPLAY_ENABLE 0   /* 原厂 A10 */

#define VO_PRESET_SYNC VO_OUTPUT_1080P25 /* SDK 可选：1080P24/25/30/50/60、 \
                                          * 720P15/25/30/50/60、             \
                                          * 1024x600_60、800x480_60 等 */

#define SCREEN_HOR_RES 1920
#define SCREEN_VER_RES 1080

#define VO_H_PULSE_WIDTH 40        /* Hpw 行同步脉冲宽度 */
#define VO_H_BACK_PORCH 360        /* Hbb 行后肩         */
#define VO_H_ACTIVE SCREEN_HOR_RES /* Hact 有效像素 */
#define VO_H_FRONT_PORCH 320       /* Hfb 行前肩         */

#define VO_V_PULSE_WIDTH 30         /* Vpw 场同步脉冲宽度 */
#define VO_V_BACK_PORCH 8         /* Vbb 场后肩         */
#define VO_V_ACTIVE SCREEN_VER_RES /* Vact 有效行 */
#define VO_V_FRONT_PORCH 7       /* Vfb 场前肩         */

/* 背景色，YUV 格式的纯黑（Y=16, U=V=128）
 */
#define VO_BG_COLOR 0xFF0000 

#define VO_LAYER_HEIGHT SCREEN_VER_RES

#define VO_DEV_ID 0
#define VO_LAYER_ID 0
#define VO_CHN_ID 0 /* 单画面，只用一个通道 */

/*VPSS*/
#define VPSS_GRP_ID 0
#define VPSS_CHN_ID 0

#define VPSS_MAX_WIDTH 1920
#define VPSS_MAX_HEIGHT 1080

#define VIDEO_FRAME_RATE 20

/*VDEC*/
#define VDEC_DEV_ID 0
#define VDEC_CHN_ID 0

#define VDEC_MAX_WIDTH 1920
#define VDEC_MAX_HEIGHT 1080

#define VDEC_BUF_SIZE (VDEC_MAX_WIDTH * VDEC_MAX_HEIGHT * 3 / 4)

#define VDEC_REF_FRAME_NUM 2
#define HEARTBEAT_KEEP_INTERVAL  5      /* 心跳发送间隔，配合 timeout 3000ms */
#define VDEC_ERR_BUF_FULL 0xa005800f

/* ==================== 网络 ====================
 */
#define SIGNAL_PORT             2222              /* 信令口：JSON 控制命令 */
#define MEDIA_PORT              2223              /* 媒体口：音视频码流   */

#define HEARTBEAT_TIMEOUT_MS    3000

#define RECONNECT_INTERVAL_MS   1000

#define REQUEST_IDR_INTERVAL_MS 200


#define PEER_COLOR_MIN      0
#define PEER_COLOR_MAX      14
#define VO_INTF_TYPE            VO_INTF_BT1120

//PLC
#define SIG100_UART_DEV     "/dev/ttyAMA2"
#define SIG100_BAUDRATE     B19200
#define SIG100_HDC_REG      (0x10020000 + 0x200)   /* GPIO_ADDR_BASE + E128 */
#define SIG100_PINMUX_REG0  0x100203a0
#define SIG100_PINMUX_REG1  0x100203a4
#define SIG100_PINMUX_VAL   0x84

#define WIFI_SYNC_OP            "wifi_info"   /* 消息里 op 的值 */
#define WIFI_SYNC_INTERVAL_MS   3000          /* 发送间隔 */
#define WIFI_SSID_MAX_LEN       32            /* SSID 最长字节数 */
#define WIFI_PASSWORD_MAX_LEN   63            /* 密码最长字符数 */
#define WIFI_SYNC_MSG_MAX_LEN   256           /* 消息缓冲区大小 */

//收 MAC,加进热点白名单
#define MAC_SYNC_OP             "tx_mac"                          
#define AP_ACCEPT_PATH          "/mnt/mtd/Config/hostapd.accept"  
#define AP_CTRL_DIR             "/var/run/hostapd"          

//热点密码
#define WIFI_PASSWORD_MIN_LEN   8             /* WPA2 口令最短 8 个字符 */
#define AP_PSK_GEN_LEN          12            /* 随机生成的密码长度 */   
#define AP_PSK_FILE             "/mnt/mtd/Config/ap_psk"   /* 自己保存的热点密码 */