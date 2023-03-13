#include "base/mixed.hpp"
#include <gtest/gtest.h>
#include <string>
using namespace reactor;

bool test_replace_all()
{
    std::string s("abcdefgh\r\n");
    auto s2 = replace_all(s, "abcdefg", "");
    if (s2 != "h\r\n")
        return false;

    auto s3 = replace_all(s2, "", "flangkbakby");
    if (s2 != s3)
        return false;

    auto s4 = replace_all(s3, "\r", "\\r");
    if (s4 != "h\\r\n")
        return false;

    auto s5 = replace_all(s4, "h", "h");
    if (s4 != s5)
        return false;

    return true;
}

TEST(string, replace_all) { EXPECT_TRUE(test_replace_all()); }