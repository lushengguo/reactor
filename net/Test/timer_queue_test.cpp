#include "net/EventLoop.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace reactor;

int main()
{
    EventLoop loop;
    loop.init();
    loop.run_after([]() { std::cout << "print after 1 second" << std::endl; },
      1000000);
    loop.loop();
}