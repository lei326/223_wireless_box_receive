#pragma once

#ifdef WIN32
#ifdef API_EXPORTS
#define XM_MIDDLEWARE_API extern "C" __declspec(dllexport)
#else
#define XM_MIDDLEWARE_API extern "C" __declspec(dllimport)
#endif
#define CALLMETHOD __stdcall
#else
#define XM_MIDDLEWARE_API 
#define CALLMETHOD 
#endif

#include "xm_middleware_def.h"
#include <unordered_map>

using namespace std;
/*------------------------全局-----------------------------*/
//设置通道数，默认为XM_MAX_CHANNEL_NUM。该接口需要在XM_Middleware_Init之前调用
XM_MIDDLEWARE_API int XM_Middleware_SetChannelNum(int channel_num);
XM_MIDDLEWARE_API int XM_Middleware_Init();
XM_MIDDLEWARE_API const char* XM_Middleware_GetVersion();
XM_MIDDLEWARE_API int XM_Middleware_Exit();
XM_MIDDLEWARE_API int XM_Middleware_SetLogLevel(XMLogLevel level);
XM_MIDDLEWARE_API int XM_Middleware_InitDebugger();

/*----------------------设置回调---------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_SetGetFrehopPermitCallback(XM_GetFrehopPermit_CallBack proc);
XM_MIDDLEWARE_API int XM_Middleware_SetRealStreamCallback(XM_MW_RealStream_CallBack proc, int64_t user);
XM_MIDDLEWARE_API int XM_Middleware_SetVodStreamCallback(XM_MW_VodStream_CallBack proc, int64_t user);
XM_MIDDLEWARE_API int XM_Middleware_SetEventCallback(XM_MW_Event_CallBack proc, int64_t user);
XM_MIDDLEWARE_API int XM_Middleware_SetLogCallback(XM_MW_LOG_CallBack proc, int64_t user);

/*------------------------存储-----------------------------*/
//设置sd卡mount路径
XM_MIDDLEWARE_API int XM_Middleware_Storage_SetSDCardConfig(XM_MW_Storage_SDCard_Config* sdcard_cfg);
//获取速度状态
XM_MIDDLEWARE_API int XM_Middleware_Storage_GetSpeedState(XMStorageSDCardNum sdc_num, int* write_speed);
//写速度到文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_WriteSpeedToFile(XMStorageSDCardNum sdc_num, int* write_speed);
//检测sd卡上的文件系统
XM_MIDDLEWARE_API int XM_Middleware_Storage_CheckSDCard(XMStorageSDCardNum sdc_num);
//创建文件系统
XM_MIDDLEWARE_API int XM_Middleware_Storage_CreateFileSystem(XMStorageSDCardNum sdc_num);
//检测sd卡上的文件系统
XM_MIDDLEWARE_API int XM_Middleware_Storage_CheckFileSystem(XMStorageSDCardNum sdc_num);
//查询sd卡容量
XM_MIDDLEWARE_API int XM_Middleware_Storage_CheckSDSpace(XMStorageSDCardNum sdc_num, XM_MW_SDCard_Space* sdcard_space);
//挂载sd卡
XM_MIDDLEWARE_API int XM_Middleware_Storage_MountSDCard(XMStorageSDCardNum sdc_num);
//格式化sd卡
XM_MIDDLEWARE_API int XM_Middleware_Storage_FormatSDCard(XMStorageSDCardNum sdc_num);
//设置录像时长，1、2、3分钟
XM_MIDDLEWARE_API int XM_Middleware_Storage_SetRecordDuration(XMStorageRecordDuration record_duration);
//设置停车监控唤醒后紧急视频时长，默认30秒
XM_MIDDLEWARE_API int XM_Middleware_Storage_SetAlarmRecordDuration(XMStorageAlarmRecordDuration dura);
//指定视频通道存储到哪张sd卡
XM_MIDDLEWARE_API int XM_Middleware_Storage_BindSDCardNum(int channel, XMStorageSDCardNum sdc_num);
//设置存储媒体格式，只针对pc版本有效，板子上会自动从mpp模块获取
XM_MIDDLEWARE_API int XM_Middleware_Storage_SetMediaInfo(int channel, XM_MW_Media_Info* media_info);
//开始录像/停止录像, channel为-1，表所有通道开启和关闭。刚插上后拉时，画面不稳定，delay_time用于延迟存储，过滤异常画面
//recorder_type == 0 表示标准录像，recorder_type == 1 表示报警录像
XM_MIDDLEWARE_API int XM_Middleware_Storage_Start(int channel, int recorder_type, int before_time, int after_time, int delay_time);
//报警录像，before_time表示提前多久录制，after_time表示报警后还需录制多久
XM_MIDDLEWARE_API int XM_Middleware_Storage_AlarmRecord(int channel);
//碰撞开机录像
XM_MIDDLEWARE_API int XM_Middleware_Storage_CollisionRecord(int channel);
//录音使能
XM_MIDDLEWARE_API int XM_Middleware_Storage_EnableRecordAudio(bool enable);
//AddVideoFrame\AddAudioFrame\AddPicture只针对pc版本有效，板子上会自动从mpp模块取流
XM_MIDDLEWARE_API int XM_Middleware_Storage_AddVideoFrame(int channel, bool key_frame, uint8_t* data, int len, int64_t pts);
XM_MIDDLEWARE_API int XM_Middleware_Storage_AddAudioFrame(int channel, uint8_t* data, int len, int64_t pts);
XM_MIDDLEWARE_API int XM_Middleware_Storage_AddPicture(int channel, uint8_t* data, int len);
XM_MIDDLEWARE_API int XM_Middleware_Storage_Stop(int channel);
XM_MIDDLEWARE_API bool XM_Middleware_Storage_ReallyStop();
//锁定文件，关键录像，不允许被覆盖。锁定历史录像文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_LockHistoryFile(XM_MW_File_Param* file_param);
//锁定所有文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_LockAllHistoryFile(XMStorageSDCardNum sdc_num, int channel, XMRecordType type);
//解锁历史文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_UnLockHistoryFile(XM_MW_File_Param* file_param);
//解锁所有文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_UnLockAllHistoryFile(XMStorageSDCardNum sdc_num, int channel, XMRecordType type);
//锁定当前正在录的一个录像文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_LockCurrentFile(XMStorageSDCardNum sdc_num, int channel, bool lock_file);
//设置加锁文件空间占比
XM_MIDDLEWARE_API int XM_Middleware_Storage_SetLockFilePercent(XM_LOCKFILE_PERCENT lockfile_percent);
//删除文件，删除前需停止播放
XM_MIDDLEWARE_API int XM_Middleware_Storage_DelFile(XM_MW_File_Param* file_param);
//删除所有文件
XM_MIDDLEWARE_API int XM_Middleware_Storage_DelAllFile(XMStorageSDCardNum sdc_num, int channel, XMRecordType type);
//获取当前录像文件已录的时间
XM_MIDDLEWARE_API int XM_Middleware_Storage_GetRecordedTime(XMStorageSDCardNum sdc_num, int channel);
//设置循环录像
XM_MIDDLEWARE_API int XM_Middleware_Storage_SetRecordLoop(int chnum, bool bLoop);

