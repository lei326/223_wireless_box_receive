#include "periphery/tp2911.h"
#include "periphery/io.h"
#include "periphery/setgpio.h"
#include "periphery/i2c.h"

#include <stdio.h>
#include <unistd.h>

#define GPIO_ADDR_BASE      0x10020000
#define XM_VO_OUT_FMT_REG   0x3101403c

#define TP_LOG(fmt, ...)  printf("[tp2911] " fmt "\n", ##__VA_ARGS__)

/* 复位脚电平。1 = 高，0 = 低 */
static int SetResetPin(int level)
{
    writel(GPIO_ADDR_BASE + B33, level ? HIGH : LOW);
    return 0;
}

int TP2911_Reset(void)
{
    SetResetPin(0);
    usleep(5 * 1000);
    SetResetPin(1);
    usleep(3 * 1000);
    TP_LOG("reset done");
    return 0;
}

int TP2911_SetVoOutFormat(void)
{
    writel(XM_VO_OUT_FMT_REG, 0x14);
    TP_LOG("vo out format set");
    return 0;
}

int TP2911_Init_1080P25(void)
{
    tp2911_i2c_write(0xff,0x00);
    tp2911_i2c_write(0x00,0x00);
    tp2911_i2c_write(0x01,0x00);
    tp2911_i2c_write(0x02,0x83);    
    tp2911_i2c_write(0x03,0x62);
    tp2911_i2c_write(0x04,0x00);
    tp2911_i2c_write(0x05,0x4a);
    tp2911_i2c_write(0x06,0x00);
    tp2911_i2c_write(0x07,0x41);
    tp2911_i2c_write(0x08,0xd0);
    tp2911_i2c_write(0x09,0x94);
    tp2911_i2c_write(0x0a,0x94);
    tp2911_i2c_write(0x0b,0x11);
    tp2911_i2c_write(0x0c,0x04);
    tp2911_i2c_write(0x0d,0x60);
    tp2911_i2c_write(0x0e,0x98);
    tp2911_i2c_write(0x0f,0x50);
    tp2911_i2c_write(0x10,0x1a);
    tp2911_i2c_write(0x11,0xe2);
    tp2911_i2c_write(0x12,0x00);
    tp2911_i2c_write(0x13,0x3c);
    tp2911_i2c_write(0x14,0x38);
    tp2911_i2c_write(0x15,0x39);
    tp2911_i2c_write(0x16,0xeb);
    tp2911_i2c_write(0x17,0x10);
    tp2911_i2c_write(0x18,0x10);
    tp2911_i2c_write(0x19,0xf0);
    tp2911_i2c_write(0x1a,0x10);
    tp2911_i2c_write(0x1b,0xa4);
    tp2911_i2c_write(0x1c,0x72);
    tp2911_i2c_write(0x1d,0x98);
    tp2911_i2c_write(0x1e,0x80);
    tp2911_i2c_write(0x1f,0x00);
    tp2911_i2c_write(0x20,0x29);
    tp2911_i2c_write(0x21,0x61);
    tp2911_i2c_write(0x22,0xbe);
    tp2911_i2c_write(0x23,0xd6);
    tp2911_i2c_write(0x24,0x86);
    tp2911_i2c_write(0x25,0x40);
    tp2911_i2c_write(0x26,0x00);
    tp2911_i2c_write(0x27,0x56);
    tp2911_i2c_write(0x28,0x00);
    tp2911_i2c_write(0x29,0x36);
    tp2911_i2c_write(0x2a,0x19);
    tp2911_i2c_write(0x2b,0x5e);
    tp2911_i2c_write(0x2c,0x60);
    tp2911_i2c_write(0x2d,0x00);
    tp2911_i2c_write(0x2e,0x00);
    tp2911_i2c_write(0x2f,0x00);
    tp2911_i2c_write(0x30,0x01);
    tp2911_i2c_write(0x31,0x00);
    tp2911_i2c_write(0x32,0x00);
    tp2911_i2c_write(0x33,0x00);
    tp2911_i2c_write(0x34,0x00);
    tp2911_i2c_write(0x35,0x00);
    tp2911_i2c_write(0x36,0x00);
    tp2911_i2c_write(0x37,0x00);
    tp2911_i2c_write(0x38,0x00);
    tp2911_i2c_write(0x39,0x00);
    tp2911_i2c_write(0x3a,0x00);
    tp2911_i2c_write(0x3b,0x90);
    tp2911_i2c_write(0x3c,0x50);
    tp2911_i2c_write(0x3d,0x80);
    tp2911_i2c_write(0x3e,0x18);
    tp2911_i2c_write(0x3f,0x00);
    tp2911_i2c_write(0x45,0x41);
    tp2911_i2c_write(0xf0,0x00);
    tp2911_i2c_write(0xf1,0x08);
    tp2911_i2c_write(0xf2,0x2a);
    tp2911_i2c_write(0xf3,0x4a);
    tp2911_i2c_write(0xf4,0x21);
    tp2911_i2c_write(0xf5,0x00);

    TP_LOG("init 1080P25/74MHz done");
    return 0;
}


int TP2911_Verify(const char *tag)
{
    static const struct {
        unsigned char reg;
        unsigned char expect;
    } chk[] = {
        { 0x02, 0x83 },   /* 你改过的那个 */
        { 0x0d, 0x60 },
        { 0x15, 0x39 },
        { 0x45, 0x41 },
        { 0xf1, 0x08 },
    };

    int n = (int)(sizeof(chk) / sizeof(chk[0]));
    int bad = 0;
    int i;

    for (i = 0; i < n; i++)
    {
        unsigned char v = tp2911_i2c_read(chk[i].reg);
        int ok = (v == chk[i].expect);
        if (!ok) bad++;
        TP_LOG("[%s] read 0x%02x = 0x%02x (expect 0x%02x) %s",
               tag, chk[i].reg, v, chk[i].expect, ok ? "OK" : "<<< MISMATCH");
    }

    TP_LOG("[%s] verify: %d/%d mismatch", tag, bad, n);
    return bad;
}


/* 引脚寄存器地址 = GPIO_BASE + 引脚号 * 4
 * 依据：setgpio.h 里 A0=0x00(0×4)、B32=0x80(32×4)、F160=0x280(160×4) */
#define GPIO_PIN_REG(n)   (GPIO_ADDR_BASE + (n) * 4)

static void DumpPinReg(int pin)
{
    unsigned int val = 0;
    readl(GPIO_PIN_REG(pin), &val);
    TP_LOG("gpio%-3d @0x%08lx = 0x%08x",
           pin, (unsigned long)GPIO_PIN_REG(pin), val);
}