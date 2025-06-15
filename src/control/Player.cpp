#include "Player.h"
#include "YPlayerSDK.h"
#include "libavutil/imgutils.h"
DECODE_CALLBACK_PF Player::decCallbackPF = nullptr; 
Player::Player(){
    workState = YPLAYER_STATE_IDLE;
    vidCallbackThreadPtr = nullptr;
    audCallbackThreadPtr = nullptr;
    pdemuxer = nullptr;
    _dataStreamCtx  = std::make_shared<DataStreamContext>();
    _vidSoftDecoder = std::make_unique<VideoSoftDecoder>(*_dataStreamCtx);
    _vidSoftDecoder = nullptr;
    _audSoftDecoder = nullptr;
}
Player::~Player(){

}
bool Player::checkUrl(string url){
    // 检查url
    pdemuxer = std::make_unique<Demuxer>(url,*_dataStreamCtx);
    if(pdemuxer->init()!=YPLAYER_OK){
        return false;
    }
    return true;
}   
// 关闭播放器      
int Player::close(){
    return true;
}
int Player::start(){
    int ret = YPLAYER_OK;
    //AVFMT_TS_DISCONT用于直播流场景,特点是,ts频繁不连续，短时跳跃,设置较小的10秒可快速重置异常状态.
    //其它本地文件场景:特点是ts严格连续，可能长间隔,设置为较大的3600可兼容超长静态画面.		
    double maxFrameDuration = pdemuxer->allowTSDiscontinue() ? 10.0 : 3600.0;
    syncMg.setMaxFrameDuration(maxFrameDuration);
    // 根据输入文件上下文决定开启音频、视频线程
    // 开启视频数据回调线程
    if(pdemuxer && pdemuxer->hasStream(AVMEDIA_TYPE_VIDEO) &&!vidCallbackThreadPtr){
        vidCallbackThreadPtr = make_shared<std::thread>(vidCallbackThreadFunc,this);
        vidCallbackThreadPtr->detach();
    }
    // 开启音频数据回调线程
    if(pdemuxer && pdemuxer->hasStream(AVMEDIA_TYPE_AUDIO) && !audCallbackThreadPtr){
        audCallbackThreadPtr = make_shared<std::thread>(audCallbackThreadFunc,this);
        audCallbackThreadPtr->detach();
    }
    return ret;
}
int Player::pause(bool pasuseState){
    int ret = YPLAYER_OK;
    return ret;
}
int Player::seek(int64_t pos,int64_t rel){
    int ret = YPLAYER_OK;
    return ret;
}
int Player::speed(ENUM_YPLAYER_PLAY_SPEED speed){
    int ret = YPLAYER_OK;
    return ret;
}
int Player::OneByOne(){
    int ret = YPLAYER_OK;
    return ret;
}
void Player::streamTogglePause(){
    if (paused) {
        frameTimer += av_gettime_relative() / 1000000.0 - syncMg.getLastUpdated(CLOCK_VIDEO);
        syncMg.setClockPasuse(CLOCK_VIDEO,0);
        syncMg.setClock(CLOCK_VIDEO,syncMg.getClock(CLOCK_VIDEO));
    }
    syncMg.setClock(CLOCK_EXTERNAL,syncMg.getClock(CLOCK_EXTERNAL));

    //反转paused状态
    paused = !paused;
    syncMg.setClockPasuse(CLOCK_AUDIO,paused);
    syncMg.setClockPasuse(CLOCK_VIDEO,paused);
    syncMg.setClockPasuse(CLOCK_EXTERNAL,paused);
}  
void Player::setDecodeCallback(DECODE_CALLBACK_PF callback,void *user){
    decCallbackPF = callback;
    puser = user;
}
// 视频Frame回调函数
int Player::vidCallbackThreadFunc(void *arg){
    double time;
    Player *pPlayer = (Player*)arg;
    int ret = YPLAYER_OK;
    pPlayer->workState = YPLAYER_STATE_PLAYING;
    double remaining_time = 0.0;
    AV_HEADER header={0};// 数据回调头部信息
    int buffer_size=0;
    while(pPlayer->workState!=YPLAYER_STATE_STOP){
    //未结束
        if(pPlayer->workState==YPLAYER_STATE_PAUSE){
            av_usleep(10000);
            continue;
        }
        // 获取并回调视频数据
        if(pPlayer->_vidSoftDecoder->nbRemaining()>0){
            remaining_time = 0.0;
            if (remaining_time > 0.0){
                av_usleep((int64_t)(remaining_time * 1000000.0));//控制播放显示的时间点
            }
            double last_duration, duration, delay;
            Frame *vp, *lastvp;
            //先peek视频帧,根据同步机制判断是否使用.
            lastvp = pPlayer->_vidSoftDecoder->peekLast();
            vp = pPlayer->_vidSoftDecoder->peek();
            if (vp->serial != pPlayer->serial) {
                pPlayer->_vidSoftDecoder->next();//丢弃该Frame
                continue;
            }
            if (lastvp->serial != vp->serial){
                pPlayer->frameTimer = av_gettime_relative() / 1000000.0;// 更新frameTimer
            }
            last_duration = pPlayer->syncMg.vpDuration(lastvp, vp);// 和上一帧之前pts差值,上一帧的持续时间.
            delay = pPlayer->syncMg.computeTargetDelay(last_duration);//同步调整后的实际延迟
            //继续
            time = av_gettime_relative()/1000000.0;
            if (time < pPlayer->frameTimer + delay) {//未到达显示时间点
                remaining_time = FFMIN(pPlayer->frameTimer + delay - time, remaining_time);//更新等待剩余时间remaining_time
                continue;
            }
            //已经到达了显示时间点
            pPlayer->frameTimer += delay;
            if (delay > 0 && time - pPlayer->frameTimer > AV_SYNC_THRESHOLD_MAX)//超过最大阈值，则重置frame_timer
            {
                pPlayer->frameTimer = time;
            }
            //更新视频时钟以及判断是否触发修正外部时钟
            if (!isnan(vp->pts)){
                pPlayer->syncMg.setClock(CLOCK_VIDEO,vp->pts);//每帧都会同步视频时钟的的pts
                pPlayer->syncMg.syncClockToSlave(CLOCK_EXTERNAL,pPlayer->syncMg.getClockObj(CLOCK_VIDEO));//修正外部时钟
            }
            //当前帧已经过时，则丢弃
            if (pPlayer->_vidSoftDecoder->nbRemaining() > 1) {
                Frame *nextvp = pPlayer->_vidSoftDecoder->peekNext();//当前Frame的下一个Frame
                duration = pPlayer->syncMg.vpDuration(vp, nextvp);//pts差值，当前帧的持续时间
                //如果不是单帧播放&同步方式不是以视频为准&当前帧已过时
                if(!pPlayer->step &&  pPlayer->syncMg.getMasterSyncType()!= AV_SYNC_VIDEO_MASTER && time > pPlayer->frameTimer + duration){
                    pPlayer->_vidSoftDecoder->FrameDropsLateInc();//framedrop:drop frames when cpu is too slow
                    pPlayer->_vidSoftDecoder->next();//丢弃该帧
                    continue;
                }
            }

            if (pPlayer->pdemuxer->hasStream(AVMEDIA_TYPE_SUBTITLE)) {
                //存在字幕流
                //待添加
            }

            pPlayer->_vidSoftDecoder->next();//当前frame出队
            pPlayer->forceRefresh = 1;//刷新
            //单帧播放时,一帧播放后需要保持暂停状态.
            if (pPlayer->step && !pPlayer->paused){
                pPlayer->streamTogglePause();//反转pause
            }
            //数据回调
            header.mediaType = 1;
            header.pts = vp->pts;
            header.width = vp->frame->width;
            header.height = vp->frame->height;
            header.format = vp->frame->format;
            if(decCallbackPF){
                //动态计算帧大小
                buffer_size = av_image_get_buffer_size((AVPixelFormat)(vp->frame->format), vp->frame->width, vp->frame->height, 1);
                decCallbackPF(pPlayer->port,header,vp->frame->data,buffer_size,pPlayer->puser);
            }
            pPlayer->forceRefresh = 0;
        }
    }

}
// 音频sample回调函数
int Player::audCallbackThreadFunc(void *arg){

}
