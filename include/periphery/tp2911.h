#pragma once

/* TP2911: RGB/BT1120 转 AHD 输出芯片，挂在 i2c0，从地址 0x88。
 * 复位脚 GPIO B33。必须在 VO 初始化之前配置完成。*/

int TP2911_Reset(void);                    /* 硬复位 */
int TP2911_BoardInit(void);                /* 显示使能 GPIO + VO 输出引脚复用 */
int TP2911_Init_1080P25(void);             /* 写寄存器，1080P25/74MHz */
int TP2911_SetVoOutFormat(void);           /* VO 输出格式寄存器，VO 初始化之后调 */
int TP2911_Verify(const char *tag);
