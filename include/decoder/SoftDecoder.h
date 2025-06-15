#ifndef _SOFTDECODER_H_
#define _SOFTDECODER_H_

#include "IDecoder.h"
#include "FrameQueue.h"
#include <libavformat/avformat.h>
#include<memory>
#include<thread>
#include<atomic>
#include "DataStreamContext.h"
using namespace std;
class SoftDecoder:public IDecoder
{
    static int decodeThread(void *arg); // 解码函数
private:
    AVFormatContext *avfctx;                // 文件上下文
    AVCodecContext *avctx;                  // 解码器上下文
    AVCodec *codec;                         // 解码器
    IPacketSource *dataSource;              // 编码数据源
    int streamIndex;                        // 码流索引
    AVStream *stream;                       // 码流
    bool packetPending;                     // 是否有待解码的数据包(暂存pkt)
    AVPacket *pendingPkt;                   // 待解码的数据包
    FrameQueue _frameQueue;                  // 解码后的帧队列
    shared_ptr<std::thread> decodeThreadPtr; // 解码线程指针
    DataStreamContext &dsctx;                // 数据流上下文
    std::atomic_int frameDropsLate;        // 因播放延迟而丢弃的帧个数
public:
    //SoftDecoder();
    SoftDecoder(DataStreamContext & ctx);  // 依赖注入
    virtual ~SoftDecoder()=default;
    // 从pDataSource中获取编码数据源,该数据源属于文件pAvfctx的stIndex这路码流.
    virtual int init(IPacketSource *pDataSource,AVFormatContext *pAvfctx,int stIndex) override;
    virtual void start() override; 
    Frame* peekReadable();               // 获取队列中可读的元素
    void   next();                       // 队列中删除元素  
    Frame* peekLast();                   // 获取队列中上次显示的元素
    Frame* peekNext();                   // 获取队列中第二个元素
    int nbRemaining();                   // 获取队列中剩余元素个数
    int FrameDropsLate();                // 获取队列中丢帧个数
    void FrameDropsLateInc();             // 队列中丢帧个数加1
    Frame* peek();                        

};

#endif