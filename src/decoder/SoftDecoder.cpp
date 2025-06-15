#include "SoftDecoder.h"
#include <thread>
#include "libavutil/avutil.h"
#ifdef __linux__
#include <pthread.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#if 0
SoftDecoder::SoftDecoder():
    avfctx(nullptr),
    avctx(nullptr),
    codec(nullptr),
    dataSource(nullptr),
    streamIndex(-1)
{
    decodeThreadPtr = nullptr;
    frameDropsLate = 0;
}
#endif
SoftDecoder::SoftDecoder(DataStreamContext & ctx):
    avfctx(nullptr),
    avctx(nullptr),
    codec(nullptr),
    dataSource(nullptr),
    streamIndex(-1),
    _frameQueue(ctx),
    dsctx(ctx)
{
    decodeThreadPtr = nullptr;
    frameDropsLate = 0;
    
}

// 获取队列中丢帧个数
int SoftDecoder::FrameDropsLate(){
    return frameDropsLate;
}
 // 队列中丢帧个数加1                
void SoftDecoder::FrameDropsLateInc(){
    frameDropsLate++;
}             
int SoftDecoder::init(IPacketSource *pDataSource,AVFormatContext *pAvfctx,int stIndex){
    int ret = 0;
    this->dataSource=pDataSource;
    this->avfctx=pAvfctx;
    this->streamIndex=stIndex;
    this->stream = pAvfctx->streams[stIndex];
    this->_frameQueue.init(MAX_FRAME_QUEUE_SIZE,1);
    // 分配解码器上下文
    if(!(avctx=avcodec_alloc_context3(NULL))){
        av_log(NULL, AV_LOG_FATAL,"avcodec alloc context failed\n");
        return AVERROR(ENOMEM);
    }
    // 设置avctx中的部分参数,和codecpar保持一致,之后可以find到正确的解码器.
    if(!(ret=avcodec_parameters_to_context(avctx, pAvfctx->streams[stIndex]->codecpar))){
        av_log(NULL, AV_LOG_FATAL,"set avcodec context parameters failed\n");
        goto fail;
    }
    // 设置解码器的timebase
    avctx->pkt_timebase = pAvfctx->streams[stIndex]->time_base;
    // 查找对应的解码器,codec_id对应编码数据的类型,例如为 AV_CODEC_ID_H264\AV_CODEC_ID_AAC\AV_CODEC_ID_MP3
    if(!(codec=avcodec_find_decoder(avctx->codec_id))){
        av_log(NULL, AV_LOG_WARNING,
            "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = AVERROR(EINVAL);
        goto fail;
    }
    // 多余么?本来就是相等的.(目的是提高鲁棒性,avctx->codec_id 可能未正确设置或被篡改了)
    avctx->codec_id = codec->id;
    // 设置丢帧方式
    pAvfctx->streams[stIndex]->discard = AVDISCARD_DEFAULT;
    goto out;

#if 0
    // lowres表示低分辨率模式,用户可指定低分辨率模式等级0:正常分辨率1:1/2分辨率2：1/4分辨率3：1/8分辨率.
    // 解码器参数max_lowRes表示是否支持低分辨率模式,由编解码器内部定义,反映硬件/算法能力,不支持用户修改
    if (stream_lowres > codec->max_lowres) {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
                codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;
    // 快速模式fast,可提高性能但是图像质量会下降.
    if (fast)
    avctx->flags2 |= AV_CODEC_FLAG2_FAST;//快速解码模式,可降低资源使用率
#endif

#if 0
    // 上层Player处理.
    // 初始化上次使用的流;用argv指定解码器初始化forced_codec_name
    switch(avctx->codec_type){
        case AVMEDIA_TYPE_AUDIO   : is->last_audio_stream    = stIndex; break;
        case AVMEDIA_TYPE_SUBTITLE: is->last_subtitle_stream = stIndex; break;
        case AVMEDIA_TYPE_VIDEO   : is->last_video_stream    = stIndex; break;
    }
    // 如果用户指定了解码器，则优先使用指定的解码器
    if (forced_codec_name)
        codec = avcodec_find_decoder_by_name(forced_codec_name);
#endif

fail:
    avcodec_free_context(&avctx);
out:
    return ret;
}
Frame* SoftDecoder::peekReadable(){
    return _frameQueue.peekReadable();
}               
void SoftDecoder::next(){
    _frameQueue.next();
} 
Frame* SoftDecoder::peekLast(){
    return _frameQueue.peekLast();
}
Frame* SoftDecoder::peekNext(){
    return _frameQueue.peekNext();
}
Frame* SoftDecoder::peek(){
    return _frameQueue.peek();
}
int SoftDecoder::nbRemaining(){
     // 获取队列中剩余元素个数
     return _frameQueue.nbRemaining();
}                  
void SoftDecoder::start(){
    if(decodeThreadPtr){
        decodeThreadPtr = make_shared<std::thread>(decodeThread,this);
        decodeThreadPtr->detach();
    }
}
int SoftDecoder::decodeThread(void *arg){
    int ret=0;
    int pktSerial = 0;
    SoftDecoder *d = (SoftDecoder *)arg;
    double pts;
    double duration;
    AVRational tb = d->stream->time_base;
    //根据已有信息调整帧率,有些情况下可能会调整
    AVRational frame_rate = av_guess_frame_rate(d->avfctx, d->stream, NULL);
    while(true){
        AVPacket pkt;
        // 创建frame,分配内存并初始化参数
        AVFrame *frame = av_frame_alloc();
        do{
            //从解码器获取一帧解码后的Frame,并放入frameQueue队列中.
            ret = avcodec_receive_frame(d->avctx, frame);
            if (ret >= 0) {
                // 更新frame的pts
                frame->pts = frame->best_effort_timestamp;
                // 送入Frame Queue
                Frame *vp = nullptr;
                if(!(vp=d->_frameQueue.peekWritable())){
                    return -1;
                }
                // 该帧的显示持续时间
                duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
                // 该帧的显示时间点
                pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
                vp->sar = frame->sample_aspect_ratio;
                vp->uploaded = 0;
                vp->width = frame->width;
                vp->height = frame->height;
                vp->format = frame->format;
                vp->pts = pts;
                vp->duration = duration;
                vp->pos = frame->pkt_pos;
                // vp指向src_frame
                av_frame_move_ref(vp->frame, frame);
                // 入队 vp
                d->_frameQueue.push();
                // 释放frame引用
                av_frame_unref(frame);

            } 
            if (ret == AVERROR_EOF) {
                //d->finished = d->pkt_serial;//结束时，将解码器finished标志置为pkt_serial
                avcodec_flush_buffers(d->avctx);
                return 0;
            }
            if (ret >= 0)
                return 1;
        }while(ret!=AVERROR(EAGAIN));

        do {
            if (d->packetPending) {
                av_packet_move_ref(&pkt, d->pendingPkt);
                d->packetPending = 0;
            } 
            else {
                //(阻塞方式)从packet队列中获取一个pkt
                // 如果获取的packet对应的数据流序号serial比较旧,则需要丢弃该包.
                if (d->dataSource->getPacket(&pkt, true,pktSerial) < 0)
                    return -1;
            }
        } while (!d->dsctx.validSerial(pktSerial));//解码器d只处理和当前pkt_serial一致的包,d->queue->serial表示当前d->queue中最新数据的逻辑流,可能先更新了,那么之前缓存的旧packet直接覆盖,不再解码
        // send pkt
        if (avcodec_send_packet(d->avctx, &pkt) == AVERROR(EAGAIN)) {
            av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
            d->packetPending = 1;
            av_packet_move_ref(d->pendingPkt, &pkt);
        }
        // 解引用当前pkt
        av_packet_unref(&pkt);

    }
}