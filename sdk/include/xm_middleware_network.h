#pragma once

#include <string>
#include <stdint.h>
#include "XMIPDef.h"

//服务端
int XM_Middleware_Network_StartServer(int signal_port, int media_port);
int XM_Middleware_Network_StopServer();
int XM_Middleware_Network_SetMediaInfo(const XMMediaInfo& media_info);
int XM_Middleware_Network_SendBuffer(int engineId, int connId, uint8_t type, char * data, int contentLen);
int XM_Middleware_Network_SendFrame(int engineId, int connId, char* data, int len, bool video_frame, bool key_frame, uint32_t pts, int id);
int XM_Middleware_Network_SetDataCallback(XM_Data_CallBack cb);
int XM_Middleware_Network_SetEventCallback(XM_Event_CallBack cb);
int XM_Middleware_Network_SetFrameList(int count);
int XM_Middleware_Network_ClearFrameList();

//客户端
void XM_Middleware_Network_Client_Startup();
int XM_Middleware_Network_Client_StopClient(int chnum);
int XM_Middleware_Network_Client_StartClient(int chnum, char *ip, int signal_port, int media_port);
int XM_Middleware_Network_Client_SendData(int chnum, uint8_t type, char* buf, int len);
int XM_Middleware_Network_Client_SendFrame(int chnum, char* buf, int len, bool video_frame, bool key_frame, uint32_t pts);
int XM_Middleware_Network_Client_SetEventCallback(int chnum, XM_Event_CallBack cb);
int XM_Middleware_Network_Client_SetDataCallback(int chnum, XM_Data_CallBack cb);
int XM_Middleware_Network_Client_CleanRecvBuffer(int chnum);
int XM_Middleware_Network_Client_processIFrame(int chnum);
//time_out:心跳超时时间（ms）  keepInterval:心跳发送间隔时间（s） selectTimeout:select连接超时时间（s）
int XM_Middleware_Network_Client_SetHeartBeatTimeOut(int chnum, int time_out, int keepInterval=1, int selectTimeout=3);
int XM_Middleware_Network_Client_SendAiDecode(int chnum, unsigned char* buf, int len, int type);
