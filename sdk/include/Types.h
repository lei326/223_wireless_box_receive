//定义一些常用的数据类型

#ifndef __TYPES_H__
#define __TYPES_H__

typedef signed char			schar;
typedef unsigned char			uchar;	
typedef unsigned int			uint;
typedef unsigned short			ushort;
typedef unsigned long			ulong;
typedef long long				int64;
typedef unsigned long long		uint64;

typedef struct Rect
{
	int left;
	int top;
	int right;
	int bottom;
} Rect;

typedef struct Point
{	
	int x;	
	int y;			
} Point; 	
	
typedef struct Size
{
	int w;
	int h;
} Size;

typedef struct Color
{
	uchar r;	
	uchar g;	
	uchar b;	
	uchar a;	
} Color;

#endif

