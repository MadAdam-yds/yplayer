#ifndef _AUDIO_SOFTDECODER_H_
#define _AUDIO_SOFTDECODER_H_ 
#include "SoftDecoder.h"
class AudioSoftDecoder:public SoftDecoder {
private:
public:
    AudioSoftDecoder()=default;
    ~AudioSoftDecoder()=default;
    // 对解码后的音频数据进行重采样、声道混合等有关的功能.
};


#endif