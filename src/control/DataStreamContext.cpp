#include "DataStreamContext.h"
// 获取数据流序列号
int DataStreamContext::getSerial()const
{
    return currentSerial.load();
}
// 数据流序列号递增
void DataStreamContext::incrementSerial(){
  
    currentSerial.fetch_add(1);
    for (auto listener:listeners) {   // 通知所有监听者
        listener->onSerialChanged(currentSerial.load());
    }

}

void DataStreamContext::addListener(ISerialListener* listener) {
    listeners.push_back(listener);
}
