#ifndef _YPLAYER_CLOCK_H_
#define _YPLAYER_CLOCK_H_
extern "C"{ 
#include "libavutil/time.h"
}
#include "DataStreamContext.h"
class Clock:public ISerialListener
{
/*   
 * 存在两个时间参考系,系统时钟和媒体时钟.
 * 系统时间是本地系统的时间,媒体时间是音视频媒体资源的时间,例如pts显示时间.
 * 靠某一种时间参考系无法表示播放的真实进度.
 */
private:
    double  pts;            // 当前帧的显示时间戳，单位：秒,媒体时钟(无法直接反应实时播放进度)
    double  ptsDrift;       // 同步机制中,核心补偿参数,pts与系统时间的差值（pts = 系统时间 + pts_drift)
    double  lastUpdated;    // 最后一次更新时间(系统时间),系统时钟
    double  _speed;          // 播放速度(默认1.0，支持快进/慢放),只在外部时钟使用.
    int     paused;         // 暂停播放
    DataStreamContext &dsctx;        // 数据流上下文 
public:
    //Clock();
    ~Clock();
    Clock(DataStreamContext &ctx);
    void    initClock();                            // 初始化时钟
    double  getClock();                             // 获取时钟,返回基于媒体时钟的时间
    void    setClock(double pts);                   // 基于媒体时钟pts设置时钟
    void    setClockAt(double pts, double time);    // 基于媒体时钟pts和基于系统时钟time设置时钟
    void    setClockSpeed(double speed);            // 设置时钟速度
    void    syncClockToSlave(Clock slave);         // 时钟纠偏,根据slave纠正当前时钟
    double  getlastUpdated(){return lastUpdated;}
    void    setPaused(bool pause){paused = pause;}
    virtual void onSerialChanged(int new_serial);

};


#endif