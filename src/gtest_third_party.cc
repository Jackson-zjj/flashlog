#include "gtest/gtest.h"
#include "fmt/core.h"
#include "person.pb.h"

TEST(ThirdPartyTest, FmtFunc) {
    EXPECT_EQ(fmt::format("Hello, {}", "World!"), "Hello, World!");
    EXPECT_EQ(fmt::format("1 + 2 = {}", 3), "1 + 2 = 3");
    EXPECT_EQ(fmt::format("{} * {} = {}", 0.5, 2, 1), "0.5 * 2 = 1");
}

TEST(ThirdPartyTest, ProtoFunc) {
    Person person;
    person.set_id(1);
    person.set_name("Jackson");
    person.set_email("jackson@example.com");

    // 序列化到字符串
    std::string cache;
    EXPECT_TRUE(person.SerializeToString(&cache));

    // 反序列化
    Person new_person;
    EXPECT_TRUE(new_person.ParseFromString(cache));
    EXPECT_EQ(new_person.id(), 1);
    EXPECT_EQ(new_person.name(), "Jackson");
    EXPECT_EQ(new_person.email(), "jackson@example.com");
}