/*------------------------回放-----------------------------*/
//mtv播放使能。解码mtv时通知到sdk，sdk内部关闭一些功能，释放更多资源来做mtv软解
XM_MIDDLEWARE_API int XM_Middleware_Playback_MTV(bool enable);
//录像(包括视频和图片)查询
XM_MIDDLEWARE_API int XM_Middleware_Playback_SetUSBPath(const char* dir_path);
XM_MIDDLEWARE_API int XM_Middleware_Playback_Query(XM_MW_Record_Query* query_info, XM_MW_Record_List* record_list);
//打开视频录像
XM_MIDDLEWARE_API long XM_Middleware_Playback_Open(XMStorageSDCardNum sdc_num, int channel, XMFileFormat file_format, 
	const char* file_full_name);
//获取录像文件信息
XM_MIDDLEWARE_API int XM_Middleware_Playback_GetFileInfo(long handle, XM_MW_File_Info* file_info);
//display_region是渲染区域的位置和大小。play_one为true时只播放一帧
XM_MIDDLEWARE_API int XM_Middleware_Playback_Play(long handle, bool play_one, RECT_S* display_region);
//seek_time，单位毫秒
XM_MIDDLEWARE_API int XM_Middleware_Playback_Seek(long handle, int seek_time);
//pause true表暂停，false表恢复
XM_MIDDLEWARE_API int XM_Middleware_Playback_Pause(long handle, bool pause);
//获取到当前播放进度，单位毫秒
XM_MIDDLEWARE_API int XM_Middleware_Playback_GetCurrentDuration(long handle);
//关闭录像文件
XM_MIDDLEWARE_API int XM_Middleware_Playback_Close(long handle);
//挂载U盘
XM_MIDDLEWARE_API int XM_Middleware_Playback_CheckUDisk();
//格式化u盘
XM_MIDDLEWARE_API int XM_Middleware_Playback_FormatUDisk();
//卸载U盘
XM_MIDDLEWARE_API int XM_Middleware_Playback_UmountUDisk();
//远程回放，增加视频
XM_MIDDLEWARE_API int XM_Middleware_Playback_AddRemoteVideoFrame(long handle, bool key_frame, uint8_t* data, int len, int64_t pts);
//远程回放，增加音频
XM_MIDDLEWARE_API int XM_Middleware_Playback_AddRemoteAudioFrame(long handle, uint8_t* data, int len, int64_t pts);
/*------------------------算法-----------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_Smart_AIDecode(int channel, unsigned char* data, int len, int type);
//设置各通道ai算法类型，默认XM_AI_TYPE_BSD
XM_MIDDLEWARE_API int XM_Middleware_Smart_SetAIType(int channel, XMAIType ai_type);
//设置adas参数
XM_MIDDLEWARE_API int XM_Middleware_Smart_SetAdasParam(int channel, XM_MW_ADAS_Param* adas_param);
//设置bsd参数
XM_MIDDLEWARE_API int XM_Middleware_Smart_SetBsdParam(int channel, XM_MW_BSD_Param* bsd_param);
//ai使能
XM_MIDDLEWARE_API int XM_Middleware_Smart_EnableAI(int channel, bool enable);
//声控使能
XM_MIDDLEWARE_API int XM_Middleware_Smart_EnableKeyword(bool enable);
//算法结果处理
XM_MIDDLEWARE_API int XM_Middleware_Smart_ProcessRst(int Channel, bool show, const XM_IA_TCP_RESULT_S *stTdRlt);

/*------------------------报警-----------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_Alarm_AlarmStart();

/*----------------------看门狗---------------------------*/
//sec是看门狗超时时间，默认3秒
XM_MIDDLEWARE_API int XM_Middleware_WDT_WatchDogStart(int sec);
XM_MIDDLEWARE_API int XM_Middleware_WDT_WatchDogStop();
XM_MIDDLEWARE_API int XM_Middleware_WDT_WatchDogStopFeed();
//注册用户态看门狗线程，timeout_msec是线程的超时时间，单位毫秒
XM_MIDDLEWARE_API int XM_Middleware_WDT_RegisterThread(const string& name, int timeout_msec, void* user_data);
//更新用户态看门狗线程的心跳
XM_MIDDLEWARE_API int XM_Middleware_WDT_UpdateHeartbeat(const string& thread_name);

