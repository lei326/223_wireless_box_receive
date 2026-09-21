/*************************************************************************
 * Copyright (C) 2022 Xmsilicon Tech. Co., Ltd.
 * @File Name: i2c.c
 * @Description:
 * @Author: songliuyang
 * @Mail: songliuyang@xmsilicon.cn
 * @Created Time: 2022.11.23
 * @Modification:
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "i2c.h"

 //#define I2C_DEV "/dev/io_i2c"
#define I2C_DEV "/dev/i2c1"


static int g_i2c_fd = -1;
//static pthread_mutex_t i2c_lock_set[2] = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };


static int i2c_init(void)
{
	g_i2c_fd = open(I2C_DEV, O_RDWR);
	if (g_i2c_fd < 0)
	{
		printf("failed to open %s\n", I2C_DEV);
		return -1;
	}
	printf("%s init.\n", I2C_DEV);
	return 0;
}

static int i2c_write(const unsigned char dev_addr, const unsigned int reg_addr, int reg_count,
	const unsigned char* data, unsigned int data_count)
{
	if (NULL == data)
	{
		printf("Invalid Paramer.\n");
		return -1;
	}

	if (g_i2c_fd < 0)
		i2c_init();

	int ret;
	i2c_data_s i2c_data;
	memset(&i2c_data, 0, sizeof(i2c_data_s));

	i2c_data.dev_addr = dev_addr >> 1;
	i2c_data.reg_addr = reg_addr;
	i2c_data.addr_byte_num = reg_count;
	i2c_data.data = *data;
	i2c_data.data_byte_num = data_count;
	ret = ioctl(g_i2c_fd, CMD_I2C_WRITE, &i2c_data);
	if (ret < 0)
	{
		return -1;
	}

	return 0;
}

static int i2c_read(const unsigned char dev_addr, const unsigned int reg_addr, int reg_count,
	unsigned char* data, unsigned int data_count)
{
	if (NULL == data)
	{
		printf("Invalid Paramer.\n");
		return -1;
	}

	if (g_i2c_fd < 0)
		i2c_init();

	int ret;
	i2c_data_s i2c_data;
	memset(&i2c_data, 0, sizeof(i2c_data_s));

	i2c_data.dev_addr = dev_addr >> 1;
	i2c_data.reg_addr = reg_addr;
	i2c_data.addr_byte_num = reg_count;
	i2c_data.data_byte_num = data_count;
	ret = ioctl(g_i2c_fd, CMD_I2C_READ, &i2c_data);
	if (ret < 0)
	{
		return -1;
	}

	*data = i2c_data.data;

	return 0;
}



/* write AD register */
int da380_i2c_write(unsigned char device_addr, unsigned char reg_addr, unsigned char reg_val)
{
	return i2c_write(device_addr, (unsigned int)reg_addr, 1, &reg_val, 1);
}


/* read AD register */
unsigned char da380_i2c_read(unsigned char device_addr, unsigned char reg_addr)
{
	unsigned char reg_val = 0x00;
	i2c_read(device_addr, (unsigned int)reg_addr, 1, &reg_val, 1);
	return reg_val;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

//xm650 tp2911 connect to i2c0
#define 	TP2911_I2C_DEV       	 "/dev/i2c0"
#define 	TP2911_SLAVE_ADDR     		 0x88
static int   g_tp2911_i2c_fd = -1;

typedef   unsigned char  	u8;
typedef   unsigned int  	u16;


static int tp2911_i2c_init(void)
{
	g_tp2911_i2c_fd = open(TP2911_I2C_DEV, O_RDWR);
	if (g_tp2911_i2c_fd < 0)
	{
		printf("failed to open %s\n", TP2911_I2C_DEV);
		return -1;
	}
	printf("%s tp2911_i2c_init.\n", TP2911_I2C_DEV);
	return 0;
}

/*

  一次只写一个寄存器
  
*/
int tp2911_i2c_write( u16 reg_addr, u8 data)
{
	
	u8 dev_addr = TP2911_SLAVE_ADDR;
	int reg_count = 1;
	u16 data_count = 1;

	if (g_tp2911_i2c_fd < 0)
		tp2911_i2c_init();

	int ret;
	i2c_data_s i2c_data;
	memset(&i2c_data, 0, sizeof(i2c_data_s));

	i2c_data.dev_addr = dev_addr >> 1;
	i2c_data.reg_addr = reg_addr;
	i2c_data.addr_byte_num = reg_count;
	i2c_data.data = data;
	i2c_data.data_byte_num = data_count;
	ret = ioctl(g_tp2911_i2c_fd, CMD_I2C_WRITE, &i2c_data);
	if (ret < 0)
	{
		return -1;
	}

	return 0;
}

u8 tp2911_i2c_read( const unsigned int reg_addr)
{

	u8  dev_addr = TP2911_SLAVE_ADDR;
	int reg_count = 1;
	u16 data_count = 1;
	u8  data=0;

	if (g_tp2911_i2c_fd < 0)
		tp2911_i2c_init();

	int ret;
	i2c_data_s i2c_data;
	memset(&i2c_data, 0, sizeof(i2c_data_s));

	i2c_data.dev_addr = dev_addr >> 1;
	i2c_data.reg_addr = reg_addr;
	i2c_data.addr_byte_num = reg_count;
	i2c_data.data_byte_num = data_count;
	ret = ioctl(g_tp2911_i2c_fd, CMD_I2C_READ, &i2c_data);
	if (ret < 0)
	{
		return -1;
	}

	data = (u8) i2c_data.data;

	return data;
}





/////////////////////////////////////////////////////////////////////////////////////////////////////

//i2c lock
//int  pthread_mutex_lock_i2c(unsigned char i2c_lock)
//{
//	int lock_sel = 0, ret;
//
//	if (9 == i2c_lock)
//	{
//		ret = pthread_mutex_lock(i2c_lock_set);
//		ret = pthread_mutex_lock(i2c_lock_set + 1);
//		return ret;
//	}
//
//	ret = pthread_mutex_lock(i2c_lock_set + lock_sel);
//	return ret;
//}
//
//int  pthread_mutex_unlock_i2c(unsigned char i2c_lock)
//{
//	int lock_sel = 0, ret;
//
//	if (9 == i2c_lock)
//	{
//		ret = pthread_mutex_unlock(i2c_lock_set + 1);
//		ret = pthread_mutex_unlock(i2c_lock_set);
//		return ret;
//	}
//
//	ret = pthread_mutex_unlock(i2c_lock_set + lock_sel);
//	return ret;
//}


