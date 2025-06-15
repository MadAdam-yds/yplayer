#include "YPlayerSDK.h"
#include "PlayerManager.h"

// 设置解码数据回调函数
void setDecodeCallback(DECODE_CALLBACK_PF callback,void *user){
    PlayerManager::instance()->setDecodeCallback(callback,user);
}   
// 播放库初始化
int init(){
    return 0;
}
// 播放库反初始化
int uninit(){
    return 0;
}
// 获取空闲端口
int getFreePort(uint32_t &port){
    return PlayerManager::instance()->getFreePort(port);
}
// 释放占用端口
int releasePort(uint32_t port){
    return PlayerManager::instance()->releasePort(port);
}
// 打开文件
int openFile(const char *url,uint32_t port){
    return PlayerManager::instance()->openFile(url,port);
}
// 关闭文件
int closeFile(uint32_t port){
    return PlayerManager::instance()->closeFile(port);
}
// 开始播放
int play(uint32_t port){
    return PlayerManager::instance()->play(port);
}
// 暂停/恢复播放
int pause(uint32_t port,bool pause){
    return PlayerManager::instance()->pause(port,pause);
}
// 跳播(回放)pos表示时间秒,rel是相对时间秒
int seek(uint32_t port,int64_t pos,int64_t rel){
    return PlayerManager::instance()->seek(port,pos,rel);
}
// 倍速播放
int speed(uint32_t port,ENUM_YPLAYER_PLAY_SPEED speed){
    return PlayerManager::instance()->speed(port,speed);
}
// 单帧播放
int OneByOne(uint32_t port){
    return PlayerManager::instance()->OneByOne(port);
}