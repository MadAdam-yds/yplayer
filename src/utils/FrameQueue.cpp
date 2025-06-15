
#include "FrameQueue.h"

#if 0
FrameQueue::FrameQueue():
    rindex(0),
    windex(0),
    size(0),
    maxSize(0),
    keepLast(0),
    rindexShown(0)
{
    // 构造函数实现
    // 可以在这里初始化成员变量或执行其他初始化操作
}
#endif

FrameQueue::FrameQueue(DataStreamContext &ctx):
    rindex(0),
    windex(0),
    size(0),
    maxSize(0),
    keepLast(0),
    rindexShown(0),
    dsctx(ctx)
{
}
FrameQueue::~FrameQueue() {
    // 析构函数实现
    // 可以在这里释放资源或执行清理操作
}

int FrameQueue::init(int max_size, int keep_last) {
    // 初始化方法实现
    int i;
    this->maxSize = FFMIN(max_size, MAX_FRAME_QUEUE_SIZE);
    this->keepLast = !!keep_last;//将任意整数转换为bool值(0或1)
    for (i = 0; i < this->maxSize; i++)
        if (!(this->fQueue[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    return 0;
}
//获取队列中第一个元素
Frame* FrameQueue::peek(){
    return &this->fQueue[(rindex + rindexShown) % maxSize];
}
//获取队列中第二个元素
Frame* FrameQueue::peekNext(){
    return &this->fQueue[(rindex + rindexShown + 1) % maxSize];

}
//获取队列中上次显示的元素
Frame* FrameQueue::peekLast(){
    return &this->fQueue[rindex];
}   
//获取队列中可写的元素
Frame* FrameQueue::peekWritable(){
     /* wait until we have space to put a new frame */
     std::unique_lock<std::mutex> lk(qmutex);
     while (this->size >= this->maxSize) {
        condNoFull.wait(lk);
     }
     lk.unlock();
     return &this->fQueue[windex];
}
//队列中添加元素
void FrameQueue::push(){
    if (++windex == maxSize)
        windex = 0;
    std::lock_guard<std::mutex> lk(qmutex);
    size++;
    //每生产一个元素就notify一次,频繁用户态和内核态切换，影响性能.
    //可考虑按批量notify，减少用户态和内核态切换次数.
    condNoEmpty.notify_one();
}
//获取队列中可读的元素
Frame* FrameQueue::peekReadable(){
    /* wait until we have a readable a new frame */
    std::unique_lock<std::mutex> lk(qmutex);
    while (this->size - this->rindexShown <= 0) {
        condNoEmpty.wait(lk);
    }
    lk.unlock();
    return &this->fQueue[(rindex + rindexShown) % maxSize];
}
void FrameQueue::unrefItem(Frame *vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}
//队列中删除元素
void FrameQueue::next(){
    if (keepLast && !rindexShown) {
        rindexShown = 1;
        return;
    }
    unrefItem(&this->fQueue[rindex]);
    if (++rindex == maxSize)
        rindex = 0;
    std::lock_guard<std::mutex> lk(qmutex);
    this->size--;
    condNoFull.notify_one();//每消费一个元素就notify一次,频繁用户态和内核态切换，影响性能.
}
//获取队列中未显示的元素个数
int FrameQueue::nbRemaining(){
    return this->size - this->rindexShown;

}
//获取队列中上次显示的元素在文件中的位置(字节)
int64_t FrameQueue::lastPos(){
    Frame *fp = &this->fQueue[rindex];
    if (rindexShown)
        return fp->pos;
    else
        return -1;
}  
void FrameQueue::destory(){
    int i;
    for (i = 0; i < this->maxSize; i++) {
        Frame *vp = &this->fQueue[i];
        unrefItem(vp);
        av_frame_free(&vp->frame);
    }
}
void FrameQueue::onSerialChanged(int new_serial){
    
}