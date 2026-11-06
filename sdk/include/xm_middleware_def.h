#pragma once

#include <stdint.h>
#include "xm_common.h"				/*复用mmp中对编解码类型的定义PAYLOAD_TYPE_E*/
#include "xm_comm_vo.h"				/*复用mmp中区域的定义RECT_S*/
#include "xm_ia_rule.h"				/*复用smart中的定义XM_IA_RULE_REGION_S*/
#include "xm_ia_adas_interface.h"	/*复用smart中的定义XM_ADAS_CAM_CAL_S*/
#include "xm_ia_comm.h"				/*复用smart中的定义sensitivity*/
#include "xm_ia_td_interface.h"

#ifndef WIN32
// #include "cr_usb.h"					/*复用usb中的定义USB_MODE_E*/
// #include "dataAssist.h"
#endif
/*------------------------------define-----------------------------*/
#define XM_MAX_STORAGE_PATH_LEN 256
#define XM_MAX_STORAGE_FILE_LEN 36
#define XM_MAX_CHANNEL_NUM 4
#define XM_MAX_RECORD_NUM 6000	//一张sd卡上的所有通道摄像头，最多允许6000个录像文件
#define XM_MAX_WIFI_PASSWORD_LEN 23
#define XM_MAX_OSD_WIDTH 320
#define XM_MAX_OSD_HEIGHT 128

/*------------------------------enum-------------------------------*/	
//SD卡号，最多支持两张SD卡
typedef enum _XMStorageSDCardNum
{
	XM_STORAGE_SDCard_0 = 0,
	XM_STORAGE_SDCard_1,
	XM_STORAGE_USB,				//U盘音频和视频，只读
	XM_STORAGE_SDCard_TotalNum
}XMStorageSDCardNum;

typedef enum _XMStorageRecordDuration
{
	//录像单文件，默认1分钟(60秒)
	XM_STORAGE_Record_Duration_60 = 0,
	XM_STORAGE_Record_Duration_120,
	XM_STORAGE_Record_Duration_180,
	XM_STORAGE_Record_Duration_300 = 5,
	XM_STORAGE_Record_Duration_600 = 10, 
	XM_STORAGE_Record_Duration_1800 = 30
}XMStorageRecordDuration;

typedef enum _XMStorageAlarmRecordDuration
{
	XM_STORAGE_AlarmRecord_Duration_30 = 0,
	XM_STORAGE_AlarmRecord_Duration_15
}XMStorageAlarmRecordDuration;

