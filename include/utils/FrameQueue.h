#ifndef _FRAME_QUEUE_H_
#define _FRAME_QUEUE_H_
#include "commonType.h"
#include <libavutil/frame.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "DataStreamContext.h"
#include "libavcodec/avcodec.h"
typedef struct Frame {
    AVFrame *frame;       //视频OR音频
    AVSubtitle sub;       //字幕
    int serial;         //对应Decoder的pkt_serial,表示该帧图像所属的逻辑流
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int width;            //图像宽度
    int height;           //图像高度
    int format;           //图像格式
    AVRational sar;       //图像像素的宽高比
    int uploaded;         //表示图像是否显示过了
    int flip_v;           //表示图像是否需要垂直翻转
} Frame;
class FrameQueue:public ISerialListener
{
private:
    #define MAX_FRAME_QUEUE_SIZE 16
    //如果将 rindex、windex、size定义为原子变量,配合原子操作。可以做到无锁环形队列.
    int rindex;                         //读索引
    int windex;                         //写索引
    int size;                           //当前队列大小(Frame个数)
    int maxSize;                        //最大队列大小(Frame个数)
    int keepLast;                       //是否保留最后显示的帧，一种队列属性；置位1时配合rindexShown使用可实现销毁next滞后读取peek_readable一个元素
    int rindexShown;                    //当前显示帧索引,和keepLast配套使用,keepLast有效时rindexShown置位1
    std::mutex qmutex;                  //互斥锁
    //考虑多生产者多消费者模型,需要两个条件变量.
    std::condition_variable condNoEmpty;     // 表示非空的条件变量
    std::condition_variable condNoFull;      // 表示非满的条件变量
    Frame fQueue[MAX_FRAME_QUEUE_SIZE];      // Frame队列
    DataStreamContext &dsctx;                 // 数据源上下文
public:
    //FrameQueue();
    ~FrameQueue();
    FrameQueue(DataStreamContext &ctx);
    int init(int max_size, int keep_last);// 初始化队列
    void destory();                       // 销毁队列
    //void signal();
    Frame* peek();                       // 获取队列中第一个元素
    Frame* peekNext();                   // 获取队列中第二个元素
    Frame* peekLast();                   // 获取队列中上次显示的元素
    Frame* peekWritable();               // 获取队列中可写的元素
    void   push();                       // 队列中添加元素
    Frame* peekReadable();               // 获取队列中可读的元素
    void   next();                       // 队列中删除元素
    int    nbRemaining();                // 获取队列中未显示的元素个数
    int64_t  lastPos();                  // 获取队列中上次显示的元素在文件中的位置(字节)
    void unrefItem(Frame *vp);           // 队列中元素解引用,释放元素资源
private:
    virtual void onSerialChanged(int new_serial);
};


#endif