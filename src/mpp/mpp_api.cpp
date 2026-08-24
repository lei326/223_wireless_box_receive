
#include <stdio.h>
#include <stdint.h>

#include "CommDef.h"
#include "xm_middleware_def.h"
#include "xm_ia_comm.h"

const int kFrameRate = 25;


/* 开始回放（SD 卡录像回看）。第一阶段与这无关 */
int XM_Middleware_Mpp_StartPlayback(CUR_PLAY_STATUS play_status,
                                    PAYLOAD_TYPE_E  codec_type,
                                    RECT_S*         display_region,
                                    bool            audio_only)
{
    printf("[stub] XM_Middleware_Mpp_StartPlayback not implemented\n");
    return -1;
}

/* 送一帧去解码（回放通路）。第一阶段与这无关 */
int XM_Middleware_Mpp_SendDecFrame(long handle,
                                   XM_MW_Media_Frame* media_frame,
                                   bool bPreAudio)
{
    return -1;
}

/* 回放的倒放/后退送帧。第一阶段与这无关 */
int XM_Middleware_Mpp_SendBackDecFrame(long handle,
                                       XM_MW_Media_Frame* media_frame)
{
    return -1;
}

/* AI 报警框的 OSD 叠加。第一阶段与这无关 */
int XM_Middleware_Mpp_OsdShow(int Channel, bool show,
                              const XM_IA_TCP_RESULT_S* stTdRlt)
{
    return -1;
}

/* 播放统计（帧率/码率/延迟）*/
int XM_Middleware_Mpp_GetPlayInfo(int channel, int* rate, int* bitrate,
                                  int* framenum, int64_t* max_frame_duration_ms,
                                  int* delay_time)
{
    return -1;
}