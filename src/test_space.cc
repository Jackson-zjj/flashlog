#include <gtest/gtest.h>
#include "space.h"

TEST(SpaceClassTest, ConstructFunc) {
    logger::B s1;
    EXPECT_EQ(s1.count(), 0);

    logger::MB s2(10);
    EXPECT_EQ(s2.count(), 10);

    logger::GB s3(20);
    EXPECT_EQ(s3.count(), 20);

    logger::TB s4(30);
    EXPECT_EQ(s4.count(), 30);
}

TEST(SpaceClassTest, CastFunc) {
    logger::B s1(1024 * 1024);
    EXPECT_EQ(logger::space_cast<logger::KB>(s1).count(), 1024);
    EXPECT_EQ(logger::space_cast<logger::MB>(s1).count(), 1);
    EXPECT_EQ(logger::space_cast<logger::GB>(s1).count(), 1/1024);

    logger::TB s2(1);
    EXPECT_EQ(logger::space_cast<logger::GB>(s2).count(), 1024);
}

TEST(SpaceClassTest, OperatorFunc) {
    logger::B s1(1);
    EXPECT_EQ((+s1).count(), 1);
    EXPECT_EQ((-s1).count(), -1);
    EXPECT_EQ((++s1).count(), 2);
    EXPECT_EQ((s1++).count(), 2);
    EXPECT_EQ((--s1).count(), 2);
    EXPECT_EQ((s1--).count(), 2);

    logger::KB s2(1);
    EXPECT_EQ((s1 += s2).count(), 1025);
    EXPECT_EQ((s1 -= s2).count(), 1);
    EXPECT_EQ((s1 *= 30).count(), 30);
    EXPECT_EQ((s1 /= 2).count(), 15);
    EXPECT_EQ((s1 %= 2).count(), 1);
}