#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "PacketQueue.h"

// 模拟 AVPacket
struct AVPacket {
    int size;
    int64_t duration;
};

// 测试用例：pkt 为 nullptr
TEST(PacketQueueTest, Put_NullPacket_ReturnsNegativeOne) {
    PacketQueue queue;
    queue.init();
    int result = queue.put(nullptr);
    EXPECT_EQ(result, -1);
}

// 测试用例：空队列
TEST(PacketQueueTest, Put_EmptyQueue_AddsPacket) {
    PacketQueue queue;
    queue.init();
    AVPacket pkt = {100, 500};
    int result = queue.put(&pkt);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(queue.nbPackets, 1);
    EXPECT_EQ(queue.size, 100);
    EXPECT_EQ(queue.duration, 500);
}

// 测试用例：非空队列
TEST(PacketQueueTest, Put_NonEmptyQueue_AddsPacketToEnd) {
    PacketQueue queue;
    queue.init();
    AVPacket pkt1 = {100, 500};
    AVPacket pkt2 = {200, 1000};
    queue.put(&pkt1);
    int result = queue.put(&pkt2);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(queue.nbPackets, 2);
    EXPECT_EQ(queue.size, 300);
    EXPECT_EQ(queue.duration, 1500);
}

// 测试用例：线程安全
TEST(PacketQueueTest, Put_MultipleThreads_AddsPacketsSafely) {
    PacketQueue queue;
    queue.init();
    std::vector<AVPacket> packets = {{100, 500}, {200, 1000}, {300, 1500}};
    std::vector<std::thread> threads;

    for (auto& pkt : packets) {
        threads.emplace_back([&queue, &pkt]() {
            queue.put(&pkt);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(queue.nbPackets, 3);
    EXPECT_EQ(queue.size, 600);
    EXPECT_EQ(queue.duration, 3000);
}

// 测试用例：通知
TEST(PacketQueueTest, Put_NotifyConditionVariable) {
    PacketQueue queue;
    queue.init();
    std::mutex waitMutex;
    std::condition_variable waitCond;
    bool notified = false;

    std::thread waitThread([&queue, &waitMutex, &waitCond, &notified]() {
        std::unique_lock<std::mutex> lock(waitMutex);
        waitCond.wait(lock, [&notified] { return notified; });
        queue.get(nullptr, 0);
    });

    AVPacket pkt = {100, 500};
    queue.put(&pkt);

    {
        std::lock_guard<std::mutex> lock(waitMutex);
        notified = true;
    }
    waitCond.notify_one();

    waitThread.join();
    EXPECT_EQ(queue.nbPackets, 0);
}
int _tmain(int argc,char * argv[])
{
        testing::InitGoogleTest(&argc,argv);
        return RUN_ALL_TESTS();
}