typedef enum _XMEventType
{
	XM_EVENT_Play_Over = 0,
	XM_EVENT_DEV_ADD ,
	XM_EVENT_DEV_REMOVE ,
	XM_EVENT_USB_ADD ,
	XM_EVENT_USB_REMOVE ,
	XM_EVENT_LOCALALARM_START,
	XM_EVENT_LOCALALARM_STOP,
	XM_EVENT_UPGRADE_PROGRESS,
	XM_EVENT_PLAY_First_I_frame,			//回放出第一帧数据
	XM_EVENT_RECORD_END,					//录完一个视频或录像结束
	XM_EVENT_SDCARD_NOENOUGH_SPACE,
	XM_EVENT_CAMERA_CONNECT,
	XM_EVENT_CAMERA_DISCONNECT,
	XM_EVENT_CAMERA_BATTERY,
	XM_EVENT_CAMERA_RATE,
	XM_EVENT_DECODE_ENABLE,						//开启关闭解码器时钟
	XM_EVENT_CAMERA_UPGRADER,
	XM_EVENT_BSD_START,
	XM_EVENT_DEV_ADD_UI,
	XM_EVENT_RETURN_FILE_RESULT,        //前端获取录像文件失败
	XM_EVENT_RETURN_FILE_DUR,			//前端返回回放文件时长
	XM_EVENT_UPDATE_SD_MSG,				//前端更新sd卡信息
	XM_EVENT_CAMERA_CFG_INIT,			//前端刚连上后端发送配置，后端要更新配置信息
	XM_EVENT_UPDATE_DN_STATE,
	XM_EVENT_BABY_CRY,
	XM_EVENT_SOUND_ALARM,
	XM_EVENT_HIGH_TEMPERATURE,
	XM_EVENT_LOW_TEMPERATURE,
	XM_EVENT_TEMPERATURE_VALUE,
	XM_EVENT_HUMIDITY_VALUE,
	XM_EVENT_DMS_STATE,
	XM_EVENT_SDCARD_READ_ONLY,				//SD卡只读
	XM_EVENT_SDCARD_FSCK_START,				//SD卡开始修复
	XM_EVENT_SDCARD_FSCK_STOP,				//SD卡结束修复
	XM_EVENT_SDCARD_DAMAGED,				//SD卡损坏

	//网络回调
	XM_EVENT_CHANGE_TIME,
	XM_EVENT_RECORD_START,
	XM_EVENT_RECORD_CLOSE,
	XM_EVENT_RECORD_SOUND_ENABLE,
	XM_EVENT_RECORDFILE_START,
	XM_EVENT_RECORDFILE_END,
	XM_EVENT_VOICE_CONTROL_ENABLE,
	XM_EVENT_SDCARD_FORMAT,
	XM_EVENT_CAMERA_SWITCH,
	XM_EVENT_CFG_RESET,
	XM_EVENT_CHANGE_RESOLUTION,
	XM_EVENT_REC_SPLIT_DUR,
	XM_EVENT_GSR_SENSITIVITY,
	XM_EVENT_SCREEN_STANDBY,
	XM_EVENT_PARKING_MONITOR,
	XM_EVENT_TIMELAPSE_RATE,
	XM_EVENT_OSD_ENABLE,
	XM_EVENT_CHANGE_WIFI_PARAM,
	XM_EVENT_NETWORK_MSG,
	XM_EVENT_KEYTONE_ENABLE,
	XM_EVENT_PARK_RECORD_TIME,
	XM_EVENT_APP_PAGE,
	XM_EVENT_BAT_INFO,
	//以下是语音识别
	XM_EVENT_KEYWORD_XIANSHIQIANLU = 100, 	//显示前路/打开前路
	XM_EVENT_KEYWORD_XIANSHIHOULU, 			//显示后路/打开后路
	XM_EVENT_KEYWORD_CHAKANQUANBU, 			//查看全部
	XM_EVENT_KEYWORD_DAKAIPINGMU, 			//打开屏幕
	XM_EVENT_KEYWORD_GUANBIPINGMU, 			//关闭屏幕
	XM_EVENT_KEYWORD_ZHUAPAIZHAOPIAN, 		//抓拍照片/我要拍照
	XM_EVENT_KEYWORD_DAKAILUYIN, 			//打开录音
	XM_EVENT_KEYWORD_GUANBILUYIN, 			//关闭录音
	XM_EVENT_KEYWORD_JINJILUXIANG, 			//紧急录像
	XM_EVENT_KEYWORD_WOYAOLUXIANG,			//我要录像
	XM_EVENT_KEYWORD_DAKAIREDIAN,			//打开热点
	XM_EVENT_KEYWORD_GUANBIREDIAN,			//关闭热点
	//单片机消息
	XM_EVENT_VOLTAGE_LOW,					//电压过低，可能从电源供电切换成电池供电，外部停止录像后开机
	XM_EVENT_KEY_SHUTDOWN,					//收到关机键命令
	XM_EVENT_USB_DISCONNECT,				//断开USB
	XM_EVENT_USB_CONNECT,                   //连接USB
	XM_EVENT_USB_UNSTABLE,					//USB不稳定
	XM_EVENT_USB_STABLE,					//USB恢复稳定
	XM_EVENT_ACC_DISCONNECT,				//ACC断开
	XM_EVENT_ACC_CONNECT,					//ACC连接
	XM_EVENT_POWERKEY_SHORTPRESS,			//短按电源键
	XM_EVENT_COLLISION_DETECTED,			//检测到碰撞
	XM_EVENT_REVERSE_DETECTED,				//检测到倒车
	XM_EVENT_REVERSE_END,					//倒车结束
	XM_EVENT_FREHOP,
	XM_EVENT_DEVICE_FREHOP_START,
	XM_EVENT_DEVICE_FREHOP_END,
	XM_EVENT_FREHOP_RLS,
	XM_EVENT_FRAME_TIMEOUT,
	XM_EVENT_PROCESS_IFRAME,
	XM_EVENT_NET_STATUS,
	XM_EVENT_VERIFY,
	XM_EVENT_KEY_PRESS,
	
	//后拉被拔出
	XM_EVENT_AD_EXIT,
	//后拉插上
	XM_EVENT_AD_CONNECT,

	//远程录像回放
	XM_EVENT_REMOTE_PLAYBACK,
	XM_EVENT_REMOTE_PLAYBACK_CLOSE,
	XM_EVENT_REMOTE_PLAYBACK_SEEK,
	//以下是bsd报警
	XM_EVENT_BSD_ALARM = 200,				//BSD报警
	XM_EVENT_BSD_ALARM_REFRESH_UI,

	//喇叭使能
	XM_EVENT_SPEAKER_ENABLE = 300,		
	XM_EVENT_WIFITYPE_INCONFORMITY,
	XM_EVENT_MD5_GET,	
}XMEventType;

