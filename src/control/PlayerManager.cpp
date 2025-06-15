#include "PlayerManager.h"
#include "YPlayerSDK.h"
PlayerManager* PlayerManager::pInstance = nullptr;

void PlayerManager::setDecodeCallback(DECODE_CALLBACK_PF callback,void *user){
    decCallbackPF = callback;
    puser = user;
}
// 获取空闲端口
int PlayerManager::getFreePort(uint32_t &port){
    std::lock_guard<std::mutex>guard(playerMapMutex);
    int ret = YPLAYER_ERROR_RESOURCE_NO_FREE_PORT;
    for(int i = 0; i < MAX_PORT_NUM; i++){
        if(_playerMap.find(i) == _playerMap.end()){
            port = i;
            _playerMap[i] = new Player();//  创建播放器
            _playerMap[i]->setPort(i);
            ret = YPLAYER_OK;
        }
    }
    return ret;
}
int PlayerManager::releasePort(uint32_t port){
    std::lock_guard<std::mutex>guard(playerMapMutex);
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    if(_playerMap.find(port) != _playerMap.end()){
        delete _playerMap[port]; // 释放播放器
        _playerMap.erase(port);  // 从播放器列表中删除
        ret =  YPLAYER_OK;
    }
    return ret;
}                 
int PlayerManager::openFile(const char *url,uint32_t port){
    std::lock_guard<std::mutex>guard(playerMapMutex);
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){ // 查找对应的解码器
        if(it->second->checkUrl(url)==false){ // 播放器检验url
            ret = YPLAYER_ERROR_RESOURCE_INVALID_URL;
        }   
    }
    return ret;
}
int PlayerManager::closeFile(uint32_t port){
    std::lock_guard<std::mutex>guard(playerMapMutex);
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){ // 查找对应的解码器
        ret = it->second->close();// 关闭播放器
        _playerMap.erase(it);
    }
    return ret;
}
int PlayerManager::play(uint32_t port){
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    std::lock_guard<std::mutex>guard(playerMapMutex);
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){
        ret = it->second->start();
    }
    return ret;
}
int PlayerManager::pause(uint32_t port,bool pasuseState){
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    std::lock_guard<std::mutex>guard(playerMapMutex);
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){
        ret = it->second->pause(pasuseState);
    }
    return ret;
}
int PlayerManager::seek(uint32_t port,int64_t pos,int64_t rel){
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    std::lock_guard<std::mutex>guard(playerMapMutex);
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){
        ret = it->second->seek(pos,rel);    // 播放器seek
    }
    return ret;
}
int PlayerManager::speed(uint32_t port,ENUM_YPLAYER_PLAY_SPEED speed){
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    std::lock_guard<std::mutex>guard(playerMapMutex);
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){
        ret = it->second->speed(speed); 
    }
    return ret;
}

int PlayerManager::OneByOne(uint32_t port){
    int ret = YPLAYER_ERROR_RESOURCE_INVALID_PORT;
    std::lock_guard<std::mutex>guard(playerMapMutex);
    PlayerMap::iterator it = _playerMap.begin();
    if((it=_playerMap.find(port)) != _playerMap.end()){
        ret = it->second->OneByOne(); 
    }
    return ret;
}