/*-----------------------SD卡升级------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_SDCard_Upgrade(const char* file_name, const char* md5_name);
XM_MIDDLEWARE_API int XM_Middleware_SDCard_GetMD5(const char* file_name, char* md5_out);
XM_MIDDLEWARE_API int XM_Middleware_SDCard_AlreadyUpgrade(const char* file_name, const char* md5_name);
XM_MIDDLEWARE_API int XM_Middleware_SDCard_GetUpgradeProgress();
XM_MIDDLEWARE_API int XM_Middleware_SDCard_CompareMd5Value(const char* path, const char* md5_value);
XM_MIDDLEWARE_API int XM_Middleware_SDCard_Reboot();

/*-----------------------外设事件通知---------------------*/
XM_MIDDLEWARE_API int XM_Middleware_Periphery_Notify(XMEventType event_type, const char* msg, int param);

/*-----------------------网络配网----------------------------*/
//查找ip配网
XM_MIDDLEWARE_API int XM_Middleware_NET_SearchIps(int ch_num, unordered_map<string, string>& searchIpMap);
//写入最初的白名单
XM_MIDDLEWARE_API int XM_Middleware_NET_WriteInitialWhiteList(int ch_num);
//删除白名单
XM_MIDDLEWARE_API int XM_Middleware_NET_DelWhiteList(int chn);
//路由中转升级
XM_MIDDLEWARE_API int XM_Middleware_NET_Routeswitch(char* search_ip);
//停止查找ip
XM_MIDDLEWARE_API int XM_Middleware_NET_StopSearchIp();
//wifi信号强度
XM_MIDDLEWARE_API int XM_Middleware_NET_GetIpcWifiLevel(char * ifname);
//生成二维码
XM_MIDDLEWARE_API int XM_Middleware_NET_GenerateQrCode(char * info);
//获取wifi信息并生成二维码
XM_MIDDLEWARE_API int XM_Middleware_NET_GetWifiInfo();
//校验Key
XM_MIDDLEWARE_API int XM_Middleware_NET_CheckRecvKey(char* key);
//获取通道ip
XM_MIDDLEWARE_API int XM_Middleware_NET_GetIpJson(int ch_num, char* getIp);
//wifi连接使能
XM_MIDDLEWARE_API int XM_Middleware_NET_WLanEnablePairing(int bEnable);
//检查驱动层mac是否连上
XM_MIDDLEWARE_API bool XM_Middleware_NET_CheckMacConnectStatus(int channel);
//低功耗唤醒ipc（-1 唤醒全部）
XM_MIDDLEWARE_API int XM_Middleware_NET_WakeUp(int channel);
//获取mac地址（只获取最后2位）
XM_MIDDLEWARE_API int XM_Middleware_NET_GetNvrMac(unsigned char* mac);
//跳频使能
XM_MIDDLEWARE_API int XM_Middleware_NET_FrehopEnable(int bEnable);
//使能非标  （0：标准信道 1：特殊信道 2：先标准后特殊 3：先特殊后标准 ）
//SDK默认打开开关 3 ：先特殊后标准   关闭开关 0：标准信道
XM_MIDDLEWARE_API int XM_Middleware_NET_SetWifiFreSpecial(int bEnable);
//前三最佳信道
XM_MIDDLEWARE_API int XM_Middleware_NET_SetBestChannel(int channel, int channel1, int channel2, int channel3);
//WIFI信号强度
XM_MIDDLEWARE_API int XM_Middleware_NET_GetWifiLevel(int channel, int level);
//设备间数量最小信道
XM_MIDDLEWARE_API int XM_Middleware_NET_SetMinChannel(int channel);
//更新连接状态
XM_MIDDLEWARE_API int XM_Middleware_NET_UpdateConnectStatus(int channel, bool bConnect);
//更新预览开启状态状态
XM_MIDDLEWARE_API int XM_Middleware_NET_UpdatePreviewEnable(int channel, bool Enable);
//获取当前信道
XM_MIDDLEWARE_API int XM_Middleware_NET_GetCurrentFrehopChannel();
/*-----------------------system----------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_SafeSystem(const char* pCmdLine);
//打印内存信息
XM_MIDDLEWARE_API int XM_Middleware_GetMemoryMsg(uint32_t& used_memory, uint32_t& available_memory);
//检测内存泄漏
XM_MIDDLEWARE_API int XM_Middleware_DumpMemoryLeaks();


/*-----------------------图片----------------------------*/
//解码渲染图片
XM_MIDDLEWARE_API int XM_Middleware_Picture_Play(const char* file_full_name, int file_len, RECT_S* display_region);
/*-----------------------AAC----------------------------*/
//aac初始化
XM_MIDDLEWARE_API int XM_Middleware_AAC_AACInit();
//aac解码
XM_MIDDLEWARE_API int XM_Middleware_AAC_Decode(uint8_t* data, int size, uint8_t* out_buffer, int* out_len);

/*-----------------------媒体推流------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_Media_VideoStream(int channel, int stream, XM_MW_Media_Frame* media_frame, int width, int height);
XM_MIDDLEWARE_API int XM_Middleware_Media_AudioStream(int ai_dev, int channel, XM_MW_Media_Frame* media_frame);
XM_MIDDLEWARE_API int XM_Middleware_Media_VideoThumbnail(int channel, char* yuv, int width, int height);
/*-----------------------uart2----------------------------*/
XM_MIDDLEWARE_API int XM_Middleware_MD5CalcByFile(const char* filename, char *outStr, int outLen);
XM_MIDDLEWARE_API int XM_Middleware_SetAdcCalibrationValue(int value);
