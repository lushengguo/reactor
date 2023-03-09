#include "net/EventLoop.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <iostream>

using namespace reactor;

int main()
{
    EventLoop         loop;
    static MicroTimeStamp now = micro_timestamp();
    loop.run_after(
        [&]() {
            std::cout << "timer task oneshot /*run_after*/ : called after " << static_cast<double>((micro_timestamp() - now)) / 1000000 << "s"
                      << " // expected one second later" << std::endl;
        },
        1000);

    loop.run_at(
        [&]() {
            std::cout << "timer task oneshot /*run_at*/ : called after " << static_cast<double>((micro_timestamp() - now)) / 1000000 << "s"
                      << " // expected one second later" << std::endl;
        },
        micro_timestamp() + 1000);

    loop.run_every(
        [&]() {
            std::cout << "timer task period : called after " << static_cast<double>((micro_timestamp() - now)) / 1000000 << "s after start/last call "
                      << " // expected period one second" << std::endl;
            now = micro_timestamp();
        },
        1000, 1000);
    loop.loop();
}