typedef enum _XMRecordType
{
	XM_Record_Video = 0,
	XM_Record_Picture,
	XM_Record_MP3, //标准音频
	XM_Record_MTV, //标准视频
}XMRecordType;

typedef enum _XMAIType
{
	XM_AI_TYPE_ADAS = 0,
	XM_AI_TYPE_BSD,
	XM_AI_TYPE_NEW_BSD,
	XM_AI_TYPE_DMS,
	XM_AI_TYPE_BSD_AND_DMS
}XMAIType;

//目前暂只支持录制AVI
typedef enum _XMFileFormat
{
	XM_FILE_FORMAT_AVI = 0,
	XM_FILE_FORMAT_MP4,
	XM_FILE_FORMAT_MP3,
}XMFileFormat;

//日志级别
typedef enum _XMLogLevel
{
	XM_LOG_SENSITIVE,
	XM_LOG_VERBOSE,
	XM_LOG_INFO,
	XM_LOG_WARNING,
	XM_LOG_ERROR,
	XM_LOG_NONE,
}XMLogLevel;

typedef enum WindowStyle
{
	split1,
	split2,	
	split3,
	split4,	
	splitNR,
}XMWindowStyle;

typedef enum _XMRealPlayMode
{
	XM_PLAY_SENSOR,		//前录
	XM_PLAY_AD,			//后录
	XM_PLAY_BOTH,		//全部
	XM_PLAY_NONE,		//全不显示,一般是关机时
}XMRealPlayMode;

typedef enum
{
	ShutDownMode_Key,
	ShutDownMode_PressKey,
	ShutDownMode_Acc,
	ShutDownMode_Gsensor,
	ShutDownMode_USBDisconnect,
}ShutDownMode;

typedef enum _XMTimeLapse
{
	XM_TIME_LAPSE_Closed = 0,
	XM_TIME_LAPSE_Frame_1,
	XM_TIME_LAPSE_Frame_2,
	XM_TIME_LAPSE_Frame_5,
}XMTimeLapse;

typedef enum _XMTimeLapseDuration
{
	XM_TIME_LAPSE_Duration_Closed = 0,
	XM_TIME_LAPSE_Duration_8,
	XM_TIME_LAPSE_Duration_16,
	XM_TIME_LAPSE_Duration_24,
}XMTimeLapseDuration;

typedef enum _XMSDCardStatus
{
    XM_SD_NOEXIST = 1,		//SD卡不存在
    XM_SD_ABNORMAL,			//SD卡不正常
    XM_SD_NORMAL,			//SD卡正常
    XM_SD_NEEDFORMAT,		//SD卡需要格式化
    XM_SD_NOTRW,			//SD卡非读写
	XM_SD_NEEDCHECKSPEED,  	//SD卡需要测速
	XM_SD_NOSPACE,			//SD卡空间不足
}XMSDCardStatus;

typedef enum _APP_PAGE
{
	APP_PAGE_PREVIEW = 0,
	APP_PAGE_PLAYBACK,
	APP_PAGE_SET,
}APP_PAGE;

typedef enum _CH_NUM
{
	CH1 = 1,
	CH2,
	CH3,
	CH4,
}CH_NUM;

#ifdef WIN32
typedef enum 
{
	USB_MODE_WIFI = 0x0,
	USB_MODE_RNDIS = 0x1,
	USB_MODE_MASS_STORAGE = 0x2,
	USB_MODE_NONE,
} USB_MODE_E;
#endif

typedef enum _XM_LOCKFILE_PERCENT
{
	XM_LOCKFILE_30_PER = 0, //加锁文件占30%
	XM_LOCKFILE_40_PER,
	XM_LOCKFILE_100_PER,
}XM_LOCKFILE_PERCENT;

//wifi低功耗消息类型
typedef enum _XMLOWPOWER_MSGTYPE{
    REG_REQ = 0,    // 注册请求
    REG_RESP = 1,   // 注册响应
    HEARTBEAT = 2,  // 心跳保活
    WAKEUP = 3      // 唤醒请求
};
/*-----------------------------struct------------------------------*/
//配置媒体格式，avi录制使用
typedef struct _XM_MW_Media_Info
{
	//视频
	PAYLOAD_TYPE_E video_codec;
	int width;
	int height;
	int frame_rate; //帧率，一般25
	int bit_rate;//码率，一般720P 1024, 1080P 2048, 1440P 3072

	//音频
	PAYLOAD_TYPE_E audio_codec;
	int bit_per_sample;
	int samples_per_sec;
	int audio_channel;
}XM_MW_Media_Info;

