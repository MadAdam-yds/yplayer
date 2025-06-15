#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_
// 对uint32_t,int64_t的定义需要再优化
#if defined(_WIN32) || defined(_WIN64)
    typedef long long int64_t;
    typedef unsigned int  uint32_t;
    typedef int int32_t;
#elif defined(__linux__)
    typedef long long int64_t;
    typedef unsigned int  uint32_t;
    typedef int int32_t;
#endif
/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

// 同步方式
typedef enum {
    AV_SYNC_AUDIO_MASTER        = 0, 
    AV_SYNC_VIDEO_MASTER        = 1,
    AV_SYNC_EXTERNAL_CLOCK      = 2,
}ENUM_AV_SYNC_TYPE;
// 工作状态
typedef enum {
    YPLAYER_STATE_IDLE          = 0,
    YPLAYER_STATE_PLAYING       = 1,
    YPLAYER_STATE_PAUSE         = 2,
    YPLAYER_STATE_STOP          = 3,
}ENUM_YPLAYER_STATE;
// 时钟类型
typedef enum {
    CLOCK_AUDIO                 = 0,   // 音频时钟 
    CLOCK_VIDEO                 = 1,   // 视频时钟
    CLOCK_EXTERNAL              = 2,   // 外部时钟
}ENUM_CLOCK_TYPE;
#endif