#include <gtest/gtest.h>
#include "space.h"

using namespace logger;

TEST(SpaceClassTest, ConstructFunc) {
    space::B s1;
    EXPECT_EQ(s1.count(), 0);

    space::MB s2(10);
    EXPECT_EQ(s2.count(), 10);

    space::GB s3(20);
    EXPECT_EQ(s3.count(), 20);

    space::TB s4(30);
    EXPECT_EQ(s4.count(), 30);
}

TEST(SpaceClassTest, CastFunc) {
    space::B s1(1024 * 1024);
    EXPECT_EQ(space::space_cast<space::KB>(s1).count(), 1024);
    EXPECT_EQ(space::space_cast<space::MB>(s1).count(), 1);
    EXPECT_EQ(space::space_cast<space::GB>(s1).count(), 1/1024);

    space::TB s2(1);
    EXPECT_EQ(space::space_cast<space::GB>(s2).count(), 1024);
}

TEST(SpaceClassTest, OperatorFunc) {
    space::B s1(1);
    EXPECT_EQ((+s1).count(), 1);
    EXPECT_EQ((-s1).count(), -1);
    EXPECT_EQ((++s1).count(), 2);
    EXPECT_EQ((s1++).count(), 2);
    EXPECT_EQ((--s1).count(), 2);
    EXPECT_EQ((s1--).count(), 2);

    space::KB s2(1);
    EXPECT_EQ((s1 += s2).count(), 1025);
    EXPECT_EQ((s1 -= s2).count(), 1);
    EXPECT_EQ((s1 *= 30).count(), 30);
    EXPECT_EQ((s1 /= 2).count(), 15);
    EXPECT_EQ((s1 %= 2).count(), 1);
}