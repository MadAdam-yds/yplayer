#include "Demuxer.h"
#include <libavutil/log.h>
#include <thread>
#include "YPlayerSDK.h"
Demuxer::Demuxer(const string &url,DataStreamContext &ctx):
    paused(false),
    lastPaused(false),
    seekReq(false),
    seekFlags(0),
    seekPos(0),
    seekRel(0),
    url(url),
    readPauseReturn(0),
    realtime(false),
    dsctx(ctx)
{
    demuxerThreadPtr = nullptr;
    for(int i=0;i<sizeof(streamsIndex)/sizeof(streamsIndex);++i){
        streamsIndex[i] = -1;
    }
    _ic = nullptr;
    videoStream = nullptr;
    audioStream = nullptr;
    subtitleStream = nullptr;
}

Demuxer::~Demuxer(){
    if (_ic){
        avformat_close_input(&_ic);
    }
}
bool Demuxer::checkUrl(){
    int ret = avformat_open_input(&_ic, url.c_str(), NULL, NULL);
    if (ret < 0) {
        return false;
    }
    else{
        avformat_close_input(&_ic);
        return true;
    }
}
bool Demuxer::hasStream(enum AVMediaType media){
    return streamsIndex[media]>=0;
}
bool Demuxer::allowTSDiscontinue(){
    return _ic->flags & AVFMT_TS_DISCONT;
}
int Demuxer::init(){
    int ret = YPLAYER_OK;
    _ic = avformat_alloc_context();
    if (!_ic) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return YPLAYER_ERROR_DEMUXER_CONTEXT_ALLOC;
    }
    // 校验url的有效性
    if(!checkUrl()){
        avformat_free_context(_ic);  //释放上下文
        return YPLAYER_ERROR_RESOURCE_INVALID_URL;
    }
    audioQ = std::make_shared<PacketQueue>(dsctx);
    videoQ = std::make_shared<PacketQueue>(dsctx);
    subtitleQ = std::make_shared<PacketQueue>(dsctx);
    // 查找详细的流信息
     ret = avformat_find_stream_info(_ic, nullptr);
     if (ret < 0) {
         av_log(NULL, AV_LOG_WARNING,"%s: could not find codec parameters\n", url.c_str());
         return YPLAYER_ERROR_DEMUXER_FIND_STREAM;
     }
     // 是否是实时流
     realtime = isRealtime();
     if(realtime){
        infiniteBuffer = true;
     }
    // 查找"最佳"匹配流
    //"最佳"视频流
    streamsIndex[AVMEDIA_TYPE_VIDEO] =
    av_find_best_stream(_ic, AVMEDIA_TYPE_VIDEO,streamsIndex[AVMEDIA_TYPE_VIDEO], -1, NULL,0);
    //"最佳"音频流
    streamsIndex[AVMEDIA_TYPE_AUDIO] =
    av_find_best_stream(_ic, AVMEDIA_TYPE_AUDIO,streamsIndex[AVMEDIA_TYPE_AUDIO],streamsIndex[AVMEDIA_TYPE_VIDEO],NULL,0);
    //"最佳"字幕流
    streamsIndex[AVMEDIA_TYPE_SUBTITLE] =
    av_find_best_stream(_ic, AVMEDIA_TYPE_SUBTITLE,                 
                        streamsIndex[AVMEDIA_TYPE_SUBTITLE],
                        (streamsIndex[AVMEDIA_TYPE_AUDIO] >= 0 ?
                        streamsIndex[AVMEDIA_TYPE_AUDIO] :
                        streamsIndex[AVMEDIA_TYPE_VIDEO]),
                        NULL, 0);
    if(streamsIndex[AVMEDIA_TYPE_VIDEO]>=0){
        videoStream = _ic->streams[streamsIndex[AVMEDIA_TYPE_VIDEO]];
    }
    if(streamsIndex[AVMEDIA_TYPE_AUDIO]>=0){
        audioStream = _ic->streams[streamsIndex[AVMEDIA_TYPE_AUDIO]];
    }
    if(streamsIndex[AVMEDIA_TYPE_SUBTITLE]>=0){
        subtitleStream = _ic->streams[streamsIndex[AVMEDIA_TYPE_SUBTITLE]];
    }
    return YPLAYER_OK;
}
int Demuxer::start(){
    if(!demuxerThreadPtr){
        demuxerThreadPtr = std::make_shared<std::thread>(&Demuxer::demuxerThread,this);
        //C++11引入了std::thread，线程对象在析构时必须被join或detach，否则会导致程序终止.
        demuxerThreadPtr->detach();//子线程在后台运行.
    }
    return 0;
}
bool Demuxer::isRealtime(){
    if(!strcmp(_ic->iformat->name, "rtp")
    || !strcmp(_ic->iformat->name, "rtsp")
    || !strcmp(_ic->iformat->name, "sdp")
 )
     return true;

 if(_ic->pb && (   !strncmp(this->url.c_str(), "rtp:", 4)
              || !strncmp(this->url.c_str(), "udp:", 4)
             )
 )
     return true;
 return false;
}

