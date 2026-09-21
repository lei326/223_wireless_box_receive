#ifndef __SETGPIO_H__
#define __SETGPIO_H__

#define REG_SPACE_SIZE      0x1000
#define GPIO_BASE           0x10020000

#define HIGH 0xc00
#define LOW  0x400
#define BITWIDTH 0x04

#define HIGH_POWER 0x3000
#define LOW_POWER  0x1000

#define A0   0x00
#define A1   (A0+BITWIDTH)
#define A2   (A1+BITWIDTH)
#define A3   (A2+BITWIDTH)
#define A4   (A3+BITWIDTH)
#define A5   (A4+BITWIDTH)
#define A6   (A5+BITWIDTH)
#define A7   (A6+BITWIDTH)
#define A8   (A7+BITWIDTH)
#define A9   (A8+BITWIDTH)
#define A10  (A9+BITWIDTH)
#define A11  (A10+BITWIDTH)
#define A12  (A11+BITWIDTH)
#define A13  (A12+BITWIDTH)
#define A14  (A13+BITWIDTH)

#define B32  0x80
#define B33  (B32+BITWIDTH)
#define B34  (B33+BITWIDTH)
#define B35  (B34+BITWIDTH)
#define B36  (B35+BITWIDTH)
#define B37  (B36+BITWIDTH)

#define F160 0x280
#define F161 (F160+BITWIDTH)
#define F162 (F161+BITWIDTH)
#define F163 (F162+BITWIDTH)
#define F164 (F163+BITWIDTH)
#define F165 (F164+BITWIDTH)
#define F166 (F165+BITWIDTH)
#define F167 (F166+BITWIDTH)
#define F168 (F167+BITWIDTH)
#define F169 (F168+BITWIDTH)

int reg_read(unsigned long phyaddr, unsigned int *val);
int reg_write(unsigned long phyaddr, unsigned int val);
int gpio_write(int gpio, unsigned char value);
int gpio_read(int gpio, unsigned char *Value);

#endif
