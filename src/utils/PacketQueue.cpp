#include "PacketQueue.h"
/*
PacketQueue::PacketQueue():
    lastPacket(nullptr),
    firstPacket(nullptr),
    nbPackets(0),
    size(0),
    duration(0)
{
    // 构造函数实现
    // 可以在这里初始化成员变量或执行其他初始化操作
}*/
PacketQueue::PacketQueue(DataStreamContext &ctx):
    lastPacket(nullptr),
    firstPacket(nullptr),
    nbPackets(0),
    size(0),
    duration(0),
    dsctx(ctx)
{
}
PacketQueue::~PacketQueue() {
    // 析构函数实现
    // 可以在这里释放资源或执行清理操作
}

void PacketQueue::init() {
    // 初始化方法实现
    //mutex和cond不用手动构造
     
}

int PacketQueue::put(AVPacket *pkt) {
    // 放入一个packet方法实现
    int ret;
    lock_guard<std::mutex>lock(qmutex);
    ret = putPrivate(pkt);
    if (ret < 0)
        av_packet_unref(pkt);
    return ret;
}

int PacketQueue::putPrivate(AVPacket *pkt) {
    // 放入一个packet私有方法实现
    MyAVPacketList *pkt1;
    pkt1 = static_cast<MyAVPacketList *>(av_malloc(sizeof(MyAVPacketList)));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    if (this->lastPacket)
        this->firstPacket = pkt1;
    else
        this->lastPacket->next = pkt1;
    this->lastPacket = pkt1;
    this->nbPackets++;
    this->size += pkt1->pkt.size + sizeof(*pkt1);
    this->duration += pkt1->pkt.duration;
    this->cond.notify_one();//emit signal
    return 0;
}
bool PacketQueue::isEmpty() {
    // 判断队列是否为空方法实现
    return this->nbPackets == 0;
}
int PacketQueue::getPacket(AVPacket *pkt, bool block,int &serial) {
    // 获取一个packet方法实现
    MyAVPacketList *pkt1;
    int ret;
    //lock_guard不支持手动lock和unlock.
    //lock_guard<std::mutex>lock(qmutex);//这里不能使用guard管理器,因为后面可能需要等待条件变量
    std::unique_lock<std::mutex>unilock(qmutex);
    for (;;) {
        
        pkt1 = this->firstPacket;
        if (pkt1) {
            this->firstPacket = pkt1->next;
            if (!this->firstPacket)
                this->lastPacket = NULL;
            this->nbPackets--;
            this->size -= pkt1->pkt.size + sizeof(*pkt1);
            this->duration -= pkt1->pkt.duration;
            *pkt = pkt1->pkt;    // copy AVPacket
            serial =  pkt1->serial;
            av_free(pkt1);
            ret = 1;
            break;
        } 
        else if (!block) {
            ret = 0;
            break;
        } 
        else {
            //wait cond
            this->cond.wait(unilock);//wait过程中会自动unlock
            //wait结束后自动lock
        }
    }
    unilock.unlock();//unlock
    return ret;
}

void PacketQueue::flush() {
    // 清空队列方法实现
    MyAVPacketList *pkt, *pkt1;
    lock_guard<std::mutex>lock(qmutex);//guard lock
    for (pkt = firstPacket; pkt; pkt = pkt1) {
        pkt1 = pkt->next;//销毁前备份next
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);// 释放内存
    }
    this->lastPacket = NULL;
    this->firstPacket = NULL;
    this->nbPackets = 0;
    this->size = 0;
    this->duration = 0;
}

void PacketQueue::destroy() {
    // 销毁队列方法实现
    this->flush();
    //mutex和cond不用手动销毁
}

void PacketQueue::abort() {
    // 退出方法实现
    lock_guard<std::mutex>lock(qmutex);//guard lock
    // q->abort_request = 1;
    // SDL_CondSignal(q->cond);
}

void PacketQueue::onSerialChanged(int new_serial){
    
}