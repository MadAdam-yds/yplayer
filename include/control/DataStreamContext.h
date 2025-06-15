#ifndef _DataStreamContext_H_
#define _DataStreamContext_H_
#include <atomic>
#include <vector>
/*
 * @brief  数据流上下文
 * @note
 * @version V1.0
 * @date 2025-05-10
 * @author YuDeShui
 * @attention
 * Copyright (c) 2025 YuDeShui.  All rights reserved.
 * 负责管理流序列号serial等业务状态.
 */
// 观察者接口
class ISerialListener {
public:
    virtual void onSerialChanged(int new_serial) = 0;
};
// 现在的逻辑比较简单,如果业务变得复杂应该将DataStreamContext定义为抽象接口类
// 然后通过依赖注入的方法注入到其它类中,例如PacketQueue,VideoSoftDecoder,FrameQueue,Clock等.
class DataStreamContext {
public:
    DataStreamContext() = default;
    ~DataStreamContext() = default;
    int getSerial()const;// 获取数据流序列号
    // 数据流序列号递增
    void incrementSerial();
    bool validSerial(int serial){return serial == currentSerial.load();}
    void addListener(ISerialListener* listener);

private:
    std::atomic<int> currentSerial; // 当前数据流序列号
    std::vector<ISerialListener*> listeners;// 观察者列表
};
#endif