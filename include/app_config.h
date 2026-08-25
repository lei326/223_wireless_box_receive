#pragma once

#define VIDEO_HOR_RES 1280
#define VIDEO_VER_RES 720

#define ALIGN_UP(x, a) (((x) + (a) - 1) / (a) * (a))
#define VIDEO_VER_RES_ALIGNED ALIGN_UP(VIDEO_VER_RES, 32) /* 720 -> 736 */

#define VB_BLK_SIZE (VIDEO_HOR_RES * VIDEO_VER_RES_ALIGNED * 3 / 2)
#define VB_BLK_CNT 3

/* ==================== 屏幕 / VO 输出 ====================
 *
 * 【已确认】TP2915 不做缩放（2026-08 与驱动工程师确认）
 *   => XM650 VO 输出分辨率 == TP2915 输出 == 屏的实际分辨率
 *   => 改分辨率时，SCREEN_HOR_RES / SCREEN_VER_RES 必须等于屏的规格
 *
 *  TODO(上板前): 屏的实际规格还没拿到，下面的值是【假设】
 *   1. 屏的分辨率？（当前按框图上的 "AHD 1080P" 假设为 1920x1080）
 *   2. 帧率？（当前假设 25fps，理由见下）
 *   3. RGB 位宽？RGB888(24位) / RGB565(16位)
 *   4. 时序是标准值还是需手填？
 *   5. 背光 PWM / display enable 是哪两个 GPIO？
 *
 * 帧率为什么倾向 25：
 *   TX 编码是 25fps。屏刷新率与之成整数倍关系时画面最平顺——
 *     25Hz → 1:1，最干净
 *     50Hz → 每帧显示 2 遍，也干净
 *     60Hz → 60/25 = 2.4 非整数，运动画面会有轻微顿挫(judder)
 *   若屏固定 60Hz，可考虑把 TX 编码帧率改成 30（30 与 60 成整数倍）
 *
 * 原厂 demo.cpp 的原文（1024x600 LCD，与本项目【链路不同】，仅供理解格式）：
 *     stVoPubAttr.enIntfSync = VO_OUTPUT_1024x600_60;   // 死赋值，被下面覆盖
 *     #ifdef OLDCOMMON
 *     stVoPubAttr.enIntfSync = VO_OUTPUT_1024x600_60;   // 老板子：预定义
 *     #else
 *     stVoPubAttr.enIntfSync = VO_OUTPUT_USER;          // 新板子：手填 ← 实际走这条
 *     stVoPubAttr.stSyncInfo.u16Hfb=160; u16Hbb=160; u16Hpw=24; u16Hact=1024;
 *     stVoPubAttr.stSyncInfo.u16Vfb=21;  u16Vbb=23;  u16Vpw=2;  u16Vact=600;
 *     #endif
 *   原厂是 XM650 直接驱动 LCD 面板，面板时序非标准所以要手填。
 *   我们中间隔了 TP2915，它是转换芯片、通常要标准时序，因此默认走预定义模式。
 */
#define BACKLIGHT_GPIO_ENABLE 0 /* 确认引脚后改成 1 */
#define GPIO_BACKLIGHT_PWM 0    /* 原厂 A8  */
#define GPIO_DISPLAY_ENABLE 0   /* 原厂 A10 */

#define VO_USE_PRESET_SYNC 0             /* 1=预定义  0=手填时序 */
#define VO_PRESET_SYNC VO_OUTPUT_1080P25 /* SDK 可选：1080P24/25/30/50/60、 \
                                          * 720P15/25/30/50/60、             \
                                          * 1024x600_60、800x480_60 等 */

#define SCREEN_HOR_RES 1920
#define SCREEN_VER_RES 1080

#define VO_H_PULSE_WIDTH 24        /* Hpw 行同步脉冲宽度 */
#define VO_H_BACK_PORCH 160        /* Hbb 行后肩         */
#define VO_H_ACTIVE SCREEN_HOR_RES /* Hact 有效像素 */
#define VO_H_FRONT_PORCH 160       /* Hfb 行前肩         */

#define VO_V_PULSE_WIDTH 2         /* Vpw 场同步脉冲宽度 */
#define VO_V_BACK_PORCH 23         /* Vbb 场后肩         */
#define VO_V_ACTIVE SCREEN_VER_RES /* Vact 有效行 */
#define VO_V_FRONT_PORCH 21        /* Vfb 场前肩         */

/* 背景色，YUV 格式的纯黑（Y=16, U=V=128）
 */
#define VO_BG_COLOR 0x108080

#define VO_LAYER_HEIGHT SCREEN_VER_RES

#define VO_DEV_ID 0
#define VO_LAYER_ID 0
#define VO_CHN_ID 0 /* 单画面，只用一个通道 */

/*VPSS*/
#define VPSS_GRP_ID 0
#define VPSS_CHN_ID 0

#define VPSS_MAX_WIDTH 1920
#define VPSS_MAX_HEIGHT 1080

#define VIDEO_FRAME_RATE 25

/*VDEC*/
#define VDEC_DEV_ID 0
#define VDEC_CHN_ID 0

#define VDEC_MAX_WIDTH 1920
#define VDEC_MAX_HEIGHT 1080

#define VDEC_BUF_SIZE (VDEC_MAX_WIDTH * VDEC_MAX_HEIGHT * 3 / 4)

#define VDEC_REF_FRAME_NUM 2

#define VDEC_ERR_BUF_FULL 0xa005800f