
#include "disp/disp_mdl.h"
#include "app_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vo.h"        
#include "xm_comm_vo.h"

#define LOGI(fmt, ...) printf("[disp] " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[disp][ERR] " fmt "\n", ##__VA_ARGS__)


#define CHECK_MPI(expr)                                     \
    do {                                                    \
        XM_S32 _ret = (expr);                               \
        if (XM_SUCCESS != _ret) {                           \
            LOGE("%s failed at %s:%d, ret=%#x",             \
                 #expr, __FUNCTION__, __LINE__, _ret);      \
            return _ret;                                    \
        }                                                   \
    } while (0)


DispMdl* DispMdl::instance_ = NULL;

DispMdl* DispMdl::Instance()
{
    if (NULL == instance_) {
        instance_ = new DispMdl();
    }
    return instance_;
}

DispMdl::DispMdl()
    : started_(false), wait_i_frame_(true)
{
}

DispMdl::~DispMdl()
{
}

int DispMdl::SysInit()
{
    LOGI("SysInit begin");

    CHECK_MPI(XM_MPI_SYS_Init());
    LOGI("XM_MPI_SYS_Init ok");

    VB_CONF_S stVbConf;
    memset(&stVbConf, 0, sizeof(stVbConf));

    stVbConf.u32MaxPoolCnt             = 1;
    stVbConf.astCommPool[0].u32BlkSize = VB_BLK_SIZE;
    stVbConf.astCommPool[0].u32BlkCnt  = VB_BLK_CNT;

    CHECK_MPI(XM_MPI_VB_SetConf(&stVbConf));
    CHECK_MPI(XM_MPI_VB_Init());

    LOGI("VB pool ok: %dx%d(aligned) x3/2 = %u bytes, %u blocks, total %u KB",
         VIDEO_HOR_RES, VIDEO_VER_RES_ALIGNED,
         (unsigned)VB_BLK_SIZE, (unsigned)VB_BLK_CNT,
         (unsigned)(VB_BLK_SIZE * VB_BLK_CNT / 1024));

    LOGI("SysInit done");
    return XM_SUCCESS;
}

int DispMdl::VoInit()
{
    VO_PUB_ATTR_S stPubAttr;
    memset(&stPubAttr, 0, sizeof(stPubAttr));

    stPubAttr.u32BgColor = VO_BG_COLOR;
    stPubAttr.enIntfType = VO_INTF_VGA | VO_INTF_LCD;

#if VO_USE_PRESET_SYNC
    stPubAttr.enIntfSync = VO_PRESET_SYNC;

    CHECK_MPI(XM_MPI_VO_SetPubAttr(VO_DEV_ID, &stPubAttr));
    LOGI("VO PubAttr ok: preset sync, %dx%d", SCREEN_HOR_RES, SCREEN_VER_RES);

#else
    stPubAttr.enIntfSync = VO_OUTPUT_USER;
    stPubAttr.stSyncInfo.u16Hpw  = VO_H_PULSE_WIDTH;
    stPubAttr.stSyncInfo.u16Hbb  = VO_H_BACK_PORCH;
    stPubAttr.stSyncInfo.u16Hact = VO_H_ACTIVE;
    stPubAttr.stSyncInfo.u16Hfb  = VO_H_FRONT_PORCH;
    stPubAttr.stSyncInfo.u16Vpw  = VO_V_PULSE_WIDTH;
    stPubAttr.stSyncInfo.u16Vbb  = VO_V_BACK_PORCH;
    stPubAttr.stSyncInfo.u16Vact = VO_V_ACTIVE;
    stPubAttr.stSyncInfo.u16Vfb  = VO_V_FRONT_PORCH;

    CHECK_MPI(XM_MPI_VO_SetPubAttr(VO_DEV_ID, &stPubAttr));
    LOGI("VO PubAttr ok: user sync, %dx%d, H(pw%d bb%d act%d fb%d) V(pw%d bb%d act%d fb%d)",
         SCREEN_HOR_RES, SCREEN_VER_RES,
         VO_H_PULSE_WIDTH, VO_H_BACK_PORCH, VO_H_ACTIVE, VO_H_FRONT_PORCH,
         VO_V_PULSE_WIDTH, VO_V_BACK_PORCH, VO_V_ACTIVE, VO_V_FRONT_PORCH);
#endif

    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = SCREEN_HOR_RES;
    stLayerAttr.stDispRect.u32Height  = VO_LAYER_HEIGHT;   
    stLayerAttr.stImageSize.u32Width  = SCREEN_HOR_RES;
    stLayerAttr.stImageSize.u32Height = VO_LAYER_HEIGHT;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    CHECK_MPI(XM_MPI_VO_SetVideoLayerAttr(VO_LAYER_ID, &stLayerAttr));
    LOGI("VO LayerAttr ok: %dx%d, YUV420SP", SCREEN_HOR_RES, VO_LAYER_HEIGHT);

    VO_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    stChnAttr.stRect.s32X      = 0;
    stChnAttr.stRect.s32Y      = 0;
    stChnAttr.stRect.u32Width  = SCREEN_HOR_RES;
    stChnAttr.stRect.u32Height = SCREEN_VER_RES;

    CHECK_MPI(XM_MPI_VO_SetChnAttr(VO_LAYER_ID, VO_CHN_ID, &stChnAttr));
    CHECK_MPI(XM_MPI_VO_EnableChn(VO_LAYER_ID, VO_CHN_ID));
    CHECK_MPI(XM_MPI_VO_ShowChn(VO_LAYER_ID, VO_CHN_ID));
    LOGI("VO Chn%d ok: full screen %dx%d", VO_CHN_ID, SCREEN_HOR_RES, SCREEN_VER_RES);

    LOGI("VoInit done");
    return XM_SUCCESS;
}