#include <string>

#include "gtest/gtest.h"
#include "mmap/mmap_aux.h"

const char* kFilePath = "test/log.txt";

TEST(MMapClassTest, DataFunc) {
    logger::MMapAux m1(kFilePath);

    m1.Clear();
    std::string str(reinterpret_cast<const char*>(m1.Data()), m1.Size());
    EXPECT_TRUE(m1.Empty());
    EXPECT_STREQ(str.c_str(), "");

    m1.Push("Hello, World", sizeof("Hello, World"));
    str.assign(reinterpret_cast<const char*>(m1.Data()), m1.Size());
    EXPECT_FALSE(m1.Empty());
    EXPECT_STREQ(str.c_str(), "Hello, World");

    m1.Clear();
}

TEST(MMapClassTest, SizeFunc) {
    logger::MMapAux m1(kFilePath);

    m1.Clear();
    EXPECT_EQ(m1.Size(), 0);

    m1.Push("Hello", sizeof("Hello"));
    EXPECT_EQ(m1.Size(), 6);

    m1.Resize(100);
    EXPECT_EQ(m1.Size(), 100);

    m1.Push("Hello", sizeof("Hello"));
    EXPECT_GT(m1.Size(), 100);
    EXPECT_LT(m1.GetRatio(), 1);

    m1.Clear();
}