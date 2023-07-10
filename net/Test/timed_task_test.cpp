#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>

using namespace reactor;

TEST(reactor, timed_task)
{
    EventLoop loop;
    // fmt::print("hello");
    static MicroTimeStamp now = micro_timestamp();
    // EventLoop::TimerId id1, id2, id3;
    loop.run_after(
        [&]() {
            log_trace("timer task oneshot /*run_after*/ : called after {}s // expected one second later",
                       static_cast<double>((micro_timestamp() - now)) / 1000000);
        },
        1000);

    // id2 = loop.run_at(
    //     [&]() {
    //         std::cout << "timer task oneshot /*run_at*/ : called after "
    //                   << static_cast<double>((micro_timestamp() - now)) / 1000000 << "s"
    //                   << " // expected one second later" << std::endl;
    //     },
    //     micro_timestamp() + 1000);

    // id3 = loop.run_every(
    //     [&]() {
    //         std::cout << "timer task period : called after " << static_cast<double>((micro_timestamp() - now)) /
    //         1000000
    //                   << "s after start/last call "
    //                   << " // expected period one second" << std::endl;
    //         now = micro_timestamp();
    //     },
    //     1000, 1000);
    loop.loop();
}