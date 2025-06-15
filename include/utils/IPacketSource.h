#ifndef _IPACKETSOURCE_H_
#define _IPACKETSOURCE_H_

#include <libavformat/avformat.h>

class IPacketSource {
public:
    virtual bool isEmpty() = 0;
    virtual int  getPacket(AVPacket *pkt,bool block,int &serial) = 0;
    // 定义一个虚析构函数，确保派生类的析构函数可以被正确调用
    virtual ~IPacketSource() = default;
};

#endif