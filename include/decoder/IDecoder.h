#ifndef _IDecoder_H_
#define _IDecoder_H_ 
#include "IPacketSource.h"
#include <memory>
class IDecoder
{
public:
    enum MediaType{MEDIA_TYPE_VIDEO,MEDIA_TYPE_AUDIO,MEDIA_TYPE_SUBTITLE};
    IDecoder()=default;
    virtual ~IDecoder()=default;
    IDecoder(const IDecoder&)=delete;               // 禁止复制构造
    IDecoder operator=(const IDecoder&)=delete;     // 禁止赋值构造拷贝
    virtual int init(IPacketSource *pDataSource,AVFormatContext *pAvfctx,int stIndex) = 0; // 初始化接口
    virtual void start() = 0;                       // 开始解码接口
    // 工厂方法
    static std::unique_ptr<IDecoder> create(MediaType type, bool use_hw);// 创建解码器

};

#endif