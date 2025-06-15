#ifndef _VIDEO_SOFT_DECODER_H_
#define _VIDEO_SOFT_DECODER_H_
#include "SoftDecoder.h"

class VideoSoftDecoder:public SoftDecoder {
private:
public: 
    VideoSoftDecoder(DataStreamContext & ctx);
    ~VideoSoftDecoder()=default;
    // 对解码后Frame色彩空间转换、缩放、剪裁有关的功能 sws_scale
};

#endif