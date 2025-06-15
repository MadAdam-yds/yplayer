#ifndef _YPLAYERSDK_H_
#define _YPLAYERSDK_H_
#include "commonType.h"
// extern "C":编译规则,禁用C++名称修饰（Name Mangling），保证符号的C语言ABI兼容性，ABI表示一些二进制级别的规范.
// __declspec(dllexport):链接规则,声明符号来自外部dll.
#if defined(_WIN32) || defined(_WIN64)
    #define YPLAYER_API  extern "C" __declspec(dllexport) 
    #define CALLBACK __stdcall
#elif defined(__linux__)
    #define YPLAYER_API  extern "C"
    #define CALLBACK  
#endif 
/****** 全局错误码开始 ******/
#define YPLAYER_OK                  0
#define YPLAYER_ERROR_BASE          1
#define YPLAYER_ERROR_RESOURCE_NO_FREE_PORT  (YPLAYER_ERROR_BASE + 1)   // 没有可用的端口号
#define YPLAYER_ERROR_RESOURCE_INVALID_PORT  (YPLAYER_ERROR_BASE + 2)   // 无效的端口号
#define YPLAYER_ERROR_RESOURCE_INVALID_URL   (YPLAYER_ERROR_BASE + 3)   // 无效的URL地址
//demuxer
#define YPLAYER_ERROR_BASE_DEMUXER          (YPLAYER_ERROR_BASE + 30)
#define YPLAYER_ERROR_DEMUXER_CONTEXT_ALLOC (YPLAYER_ERROR_BASE_DEMUXER + 1)   // 上下文申请失败
#define YPLAYER_ERROR_DEMUXER_FIND_STREAM   (YPLAYER_ERROR_BASE_DEMUXER + 2)   // 查找流信息失败

/****** 全局错误码结束 ******/

/****** 枚举定义开始 ******/
typedef enum {
    PLAY_SPEED_NORMAL   = 0,     // 正常播放
    PLAY_SPEED_FAST_2X  = 1,     // 2倍快播
    PLAY_SPEED_FAST_4X  = 2,     // 4倍快播
    PLAY_SPEED_FAST_8X  = 3,     // 8倍快播
    PLAY_SPEED_FAST_16X = 4,     // 16倍快播
    PLAY_SPEED_SLOW_2X  = 10,    // 2倍慢播
    PLAY_SPEED_SLOW_4X  = 11,    // 4倍慢播
    PLAY_SPEED_SLOW_8X  = 12,    // 8倍慢播
    PLAY_SPEED_SLOW_16X = 13,    // 16倍慢播
} ENUM_YPLAYER_PLAY_SPEED;
/****** 枚举定义结束 ******/

/****** 结构体定义开始 ******/
// 媒体数据头信息
typedef struct {
    int mediaType;      // 媒体类型 0:音频 1:视频
    int64_t pts;        // 时间戳
    uint32_t width;     // 图像宽度
    uint32_t height;    // 图像高度
    uint32_t format;    // 图像格式 0:YUV420P 1:YUV420P10LE
}AV_HEADER;
/****** 结构体定义结束 ******/

// 解码数据回调函数
typedef void (CALLBACK *DECODE_CALLBACK_PF)(uint32_t port, AV_HEADER header,void* data,uint32_t dataLen,void *user);
// 设置解码数据回调函数
YPLAYER_API void setDecodeCallback(DECODE_CALLBACK_PF callback,void *user);
// 播放库初始化
YPLAYER_API int init();
// 播放库反初始化
YPLAYER_API int uninit();
// 获取空闲端口
YPLAYER_API int getFreePort(uint32_t &port);
// 释放占用端口
YPLAYER_API int releasePort(uint32_t port);
// 打开文件
YPLAYER_API int openFile(const char *url,uint32_t port);
// 关闭文件
YPLAYER_API int closeFile(uint32_t port);
// 开始播放
YPLAYER_API int play(uint32_t port);
// 暂停/恢复播放
YPLAYER_API int pause(uint32_t port,bool pauseState);
// 跳播(回放)pos表示时间秒,rel是相对时间秒
YPLAYER_API int seek(uint32_t port,int64_t pos,int64_t rel);
// 倍速播放
YPLAYER_API int speed(uint32_t port,ENUM_YPLAYER_PLAY_SPEED speed);
// 单帧播放,调用后进入单帧播放模式,调用pause恢复播放后会进入正常播放模式
YPLAYER_API int OneByOne(uint32_t port);
#endif