#include "gtest/gtest.h"
#include "fmt/core.h"

TEST(ThirdPartyTest, FmtFunc) {
    fmt::print("this is {} \n", "fmt print");
    EXPECT_EQ(fmt::format("Hello, {}", "World!"), "Hello, World!");
    EXPECT_EQ(fmt::format("1 + 2 = {}", 3), "1 + 2 = 3");
    EXPECT_EQ(fmt::format("{} * {} = {}", 0.5, 2, 1), "0.5 * 2 = 1");
}