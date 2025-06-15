#include "YClock.h"
#include <math.h>
/*
Clock::Clock():
    pts(0.0),
    ptsDrift(0.0),
    lastUpdated(0.0),
    _speed(1.0),
    paused(0)
{
}
*/
Clock::Clock(DataStreamContext &ctx):
    pts(0.0),
    ptsDrift(0.0),
    lastUpdated(0.0),
    _speed(1.0),
    paused(0),
    dsctx(ctx)
{
}

Clock::~Clock(){
}
// 初始化时钟
void Clock::initClock(){
    this->_speed = 1.0;
    this->paused = 0;
    setClock(NAN);
} 
// 获取时钟,返回基于媒体时钟的时间                         
double Clock::getClock(){
    if (this->paused) {
        return pts;//暂停状态返回上次最新的pts
    } 
    else {
        double time = av_gettime_relative() / 1000000.0;//取当前系统时间
        return ptsDrift + time - (time - lastUpdated) * (1.0 - _speed);//根据速度动态调整,返回的是媒体时钟时间,系统时间time转换为媒体时间
    }
}
// 基于媒体时钟pts设置时钟                            
void Clock::setClock(double pts){
    double time = av_gettime_relative() / 1000000.0;
    setClockAt(pts, time);
}
// 基于媒体时钟pts和基于系统时钟time设置时钟                  
void Clock::setClockAt(double pts, double time){
    this->pts = pts;
    this->lastUpdated = time;
    this->ptsDrift = this->pts - time;//初始差值,媒体时间-系统时间
}
// 设置时钟速度    
void Clock::setClockSpeed(double speed){
    setClock(getClock());
    _speed = speed;
}
// 时钟纠偏,根据slave纠正当前时钟            
void Clock::syncClockToSlave(Clock slave){
    double clock = getClock();
    double slave_clock = slave.getClock();
    //(isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD) 这个条件在上层应用判断.否则破坏单一职责原则.
    if (!isnan(slave_clock))//需要在clock无效或者偏差超过阈值时，才会触发同步
        setClock(slave_clock);//将从时钟的值，赋值给主时钟
}
void Clock::onSerialChanged(int new_serial){
    
}        