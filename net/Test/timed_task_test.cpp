#include "base/log.hpp"
#include "base/timestamp.hpp"
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
    size_t counter = 0;
    size_t period_task_called_time = 0;
    MilliTimestamp start = get_milli_timestamp();

    loop.run_after(
        [&]() {
            counter += 10;
            log_trace("running run_after task, counter={}, run after {}ms ", counter, get_milli_timestamp() - start);
        },
        30);

    loop.run_at(
        [&]() {
            counter += 100;
            log_trace("running run_at task, counter={}, run after {}ms ", counter, get_milli_timestamp() - start);
        },
        start + 80);

    loop.run_every(
        [&]() {
            period_task_called_time++;
            counter++;

            if (period_task_called_time <= 3)
                log_trace("running run_every task, counter={}, run after {}ms ", counter,
                          get_milli_timestamp() - start);

            if (period_task_called_time == 1)
            {
                EXPECT_TRUE(counter == 1);
            }
            else if (period_task_called_time == 2)
            {
                EXPECT_TRUE(counter == 12);
            }
            else if (period_task_called_time == 3)
            {
                EXPECT_TRUE(counter == 113);
            }
        },
        50, 0);

    loop.loop(500);
}