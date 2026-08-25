
#include "disp/disp_mdl.h"
#include "app_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vo.h"
#include "xm_comm_vo.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"

#define LOGI(fmt, ...) printf("[disp] " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[disp][ERR] " fmt "\n", ##__VA_ARGS__)

#define CHECK_MPI(expr)                                \
    do                                                 \
    {                                                  \
        XM_S32 _ret = (expr);                          \
        if (XM_SUCCESS != _ret)                        \
        {                                              \
            LOGE("%s failed at %s:%d, ret=%#x",        \
                 #expr, __FUNCTION__, __LINE__, _ret); \
            return _ret;                               \
        }                                              \
    } while (0)

DispMdl *DispMdl::instance_ = NULL;

DispMdl *DispMdl::Instance()
{
    if (NULL == instance_)
    {
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

    stVbConf.u32MaxPoolCnt = 1;
    stVbConf.astCommPool[0].u32BlkSize = VB_BLK_SIZE;
    stVbConf.astCommPool[0].u32BlkCnt = VB_BLK_CNT;

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
    stPubAttr.stSyncInfo.u16Hpw = VO_H_PULSE_WIDTH;
    stPubAttr.stSyncInfo.u16Hbb = VO_H_BACK_PORCH;
    stPubAttr.stSyncInfo.u16Hact = VO_H_ACTIVE;
    stPubAttr.stSyncInfo.u16Hfb = VO_H_FRONT_PORCH;
    stPubAttr.stSyncInfo.u16Vpw = VO_V_PULSE_WIDTH;
    stPubAttr.stSyncInfo.u16Vbb = VO_V_BACK_PORCH;
    stPubAttr.stSyncInfo.u16Vact = VO_V_ACTIVE;
    stPubAttr.stSyncInfo.u16Vfb = VO_V_FRONT_PORCH;

    CHECK_MPI(XM_MPI_VO_SetPubAttr(VO_DEV_ID, &stPubAttr));
    LOGI("VO PubAttr ok: user sync, %dx%d, H(pw%d bb%d act%d fb%d) V(pw%d bb%d act%d fb%d)",
         SCREEN_HOR_RES, SCREEN_VER_RES,
         VO_H_PULSE_WIDTH, VO_H_BACK_PORCH, VO_H_ACTIVE, VO_H_FRONT_PORCH,
         VO_V_PULSE_WIDTH, VO_V_BACK_PORCH, VO_V_ACTIVE, VO_V_FRONT_PORCH);
#endif

    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    memset(&stLayerAttr, 0, sizeof(stLayerAttr));

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stDispRect.u32Width = SCREEN_HOR_RES;
    stLayerAttr.stDispRect.u32Height = VO_LAYER_HEIGHT;
    stLayerAttr.stImageSize.u32Width = SCREEN_HOR_RES;
    stLayerAttr.stImageSize.u32Height = VO_LAYER_HEIGHT;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    CHECK_MPI(XM_MPI_VO_SetVideoLayerAttr(VO_LAYER_ID, &stLayerAttr));
    LOGI("VO LayerAttr ok: %dx%d, YUV420SP", SCREEN_HOR_RES, VO_LAYER_HEIGHT);

    VO_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    stChnAttr.stRect.s32X = 0;
    stChnAttr.stRect.s32Y = 0;
    stChnAttr.stRect.u32Width = SCREEN_HOR_RES;
    stChnAttr.stRect.u32Height = SCREEN_VER_RES;

    CHECK_MPI(XM_MPI_VO_SetChnAttr(VO_LAYER_ID, VO_CHN_ID, &stChnAttr));
    CHECK_MPI(XM_MPI_VO_EnableChn(VO_LAYER_ID, VO_CHN_ID));
    CHECK_MPI(XM_MPI_VO_ShowChn(VO_LAYER_ID, VO_CHN_ID));
    LOGI("VO Chn%d ok: full screen %dx%d", VO_CHN_ID, SCREEN_HOR_RES, SCREEN_VER_RES);

    LOGI("VoInit done");
    return XM_SUCCESS;
}

int DispMdl::VpssInit()
{
    LOGI("VpssInit begin");
    VPSS_GRP_ATTR_S stGrpAttr;
    memset(&stGrpAttr, 0, sizeof(stGrpAttr));
    stGrpAttr.u32MaxW = VPSS_MAX_WIDTH;
    stGrpAttr.u32MaxH = VPSS_MAX_HEIGHT;
    CHECK_MPI(XM_MPI_VPSS_CreateGrp(VPSS_GRP_ID, &stGrpAttr));
    LOGI("VPSS CreateGrp%d ok: max %dx%d",
         VPSS_GRP_ID, VPSS_MAX_WIDTH, VPSS_MAX_HEIGHT);
    VPSS_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    stChnAttr.u32Width = VIDEO_HOR_RES;
    stChnAttr.u32Height = VIDEO_VER_RES;
    stChnAttr.stFrameRate.s32SrcFrameRate = VIDEO_FRAME_RATE;
    stChnAttr.stFrameRate.s32DstFrameRate = VIDEO_FRAME_RATE;
    stChnAttr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stChnAttr.bMirror = XM_FALSE;
    stChnAttr.bFlip = XM_FALSE;
    CHECK_MPI(XM_MPI_VPSS_SetChnAttr(VPSS_GRP_ID, VPSS_CHN_ID, &stChnAttr));
    LOGI("VPSS Chn%d ok: out %dx%d @%dfps (no scaling, VO will scale to %dx%d)",
         VPSS_CHN_ID, VIDEO_HOR_RES, VIDEO_VER_RES, VIDEO_FRAME_RATE,
         SCREEN_HOR_RES, SCREEN_VER_RES);

    LOGI("VpssInit done");
    return XM_SUCCESS;
}

int DispMdl::VdecInit()
{
    LOGI("VdecInit begin");

    VDEC_DEV_ATTR_S stDevAttr;
    memset(&stDevAttr, 0, sizeof(stDevAttr));

    stDevAttr.u32PicWidth = VDEC_MAX_WIDTH;
    stDevAttr.u32PicHeight = VDEC_MAX_HEIGHT;
    stDevAttr.u32BufSize = VDEC_BUF_SIZE;
    stDevAttr.enType = PT_H265;

    stDevAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
    stDevAttr.stVdecVideoAttr.u32RefFrame = VDEC_REF_FRAME_NUM;

    CHECK_MPI(XM_MPI_VDEC_CreateDev(VDEC_DEV_ID, &stDevAttr));
    LOGI("VDEC CreateDev%d ok: H.265, max %dx%d, bufsize %u KB, refframe %d",
         VDEC_DEV_ID, VDEC_MAX_WIDTH, VDEC_MAX_HEIGHT,
         (unsigned)(VDEC_BUF_SIZE / 1024), VDEC_REF_FRAME_NUM);

    VDEC_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0, sizeof(stChnAttr));

    stChnAttr.u32OutWidth = VIDEO_HOR_RES;
    stChnAttr.u32OutHeight = VIDEO_VER_RES;

    CHECK_MPI(XM_MPI_VDEC_SetChnAttr(VDEC_DEV_ID, VDEC_CHN_ID, &stChnAttr));
    LOGI("VDEC Chn%d ok: out %dx%d", VDEC_CHN_ID, VIDEO_HOR_RES, VIDEO_VER_RES);

    CHECK_MPI(XM_MPI_VDEC_StartRecvStream(VDEC_DEV_ID));
    CHECK_MPI(XM_MPI_VDEC_EnableChn(VDEC_DEV_ID, VDEC_CHN_ID));
    LOGI("VDEC StartRecvStream + EnableChn ok");

    wait_i_frame_ = true;

    LOGI("VdecInit done");
    return XM_SUCCESS;
}

int DispMdl::Bind()
{
    LOGI("Bind begin");
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDstChn;
    memset(&stSrcChn, 0, sizeof(stSrcChn));
    memset(&stDstChn, 0, sizeof(stDstChn));

    stSrcChn.enModId = XM_ID_VDEC;
    stSrcChn.s32DevId = VDEC_DEV_ID;
    stSrcChn.s32ChnId = VDEC_CHN_ID;

    stDstChn.enModId = XM_ID_VPSS;
    stDstChn.s32DevId = VPSS_GRP_ID;
    stDstChn.s32ChnId = VPSS_CHN_ID;

    CHECK_MPI(XM_MPI_SYS_Bind(&stSrcChn, &stDstChn));
    LOGI("Bind VDEC(dev%d,chn%d) -> VPSS(grp%d,chn%d)",
         VDEC_DEV_ID, VDEC_CHN_ID, VPSS_GRP_ID, VPSS_CHN_ID);

    memset(&stSrcChn, 0, sizeof(stSrcChn));
    memset(&stDstChn, 0, sizeof(stDstChn));

    stSrcChn.enModId = XM_ID_VPSS;
    stSrcChn.s32DevId = VPSS_GRP_ID;
    stSrcChn.s32ChnId = VPSS_CHN_ID;

    stDstChn.enModId = XM_ID_VOU;
    stDstChn.s32DevId = VO_DEV_ID;
    stDstChn.s32ChnId = VO_CHN_ID;

    CHECK_MPI(XM_MPI_SYS_Bind(&stSrcChn, &stDstChn));
    LOGI("Bind VPSS(grp%d,chn%d) -> VOU(dev%d,chn%d)",
         VPSS_GRP_ID, VPSS_CHN_ID, VO_DEV_ID, VO_CHN_ID);
    CHECK_MPI(XM_MPI_VPSS_StartGrp(VPSS_GRP_ID));
    CHECK_MPI(XM_MPI_VPSS_EnableChn(VPSS_GRP_ID, VPSS_CHN_ID));
    LOGI("VPSS Grp%d started, Chn%d enabled", VPSS_GRP_ID, VPSS_CHN_ID);

    LOGI("Bind done");
    return XM_SUCCESS;
}

int DispMdl::Unbind()
{
    LOGI("Unbind begin");

    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDstChn;

    XM_MPI_VPSS_DisableChn(VPSS_GRP_ID, VPSS_CHN_ID);
    XM_MPI_VPSS_StopGrp(VPSS_GRP_ID);

    memset(&stSrcChn, 0, sizeof(stSrcChn));
    memset(&stDstChn, 0, sizeof(stDstChn));
    stSrcChn.enModId = XM_ID_VPSS;
    stSrcChn.s32DevId = VPSS_GRP_ID;
    stSrcChn.s32ChnId = VPSS_CHN_ID;
    stDstChn.enModId = XM_ID_VOU;
    stDstChn.s32DevId = VO_DEV_ID;
    stDstChn.s32ChnId = VO_CHN_ID;
    XM_MPI_SYS_UnBind(&stSrcChn, &stDstChn);

    memset(&stSrcChn, 0, sizeof(stSrcChn));
    memset(&stDstChn, 0, sizeof(stDstChn));
    stSrcChn.enModId = XM_ID_VDEC;
    stSrcChn.s32DevId = VDEC_DEV_ID;
    stSrcChn.s32ChnId = VDEC_CHN_ID;
    stDstChn.enModId = XM_ID_VPSS;
    stDstChn.s32DevId = VPSS_GRP_ID;
    stDstChn.s32ChnId = VPSS_CHN_ID;
    XM_MPI_SYS_UnBind(&stSrcChn, &stDstChn);

    LOGI("Unbind done");
    return XM_SUCCESS;
}

int DispMdl::Start()
{
    int ret;

    if (started_)
    {
        LOGE("already started, ignore");
        return -1;
    }

    LOGI("########## DispMdl Start ##########");

    ret = SysInit();
    if (XM_SUCCESS != ret)
        return ret; /* ① */
    ret = VoInit();
    if (XM_SUCCESS != ret)
        return ret; /* ② */
    ret = VpssInit();
    if (XM_SUCCESS != ret)
        return ret; /* ③ */
    ret = VdecInit();
    if (XM_SUCCESS != ret)
        return ret; /* ④ */
    ret = Bind();
    if (XM_SUCCESS != ret)
        return ret; /* ⑤ */

    started_ = true;
    LOGI("########## DispMdl Start OK ##########");
    return XM_SUCCESS;
}

int DispMdl::Stop()
{
    if (!started_)
    {
        LOGI("not started, nothing to stop");
        return XM_SUCCESS;
    }

    LOGI("########## DispMdl Stop ##########");

    Unbind();

    XM_MPI_VDEC_StopRecvStream(VDEC_DEV_ID);
    XM_MPI_VDEC_DisableChn(VDEC_DEV_ID, VDEC_CHN_ID);
    XM_MPI_VDEC_DestroyDev(VDEC_DEV_ID);

    XM_MPI_VPSS_DestroyGrp(VPSS_GRP_ID);

    XM_MPI_VO_HideChn(VO_LAYER_ID, VO_CHN_ID);
    XM_MPI_VO_DisableChn(VO_LAYER_ID, VO_CHN_ID);

    XM_MPI_VB_Exit();

    started_ = false;
    LOGI("########## DispMdl Stop OK ##########");
    return XM_SUCCESS;
}

int DispMdl::SendFrame(const unsigned char *data, unsigned int len, bool key_frame)
{
    if (!started_)
    {
        return -1;
    }
    if (NULL == data || 0 == len)
    {
        return -1;
    }
    if (wait_i_frame_)
    {
        if (!key_frame)
        {
            return -1;
        }
        wait_i_frame_ = false;
        LOGI("first I frame arrived, start decoding");
    }

    VDEC_STREAM_S stStream;
    memset(&stStream, 0, sizeof(stStream));

    stStream.pu8Addr = (XM_U8 *)data;
    stStream.u32Len = len;
    stStream.u64PTS = 0;

    XM_S32 ret = XM_MPI_VDEC_SendStream(VDEC_DEV_ID, &stStream, 0);
    if (XM_SUCCESS != ret)
    {
        LOGE("VDEC_SendStream failed, ret=%#x, len=%u", ret, len);
        if (VDEC_ERR_BUF_FULL == ret)
        {
            ResetDecoder();
        }
        return -1;
    }
    return 0;
}

void DispMdl::ResetDecoder()
{
    LOGE("reset VDEC dev%d", VDEC_DEV_ID);

    XM_MPI_VDEC_StopRecvStream(VDEC_DEV_ID);

    XM_S32 ret = XM_MPI_VDEC_ResetDev(VDEC_DEV_ID);
    if (XM_SUCCESS != ret)
    {
        LOGE("VDEC_ResetDev failed, ret=%#x", ret);
    }

    XM_MPI_VDEC_StartRecvStream(VDEC_DEV_ID);
    wait_i_frame_ = true;
}