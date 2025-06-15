
#ifndef _PACKET_QUEUE_H_
#define _PACKET_QUEUE_H_

extern "C"{ 
#include <libavformat/avformat.h>

}
#include <mutex>
#include <condition_variable>
#include "IPacketSource.h"
#include "commonType.h"
#include "DataStreamContext.h"
using namespace std;

class PacketQueue:public IPacketSource,ISerialListener
{
private:
    //私有结构体
    typedef struct MyAVPacketList {
        AVPacket pkt;
        struct MyAVPacketList *next;
        int serial;   //每个pkt对应的逻辑流.
    } MyAVPacketList;
    //隐藏结构体的二进制兼容性方案:头文件中前置声明,在cpp中实现struct PacketQueue::MyAVPacketList结构体的定义.
    //struct MyAVPacketList;  //前置声明，
    
    MyAVPacketList *lastPacket;        //队列尾部
    MyAVPacketList *firstPacket;      //队列头部
    int nbPackets;                    //队列中packet个数
    int size;                         //队列中所有packet总大小(Byte)
    int64_t duration;                 //队列中所有packet总duration    
    //int abortRequest;               //是否退出(破坏单一职责原则)
    std::mutex qmutex;                //互斥锁
    std::condition_variable cond;     //条件变量
    //一个mutex应该就够了,不用cond和其它线程交互,否则破坏了单一职责原则.
    //如果这个cond用户类内部生产消费模型,就没破坏单一职责原则.这里没有容量限制，所以生产者不用wait cond,故用一个cond就行.
    DataStreamContext &dsctx;        // 数据流上下文       
public:
    //PacketQueue();
    ~PacketQueue();
    PacketQueue(DataStreamContext &ctx); // 依赖注入
    void init();                //初始化
    int  put(AVPacket *pkt);    //放入一个packet
    //这个函数放到这里不合适,因为需要和stream_index强关联，违反了单一职责原则.应该
    //方法1.放到上层,直接向指定解码器发送Null Packet,不经过队列.这种方法需要保证发送递Null Packet时,队列是空的.
    //方法2.上层构造好 Null Packet后put到PacketQueue中.
    //int  putNullPacket( int stream_index);
    virtual int getPacket(AVPacket *pkt, bool block,int &serial);//获取一个packet,阻塞或非阻塞
    virtual bool isEmpty();                          //判断队列是否为空
    void flush();                       //清空队列
    void destroy();                     //销毁队列
    void abort();                       //退出
    int64_t getDuration()const {return duration;}
    int getnbPackets()const {return nbPackets;}
    int getSize()const {return size;}
    //void start();                     //开始(破坏单一职责原则)
private:
    int putPrivate(AVPacket *pkt);      //放入一个packet
    virtual void onSerialChanged(int new_serial);

};



#endif