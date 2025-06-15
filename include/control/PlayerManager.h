#ifndef _PLAYER_MANAGER_H_
#define _PLAYER_MANAGER_H_

#include "Player.h"
#include "commonType.h"
#include <map>
#include <mutex>
typedef std::map<uint32_t, Player*> PlayerMap; // 端口与播放器的映射
#define MAX_PORT_NUM  64
class PlayerManager
{
public:
    // 获取单例
    static PlayerManager* instance(){
        if(!pInstance){
            pInstance = new PlayerManager();
        }
        return pInstance;
    }
    ~PlayerManager();
    void setDecodeCallback(DECODE_CALLBACK_PF callback,void *user);
    int getFreePort(uint32_t &port);                // 获取空闲端口
    int releasePort(uint32_t port);                 // 释放端口
    int openFile(const char *url,uint32_t port);    // 打开文件
    int closeFile(uint32_t port);                   // 关闭文件
    int play(uint32_t port);                        // 开始播放
    int pause(uint32_t port,bool pasuseState);      // 暂停或恢复播放器
    int seek(uint32_t port,int64_t pos,int64_t rel);// 跳转seek
    int speed(uint32_t port,ENUM_YPLAYER_PLAY_SPEED speed);//  设置播放速度
    int OneByOne(uint32_t port);                        // 单帧播放
    //...
private:
	PlayerManager()=default;                    // 私有默认构造函数
    PlayerManager(const PlayerManager&)=delete;// 禁用拷贝构造函数
    PlayerManager& operator=(const PlayerManager&)=delete;// 禁用赋值操作
    static PlayerManager*pInstance;             // 私有静态指针
    PlayerMap _playerMap;                        // 播放器列表   
    std::mutex playerMapMutex;                  // 播放器列表的锁
    DECODE_CALLBACK_PF decCallbackPF;           // 解码后数据的回调函数
    void *puser;                                // 解码后数据的回调函数的参数
};


#endif