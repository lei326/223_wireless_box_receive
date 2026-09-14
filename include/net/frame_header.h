#pragma once

/* 起始码，四种包共有 */
#define FRAME_START_CODE_0 0x00
#define FRAME_START_CODE_1 0x00
#define FRAME_START_CODE_2 0x01

#define FRAME_TYPE_I_FRAME 0xFD /* H.265 I 帧，头长 24 */
#define FRAME_TYPE_P_FRAME 0xFC /* H.265 P 帧，头长 18 */
#define FRAME_TYPE_AUDIO 0xFA   /* PCMA 音频，头长 14（本阶段不管）*/
#define FRAME_TYPE_JPEG 0xFB    /* JPEG 图片，头长 12（本阶段不管）*/

/* 头长度 */
#define FRAME_LEN_I_FRAME 24
#define FRAME_LEN_P_FRAME 18

/* ---- I 帧头内各字段的偏移 ---- */
#define FRAME_I_OFF_SEQ 5        /* 1 字节，帧序号 */
#define FRAME_I_OFF_WIDTH 8      /* 2 字节，小端 */
#define FRAME_I_OFF_HEIGHT 10    /* 2 字节，小端 */
#define FRAME_I_OFF_TIMESTAMP 12 /* 4 字节，小端 */
#define FRAME_I_OFF_DATA_LEN 16  /* 4 字节，小端 */
#define FRAME_I_OFF_CRC32 20     /* 4 字节，小端 */

/* ---- P 帧头内各字段的偏移 ---- */
#define FRAME_P_OFF_SEQ 4
#define FRAME_P_OFF_TIMESTAMP 6
#define FRAME_P_OFF_DATA_LEN 10
#define FRAME_P_OFF_CRC32 14

#define FRAME_RD_U16(p, off) ((uint16_t)((p)[(off)] | ((p)[(off) + 1] << 8)))
#define FRAME_RD_U32(p, off) ((uint32_t)((p)[(off)] |             \
                                         ((p)[(off) + 1] << 8) |  \
                                         ((p)[(off) + 2] << 16) | \
                                         ((uint32_t)(p)[(off) + 3] << 24)))
