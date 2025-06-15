extern "C"{
#include "libavutil\log.h"
}
#include "SyncManager.h"

SyncManager::SyncManager(){
    max_frame_duration = 0.0;
}
SyncManager::~SyncManager(){
    //资源释放


}
ENUM_AV_SYNC_TYPE SyncManager::getMasterSyncType(){
    return AV_SYNC_VIDEO_MASTER;// 暂时先这样
}
double SyncManager::getMasterClock(){
    return 0.0;   // 暂时先这样
} 
void  SyncManager::setExtClockSpeed(double speed){
    
}
void SyncManager::setMaxFrameDuration(double max){
    max_frame_duration = max;
}
double SyncManager::vpDuration(Frame *vp, Frame *nextvp){
    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > max_frame_duration)
            return vp->duration;
        else
            return duration;
    } else {
        return 0.0;
    }
}
double SyncManager::computeTargetDelay(double delay){
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    if (getMasterSyncType()!= AV_SYNC_VIDEO_MASTER) {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        diff = pVidClock->getClock() - getMasterClock();//视频时钟与主时钟(音频时钟或外部时钟)的差值，基于媒体时间

        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isnan(diff) && fabs(diff) < max_frame_duration) {
            if (diff <= -sync_threshold)
                delay = FFMAX(0, delay + diff);//视频时钟滞后,减小deley,加快显示
            else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                delay = delay + diff;       //视频时钟超前,增大deley,减缓显示,delay较大,大范围减速.
            else if (diff >= sync_threshold)
                delay = 2 * delay;          //视频时钟超前,增大deley,减缓显示,delay较小,平滑减速.
        }
    }
    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",delay, -diff);
    return delay;
}    
double SyncManager::getLastUpdated(ENUM_CLOCK_TYPE clockType){
    switch (clockType){
        case CLOCK_AUDIO: 
            return pAudClock->getlastUpdated();
            break;
        case CLOCK_VIDEO: 
            return pVidClock->getlastUpdated();
            break;
        case CLOCK_EXTERNAL: 
            return pExtClock->getlastUpdated();
            break;
        default:
            return -1;
            break;
    }
}
void SyncManager::setClockPasuse(ENUM_CLOCK_TYPE clockType,bool pause){
    switch (clockType){
        case CLOCK_AUDIO: 
            pAudClock->setPaused(pause);
            break;
        case CLOCK_VIDEO: 
            pVidClock->setPaused(pause);
            break;
        case CLOCK_EXTERNAL: 
            pExtClock->setPaused(pause);
            break;
        default:
            break;
    }
}
void SyncManager::setClock(ENUM_CLOCK_TYPE clockType,double pts){
    switch (clockType){
        case CLOCK_AUDIO: 
            pAudClock->setClock(pts);
            break;
        case CLOCK_VIDEO: 
            pVidClock->setClock(pts);
            break;
        case CLOCK_EXTERNAL: 
            pExtClock->setClock(pts);
            break;
        default:
            break;
    }
}
double SyncManager::getClock(ENUM_CLOCK_TYPE clockType){
    switch (clockType){
        case CLOCK_AUDIO: 
            return pAudClock->getClock();
        case CLOCK_VIDEO: 
            return pVidClock->getClock();
        case CLOCK_EXTERNAL: 
            return pExtClock->getClock();
        default:
            return -1;
            break;
    }
}
void SyncManager::syncClockToSlave(ENUM_CLOCK_TYPE clockType,Clock slave){
    switch (clockType){
        case CLOCK_AUDIO: 
            pAudClock->syncClockToSlave(slave);
        case CLOCK_VIDEO: 
            pVidClock->syncClockToSlave(slave);
        case CLOCK_EXTERNAL: 
            pExtClock->syncClockToSlave(slave);
        default:
            break;
    }
}
Clock SyncManager::getClockObj(ENUM_CLOCK_TYPE clockType){
    switch (clockType){
        case CLOCK_AUDIO: 
            return *pAudClock;
        case CLOCK_VIDEO: 
            return *pVidClock;
        case CLOCK_EXTERNAL: 
            return *pExtClock;
       default: 
            return *pVidClock; // 暂时先这样
    }
}