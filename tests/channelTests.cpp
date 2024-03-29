#include <gtest/gtest.h>

#include <string>

#include "Channel/channel.hpp"

TEST(make_channel, shouldCreateSenderAndReceiverForBuiltInType) {
    auto [sender, receiver] = make_channel<std::string>();
}

TEST(make_channel, shouldCreateSenderAndReceiverForCustomType) {
    struct Foo {
        int x;
        std::string y;
    };

    auto [sender, receiver] = make_channel<Foo>();
}

TEST(sender, canSendMessage) {
    auto [sender, receiver] = make_channel<int>();
    sender.send(5);
}

TEST(receiver, shouldReceiveMessage) {
    auto [sender, receiver] = make_channel<int>();
    sender.send(5);

    EXPECT_EQ(receiver.recv(), 5);
}

TEST(receiver, shouldNotHangWhenNoSenders) {
    auto [sender, receiver] = make_channel<int>();

    { auto _ = std::move(sender); }

    EXPECT_EQ(receiver.recv(), std::nullopt);
}

TEST(sender, canBeMoved) {
    auto [sender, receiver] = make_channel<std::string>();

    {
        auto s = std::move(sender);
        s.send("Hello");
    }

    EXPECT_EQ(receiver.recv(), "Hello");
    EXPECT_EQ(receiver.recv(), std::nullopt);
}

TEST(receiver, canBeMoved) {
    auto [sender, receiver] = make_channel<int>();

    sender.send(1);
    sender.send(2);

    EXPECT_EQ(receiver.recv(), 1);

    auto moved_receiver = std::move(receiver);

    { auto _ = std::move(sender); }

    EXPECT_EQ(moved_receiver.recv(), 2);
    EXPECT_EQ(moved_receiver.recv(), std::nullopt);
}
