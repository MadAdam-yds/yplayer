#ifndef _DEMUXER_H_
#define _DEMUXER_H_
extern "C"{ 
#include <libavutil/log.h>
#include <libavformat/avformat.h>
}
#include <string>
#include"commonType.h"
#include "PacketQueue.h"
#include <memory>
#include "DataStreamContext.h"
#include <thread>
using namespace std;
class Demuxer
{
private:
    // 宏不受C++类作用域约束,不管是定义在public、private、protected中,都是全局可见的.
    #define MIN_FRAMES 25
    #define MAX_QUEUE_SIZE  (15 * 1024 * 1024)
    bool paused;            // 暂停标志
    bool lastPaused;        // 上一次暂停标志,配合paused使用可表示状态切换的动作
    bool seekReq;           // seek请求
    int  seekFlags;         // seek标志
    int64_t  seekPos;       // seek位置:相对文件起始位置,单位(秒)
    int64_t  seekRel;       // seek相对位置:相对上次播放的位置,单位(秒)
    string url;             // 文件路径
    int readPauseReturn;    // 暂停读取数据接口的返回值
    bool realtime;          // 是不是实时流
    int audioStreamIndex;
    int videoStreamIndex;
    int subtitleStreamIndex;
    bool eof;
    bool infiniteBuffer;
    AVStream *audioStream;
    AVStream *videoStream;
    AVStream *subtitleStream;
    std::mutex qmutex;                               // 互斥锁
    std::condition_variable condContinueDemuxer;     // 条件变量
    AVFormatContext *_ic;     // 输入文件格式上下文
    int streamsIndex[AVMEDIA_TYPE_NB];// 媒体流索引
    std::shared_ptr<thread> demuxerThreadPtr;       // demuxer thread
    std::shared_ptr<PacketQueue> audioQ;            // 音频队列
    std::shared_ptr<PacketQueue> videoQ;            // 视频队列
    std::shared_ptr<PacketQueue> subtitleQ;         // 字幕队列
    DataStreamContext &dsctx;                       // 数据流上下文       
public:
    Demuxer(const string &url,DataStreamContext &ctx);
    ~Demuxer();
    int init();                 // 初始化 >=0 success
    int start();                // 启动 demuxer thread
    void setPaused(bool paused);// 设置暂停状态
    void streamSeek(int64_t pos, int64_t rel, int seekByBytes);//流Seek
    bool hasStream(enum AVMediaType media); // 检查是否具有某种流
    bool allowTSDiscontinue();//  是否允许TS discontinuity
private:
    static int demuxerThread(void *arg);            // demuxer func
    bool isRealtime();
    bool streamHasEnoughPackets(AVStream *st, int stream_id, PacketQueue *queue)const;
    int  packetQueuePutNullpacket(PacketQueue *q, int stream_index);
    bool checkUrl();                                // 检查url是否有效
};




#endif