#include "stat/play_stat.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOGI(fmt, ...) printf("[stat] " fmt "\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[stat][ERR] " fmt "\n", ##__VA_ARGS__)

static int64_t NowMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

PlayStat *PlayStat::instance_ = NULL;

PlayStat *PlayStat::Instance()
{
    if (instance_ == NULL)
    {
        instance_ = new PlayStat();
    }
    return instance_;
}

PlayStat::PlayStat() : total_frames_(0)
{
}

PlayStat::~PlayStat()
{
}

void PlayStat::AddFrame(unsigned int frame_len)
{
    FrameRecord rec;
    rec.recv_time_ms = NowMs();
    rec.frame_len = (int)frame_len;
    std::lock_guard<std::mutex> guard(mutex_);
    records_.push_back(rec);
    if ((int)records_.size() > STAT_MAX_RECORDS)
    {
        records_.pop_front();
    }
    total_frames_++;
}

void PlayStat::Reset()
{
    std::lock_guard<std::mutex> guard(mutex_);
    records_.clear();
    LOGI("stat reset (total_frames kept at %lld)", (long long)total_frames_);
}

int PlayStat::GetPlayInfo(PlayInfo *out)
{
    if (NULL == out)
    {
        return -1;
    }
    memset(out, 0, sizeof(*out));

    const int64_t now = NowMs();

    int frame_count = 0;
    int total_len = 0;
    int64_t total_duration = 0;

    int64_t prev_recv_time = -1;

    std::lock_guard<std::mutex> guard(mutex_);

    const int size = (int)records_.size();

    for (int i = size - 1; i >= 0; i--)
    {
        const FrameRecord &rec = records_[i];
        if (now - rec.recv_time_ms > STAT_WINDOW_MS)
        {
            break;
        }
        if (prev_recv_time >= 0)
        {
            total_duration += (prev_recv_time - rec.recv_time_ms);
        }
        prev_recv_time = rec.recv_time_ms;
        frame_count++;
        total_len += rec.frame_len;
    }

    out->fps = frame_count;
    out->bitrate_bps = total_len * 8;
    out->total_frames = total_frames_;

    if(frame_count > 1)
    {
        out->avg_interval_ms = (int)(total_duration) / (frame_count - 1);
    }else{
        out->avg_interval_ms = 0;
    }
    return 0;
}