//编码参数配置
typedef struct _XM_MW_Encode_Info {
	int channel_num; //要配置几个通道
	XM_MW_Media_Info enc_medias[XM_MAX_CHANNEL_NUM][2];
}XM_MW_Encode_Info;

typedef struct _XM_MW_File_Info
{
	XM_MW_Media_Info media_info;
	int duration;//文件总时长，单位毫秒
}XM_MW_File_Info;

typedef struct _XM_MW_Storage_SDCard_Config
{
	//sd卡数量，不能超过XM_STORAGE_SDCard_TotalNum
	int sdcard_count;
	//分区路径，一般是/dev/mmcblk0
	char partition_path[XM_STORAGE_SDCard_TotalNum][XM_MAX_STORAGE_PATH_LEN];
	//一般是/mnt/tfcard
	char mount_path[XM_STORAGE_SDCard_TotalNum][XM_MAX_STORAGE_PATH_LEN];
}XM_MW_Storage_SDCard_Config;

typedef struct _XM_MW_SDCard_Space {
	//sd卡总容量
	int64_t sdcard_total_space;
	//sd卡剩余容量
	int64_t sdcard_free_space;
}XM_MW_SDCard_Space;

typedef struct _XM_MW_Media_Frame
{
	bool video;
	PAYLOAD_TYPE_E codec_type;
	bool key_frame;
	uint8_t* frame_buffer;
	int frame_size;
	int64_t timestamp;
}XM_MW_Media_Frame;

typedef struct _XM_MW_Video_Extra_Info
{
	bool file_complete;//文件是否完整
	bool file_locked;//文件被锁定(关键文件，不会覆盖)
	bool file_available;//是否可用
}XM_MW_Video_Extra_Info;

typedef struct _XM_MW_Picture_Extra_Info
{
	short width;
	short height;
	bool file_locked;
}XM_MW_Picture_Extra_Info;

typedef struct _XM_MW_Record_Info 
{
	int file_len;//文件长度，单位字节
	int channel;//通道号
	int64_t create_time;//文件创建时间
	char file_name[XM_MAX_STORAGE_FILE_LEN];//不包含路径名，以节约内存
	union {
		XM_MW_Video_Extra_Info video_info;
		XM_MW_Picture_Extra_Info picture_info;
	};
}XM_MW_Record_Info;

typedef struct _XM_MW_Record_List
{
	char dir_name[XM_MAX_STORAGE_PATH_LEN];
	int record_count;
	XM_MW_Record_Info records[XM_MAX_RECORD_NUM];
}XM_MW_Record_List;

typedef struct _XM_MW_Record_Query {
	XMStorageSDCardNum sdc_num;
	int channel;
	int64_t start_time; //日历时间，单位秒
	int64_t end_time;	//日历时间，单位秒
	XMRecordType type;	
}XM_MW_Record_Query;

typedef struct _XM_MW_File_Param {
	XMStorageSDCardNum sdc_num;
	int channel;
	XMRecordType type;
	char file_name[XM_MAX_STORAGE_FILE_LEN];
}XM_MW_File_Param;

typedef struct _XM_MW_ADAS_Param {
	XM_IA_ADAS_CONFIG_S cam_cal;
}XM_MW_ADAS_Param;

typedef struct _XM_MW_BSD_Param {
	XM_IA_SENSE_E sensitivity; //报警灵敏度
	int rule_num; //必须小于XM_IA_RULE_NUM_MAX
	XM_IA_RULE_REGION_S rule_region[XM_IA_RULE_NUM_MAX];
}XM_MW_BSD_Param;

typedef struct _XM_MW_SPLIT_Param{
	int spt;
	int chnmask;
	int chnnum;
	RECT_S rect[XM_MAX_CHANNEL_NUM];
}XM_MW_SPLIT_Param;

typedef struct _XM_MW_SPLIT_RECT_Param{
	RECT_S maxRect;
	int iHorNum;
	int iVerNum;
}XM_MW_SPLIT_RECT_Param;

typedef struct VIDEO_RECT_PARAM
{
	unsigned short left;
	unsigned short top;
	unsigned short right;
	unsigned short bottom;
}VIDEO_RECT;

typedef enum{
	WATCHER_COMMON_CHANNEL,  //普通信道
	WATCHER_SPECIAL_CHANNEL, //特殊信道
    WATCHER_INSERT_WHITE_LIST,
	WATCHER_ELIMINATE_WHITE_LIST,
	WATCHER_NR
}WATCHER_STATE_E;

typedef struct {
    WATCHER_STATE_E		mode;
	int 	            channel;
	unsigned char mac[20];
}CONFIG_WATCHER_HS;