bool Demuxer::streamHasEnoughPackets(AVStream *st, int stream_id, PacketQueue *queue)const
{
    return stream_id < 0 ||
    (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
    queue->getnbPackets() > MIN_FRAMES && (!queue->getDuration() || av_q2d(st->time_base) * queue->getDuration() > 1.0);
}

int Demuxer::packetQueuePutNullpacket(PacketQueue *q, int stream_index){
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return q->put(pkt);
}

void Demuxer::setPaused(bool paused){

}
void Demuxer::streamSeek(int64_t pos, int64_t rel, int seekByBytes){

}
int Demuxer::demuxerThread(void *arg){
    Demuxer* demuxer = (Demuxer*)arg;
    int err = 0;
    int ret = 0;
    AVPacket pkt1, *pkt = &pkt1;

    // 打开输入文件、查找详细的流信息、查找"最佳"匹配流在init初始化阶段完成.
    // 循环读取数据包
    for(;;){
        // 用last_paused和paused是否相同来判断是不是第一次，保证av_read_pause和av_read_play只调用一次
        if (demuxer->paused != demuxer->lastPaused) {
            demuxer->lastPaused = demuxer->paused;
            if (demuxer->paused){
                //暂停网络流,例如RTSP；如果是本地文件返回错误码AVERROR(ENOSYS).
                demuxer->readPauseReturn = av_read_pause(demuxer->_ic);
            }
            else{
                //恢复流
                av_read_play(demuxer->_ic);
            }
        }
        // 暂停状态下的实时流
        #if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
        if (demuxer->paused &&
                (!strcmp(demuxer->_ic->iformat->name, "rtsp") ||
                 (demuxer->_ic->pb && !strncmp(demuxer->url.c_str(), "mmsh:", 5)))) {
            /* wait 10 ms to avoid trying to get another packet */
            /* XXX: horrible */
            SDL_Delay(10);
            continue;
        }
        #endif
        // 处理seek请求
        // FIXME the +-2 is due to rounding being not done in the correct direction in generation of the seekPos/seekRel variables
        // +-2的一个目的是找seek_target附近的IDR帧.
        if (demuxer->seekReq) {
            int64_t seek_target = demuxer->seekPos;
            int64_t seek_min    = demuxer->seekRel > 0 ? seek_target - demuxer->seekRel + 2: INT64_MIN;
            int64_t seek_max    = demuxer->seekRel < 0 ? seek_target - demuxer->seekRel - 2: INT64_MAX;
            ret = avformat_seek_file(demuxer->_ic, -1, seek_min, seek_target, seek_max, demuxer->seekFlags);//seek到时间戳seek_target
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR,"%s: error while seeking\n", demuxer->url);
            }
            else{
                //seek failed then flush packet queue
                if (demuxer->audioStreamIndex>=0) {
                    demuxer->audioQ->flush();
                    //packet_queue_put(&is->audioq, &flush_pkt);//flush_pkt会导致queue的serial改变 q->serial++(表示逻辑流改变)->重置内部解码器,刷新(清空)内部缓存，重置结束标志finished为0;
                    //需要设计新的方法,不用flush_pkt
                }
                if (demuxer->videoStreamIndex>=0) {
                    demuxer->videoQ->flush();
                }
                if (demuxer->subtitleStreamIndex>=0) {
                    demuxer->subtitleQ->flush();
                }
                // 外部时钟在上层应用设置，这里设置不合适.

                demuxer->seekReq = false;
                //demuxer->queue_attachments_req = 1;//附加请求,对应与视频流或音频流相关的附加图片流，例如音频文件中作为封面艺术的图片
                demuxer->eof = false;
                // pause状态和时钟的交互需要换种方法
                // if (demuxer->paused)
                //     step_to_next_frame(is);//暂停状态下seek效果是step
            }

        }
        // 根据缓存情况判断是否继续读取数据包
        // 如果读取的是本地文件,暂停状态下也会将队列读满.如果是实时流则不限制队列大小
         if (demuxer->infiniteBuffer &&
            (demuxer->audioQ->getSize() + demuxer->videoQ->getSize() + demuxer->subtitleQ->getSize() > MAX_QUEUE_SIZE)
          || (demuxer->streamHasEnoughPackets(demuxer->audioStream, demuxer->audioStreamIndex, demuxer->audioQ.get()) &&
              demuxer->streamHasEnoughPackets(demuxer->videoStream, demuxer->videoStreamIndex, demuxer->videoQ.get()) &&
              demuxer->streamHasEnoughPackets(demuxer->subtitleStream, demuxer->subtitleStreamIndex, demuxer->subtitleQ.get()))) {
          /* wait 10 ms */
          std::unique_lock<std::mutex>unilock(demuxer->qmutex);
          demuxer->condContinueDemuxer.wait_for(unilock,std::chrono::seconds(10));
          unilock.unlock();
          continue;
      }
      /* 循环播放或自动退出在上层player中判断 */
      ret = av_read_frame(demuxer->_ic, pkt);
      if (ret < 0) {
          if ((ret == AVERROR_EOF || avio_feof(demuxer->_ic->pb)) && !demuxer->eof) {
              if (demuxer->videoStreamIndex >= 0)
                demuxer->packetQueuePutNullpacket(demuxer->videoQ.get(), demuxer->videoStreamIndex);//插入空包packet
              if (demuxer->audioStreamIndex >= 0)
                demuxer->packetQueuePutNullpacket(demuxer->audioQ.get(), demuxer->audioStreamIndex);//插入空包packet
              if (demuxer->subtitleStreamIndex >= 0)
                demuxer->packetQueuePutNullpacket(demuxer->subtitleQ.get(), demuxer->subtitleStreamIndex);//插入空包packet
              demuxer->eof = 1;
          }
          if (demuxer->_ic->pb && demuxer->_ic->pb->error){//底层IO也报错了
             break;
          }
          std::unique_lock<std::mutex>unilock(demuxer->qmutex);
          demuxer->condContinueDemuxer.wait_for(unilock,std::chrono::seconds(10));
          unilock.unlock();
          continue;
    }
    else{
        demuxer->eof = false;
    }

    /* check if packet is in play range specified by user, then queue, otherwise discard */
    // pkt_in_play_range
    if (pkt->stream_index == demuxer->audioStreamIndex) {
        demuxer->audioQ->put(pkt);//入队
    } 
    else if (pkt->stream_index == demuxer->videoStreamIndex
               && !(demuxer->videoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
        demuxer->videoQ->put(pkt);//入队
    }
    else if (pkt->stream_index == demuxer->subtitleStreamIndex ) {
        demuxer->subtitleQ->put(pkt);//入队
    }
    else {
        av_packet_unref(pkt);//解引用当前pkt
    }

  }
  ret = 0;
  //exit
fail:
  if (demuxer->_ic){
    avformat_close_input(&demuxer->_ic);
  }
  if (ret != 0) {
    #if 0
    SDL_Event event;
    event.type = FF_QUIT_EVENT;
    event.user.data1 = is;
    SDL_PushEvent(&event);
    #endif
}

  return 0;
}