#ifndef _SYNC_MANAGER_H_
#define _SYNC_MANAGER_H_
#include "YClock.h"
#include "commonType.h"
#include "DataStreamContext.h"
#include "FrameQueue.h"
class SyncManager {
private: 
    ENUM_AV_SYNC_TYPE syncType;   // 同步方式
    Clock *pAudClock;             // 音频时钟
    Clock *pVidClock;             // 视频时钟
    Clock *pExtClock;             // 外部时钟
    double max_frame_duration;    // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
public:
    SyncManager();
    ~SyncManager();
    ENUM_AV_SYNC_TYPE getMasterSyncType();  // 获取主时钟类型
    double getMasterClock();                // 获取主时钟pts
    void  setExtClockSpeed(double speed);   // 设置外部时钟速度
    void  setMaxFrameDuration(double max);   // 设置最大帧时长
    double vpDuration(Frame *vp, Frame *nextvp);// 计算帧时长
    double computeTargetDelay(double delay);    
    double getLastUpdated(ENUM_CLOCK_TYPE clockType);//
    void   setClockPasuse(ENUM_CLOCK_TYPE clockType,bool pause);//
    void   setClock(ENUM_CLOCK_TYPE clockType,double pts);//
    double getClock(ENUM_CLOCK_TYPE clockType);//
    void   syncClockToSlave(ENUM_CLOCK_TYPE clockType,Clock slave);
    Clock getClockObj(ENUM_CLOCK_TYPE clockType);
    //check_external_clock_speed();     // 应该在上层player判断播放速度更合适,然后调用setExtClockSpeed
};

#endif