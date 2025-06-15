#ifndef _PLAYER_H_
#define _PLAYER_H_
#include "SyncManager.h"
#include "Demuxer.h"
#include "VideoSoftDecoder.h"
#include "AudioSoftDecoder.h"
#include "DataStreamContext.h"
#include "YPlayerSDK.h"
#include <memory>
#include <thread>
class Player
{ 
public:
    static int audCallbackThreadFunc(void *arg); // 音频回调函数
    static int vidCallbackThreadFunc(void *arg); // 视频回调函数

    // 不再需要flush_pkt,所有和flush_pkt有关的状态控制逻辑通过观察者模式实现
    //static AVPacket   flush_pkt;        // 刷新包
private:
    std::shared_ptr<DataStreamContext> _dataStreamCtx;    // 数据流上下文
    SyncManager syncMg;                 // 同步管理器
    std::unique_ptr<Demuxer> pdemuxer;  // 解复用器,使用指针是为了在Player构造函数后再延迟初始化
    std::unique_ptr<VideoSoftDecoder>  _vidSoftDecoder;   // 视频软解码器
    std::unique_ptr<AudioSoftDecoder>  _audSoftDecoder;   // 音频软解码器
    bool forceRefresh;                  // 强制刷新标志
    bool abortRequest;                  // 退出标志
    bool paused;                        // 暂停标志
    bool lastPaused;                    // 上次暂停标志
    bool seekReq;                       // seek请求
    int  seekFlags;                     // seek标志
    int  seekPos;                       // seek位置
    int  seekRel;                       // seek相对位置
    std::atomic<int> serial;            // 序列号(需要思考serail的怎么用,在ffplay中多个模块都有对应的serail)
    // 三个流索引是否有用?
    int videoStreamIdx;                 // 视频流索引
    int audioStreamIdx;                 // 音频流索引
    int subtitleStreamIdx;              // 字幕流索引
    shared_ptr<std::thread> vidCallbackThreadPtr; // 视频回调线程指针
    shared_ptr<std::thread> audCallbackThreadPtr; // 音频回调线程指针
    ENUM_YPLAYER_STATE workState;            // 播放器工作状态
    double frameTimer;                      // 视频帧显示的时间点,系统时间(微秒)
    bool step;                               // 单帧播放的标志,step to next  frame
public:
    Player();
    ~Player();
    int close();                       // 关闭播放器
    int start();                       // 启动播放器
    int pause(bool pasuseState);       // 暂停或恢复播放器
    int seek(int64_t pos,int64_t rel); //播放器跳转
    int speed(ENUM_YPLAYER_PLAY_SPEED speed);//设置播放速度
    int OneByOne();                     // 单帧播放
    void streamTogglePause();           // pause or resume player
    void setDecodeCallback(DECODE_CALLBACK_PF callback,void *user);
    static DECODE_CALLBACK_PF decCallbackPF;    // 解码后数据的回调函数
    void *puser;                                // 解码后数据的回调函数的参数
    setPort(uint32_t po){port = po;}
    bool checkUrl(string url);         // 检查url
private:
    uint32_t port;
};
#endif