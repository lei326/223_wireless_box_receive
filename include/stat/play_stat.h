#pragma once

#include <stdint.h>
#include <deque>
#include <mutex>

/* 队列保留多少条记录。
 * 原厂：addTcpFrame 里写死 200（"//只保存最新200帧的信息"）
 * 我们 20fps，200 帧 = 10 秒的历史。统计窗口只用最近 1 秒，
 * 留 10 秒是给将来做更长窗口的统计留余量
 */
#define STAT_MAX_RECORDS        200

/* 统计窗口。原厂 GetPlayInfo 里写死 1000ms（"// 只统计1秒内的数据"）*/
#define STAT_WINDOW_MS          1000

/* 一帧的记录
 * 原厂对应：MppMdl.h 的 struct BitRateInfo（去掉了 delay_time）
 */
struct FrameRecord {
    int64_t recv_time_ms;       /* 收到的时刻。原厂：recv_time */
    int     frame_len;          /* 这帧多少字节。原厂：frame_len */
};

/* 一次查询的结果
 * 原厂对应：GetPlayInfo 的输出参数 rate / bitrate / framenum
 *           / avg_frame_duration_ms
 */
struct PlayInfo {
    int     fps;                /* 最近 1 秒收到几帧      原厂 *rate      */
    int     bitrate_bps;        /* 最近 1 秒收了多少比特  原厂 *bitrate   */
    int64_t total_frames;       /* 开机至今累计帧数        原厂 *framenum  */
    int     avg_interval_ms;    /* 平均帧间隔（毫秒）      原厂 *avg_frame_duration_ms */
};

class PlayStat
{
public:
    static PlayStat* Instance();

    void AddFrame(unsigned int frame_len);

    int GetPlayInfo(PlayInfo* out);

    void Reset();

private:
    PlayStat();
    ~PlayStat();

    static PlayStat* instance_;

    std::deque<FrameRecord> records_;

    int64_t total_frames_;

    std::mutex mutex_;
};