/// 视频覆盖参数
typedef struct VIDEO_COVER_PARAM
{
	/// 覆盖的矩形区域，采用相对坐标体系。
	VIDEO_RECT	rect;

	/// 被覆盖区域显示的颜色。16进制表示为0x00BBGGRR，最高字节保留。
	unsigned int		color;

	/// 区域操作的类型， 取video_cover_t类型的值。窗口和全屏操作应该是独立的， 
	/// 即窗口内的操作不影响全屏，全屏内的操作也不影响窗口。 清除区域时，其他
	/// 参数被忽略。
	int			enable;
}VIDEO_COVER_PARAM;

typedef struct WIFI_PARAM
{
	char name[XM_MAX_WIFI_PASSWORD_LEN];
	char password[XM_MAX_WIFI_PASSWORD_LEN];
}WIFI_PARAM_S;

typedef struct WIFI_MAC_PARAM
{
	char ssid[XM_MAX_WIFI_PASSWORD_LEN];
	char password[XM_MAX_WIFI_PASSWORD_LEN];
	char mac[18];
}WIFI_MAC_PARAM_S;

typedef struct APP_PARAM_ITEMS
{
	int record;
	int osd_time;
	int mic;
	int voice_control;
	int sdcard_speed;
	int resolution;
	int circular_record_time;
	int gsr_sensitivity;
	int parking_monitor;
	int screen_standby;
	int timelapse_rate;
	int key_tone;
	int park_record_time;
	char version[32];
}APP_PARAM_ITEMS_S;

//缩放参数
typedef struct _XM_MW_ZOOM_PARAM
{
    int width;
    int height;
    int offsetX;
    int offsetY;
}XM_MW_ZOOM_PARAM;

typedef struct _XM_MW_OSD_INFO
{
	bool enable;//使能
	int x;//超始位置横坐标，以8192为基准
	int y;//超始位置纵坐标，以8192为基准
	int width;//不能超过XM_MAX_OSD_WIDTH
	int height;//不超过XM_MAX_OSD_HEIGHT
	char osd_buf[XM_MAX_OSD_WIDTH*XM_MAX_OSD_HEIGHT*2];//rgb1555格式
	bool up;//是否放大，true放大1倍
}XM_MW_OSD_INFO;

typedef struct _XM_MW_OSD_INFOS
{
	XM_MW_OSD_INFO osd[3][2];//3个通道，每个通道2个osd区域。通道：0前摄主码流 1后路主码流 2前摄子码流。区域：0一般是固定的logo图片 1是变化的gps信息等
}XM_MW_OSD_INFOS;

typedef struct _XM_MW_OSD_UPDATE_INFO
{
	char osd_buf[XM_MAX_OSD_WIDTH*XM_MAX_OSD_HEIGHT*2];//rgb1555格式
}XM_MW_OSD_UPDATE_INFO; //目前只针对osd区域1有效


enum CUR_PLAY_STATUS {
    PLAY_NONE = 0,
    PLAY_PREVIEW,
    PLAY_PLAYBACK_VOD,
    PLAY_PLAYBACK_PICTURE,
};

//自定义算法结果返回

typedef struct
{
    unsigned char ucAlarm;
    XM_IA_RECT_S stRect;
}XM_IA_TCP_INFO_S;

typedef struct
{
    unsigned int uiTargetNum;
    XM_IA_TCP_INFO_S *pstInfo;
}XM_IA_TCP_RESULT_S;
//wifi低功耗前端设备信息
typedef struct 
{
	char devid[18];			//设备唯一id（暂时用mac地址）
	char check_word[17];	//注册时设备产生的随机key
	char status;			//设备状态（1：唤醒 0：休眠 -1:未注册）
	int connId;          //心跳保活连接的套接字
}XM_LOWPOWER_DEV_INFO_S;
/*----------------------------callback-----------------------------*/
//获取跳频许可回调
typedef bool (*XM_GetFrehopPermit_CallBack)(void);
//点播流回调，回放时当返回false，会继续回调该帧
typedef bool (*XM_MW_VodStream_CallBack)(long handle, XM_MW_Media_Frame* media_frame, int64_t user);
//实时流回调
typedef void (*XM_MW_RealStream_CallBack)(int channel, int stream, XM_MW_Media_Frame* media_frame, int64_t user);
//事件回调
typedef void (*XM_MW_Event_CallBack)(long handle, XMEventType event_type, const char* msg, int param, int64_t user);
//日志回调
typedef void (*XM_MW_LOG_CallBack)(char* log_buf, int log_len, int64_t user);
