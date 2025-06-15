#include <gtest/gtest.h>
#include "utils/PacketQueue.h"

// 测试初始化方法
TEST(PacketQueueTest, InitTest) {
    PacketQueue queue;
    queue.init();
    // 验证初始化后的状态
    EXPECT_EQ(queue.nbPackets, 0);
    EXPECT_EQ(queue.size, 0);
    EXPECT_EQ(queue.duration, 0);
}

// 测试放入和获取 packet
TEST(PacketQueueTest, PutAndGetTest) {
    PacketQueue queue;
    queue.init();

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)av_malloc(10);
    pkt.size = 10;
    pkt.duration = 100;

    // 放入一个 packet
    EXPECT_EQ(queue.put(&pkt), 0);

    // 获取一个 packet
    AVPacket pkt2;
    av_init_packet(&pkt2);
    EXPECT_EQ(queue.get(&pkt2, 0), 1);

    // 验证获取的 packet 是否正确
    EXPECT_EQ(pkt2.size, 10);
    EXPECT_EQ(pkt2.duration, 100);

    // 释放资源
    av_packet_unref(&pkt2);
    av_free(pkt.data);
}

// 测试清空队列
TEST(PacketQueueTest, FlushTest) {
    PacketQueue queue;
    queue.init();

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)av_malloc(10);
    pkt.size = 10;
    pkt.duration = 100;

    // 放入一个 packet
    EXPECT_EQ(queue.put(&pkt), 0);

    // 清空队列
    queue.flush();

    // 验证队列是否为空
    EXPECT_EQ(queue.nbPackets, 0);
    EXPECT_EQ(queue.size, 0);
    EXPECT_EQ(queue.duration, 0);

    // 释放资源
    av_free(pkt.data);
}

// 测试销毁队列
TEST(PacketQueueTest, DestroyTest) {
    PacketQueue queue;
    queue.init();

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)av_malloc(10);
    pkt.size = 10;
    pkt.duration = 100;

    // 放入一个 packet
    EXPECT_EQ(queue.put(&pkt), 0);

    // 销毁队列
    queue.destroy();

    // 验证队列是否为空
    EXPECT_EQ(queue.nbPackets, 0);
    EXPECT_EQ(queue.size, 0);
    EXPECT_EQ(queue.duration, 0);

    // 释放资源
    av_free(pkt.data);
}

// 测试退出方法
TEST(PacketQueueTest, AbortTest) {
    PacketQueue queue;
    queue.init();

    // 调用退出方法
    queue.abort();

    // 验证队列状态（这里没有具体的状态可以验证，但可以确保没有崩溃）
    EXPECT_EQ(queue.nbPackets, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}