#include "net/EventLoop.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <iostream>

using namespace reactor;

int main()
{
    EventLoop         loop;
    static mTimestamp now = mtime();
    loop.run_after(
      [&]() {
          std::cout << "timer task oneshot /*run_after*/ : called after "
                    << static_cast<double>((mtime() - now)) / 1000000 << "s"
                    << " // expected one second later" << std::endl;
      },
      1000);

    loop.run_at(
      [&]() {
          std::cout << "timer task oneshot /*run_at*/ : called after "
                    << static_cast<double>((mtime() - now)) / 1000000 << "s"
                    << " // expected one second later" << std::endl;
      },
      mtime() + 1000);

    loop.run_every(
      [&]() {
          std::cout << "timer task period : called after "
                    << static_cast<double>((mtime() - now)) / 1000000
                    << "s after start/last call "
                    << " // expected period one second" << std::endl;
          now = mtime();
      },
      1000,
      1000);
    loop.loop();
}