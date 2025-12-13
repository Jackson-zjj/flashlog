#include "gtest/gtest.h"

#include <iostream>

#include "scheduler/thread_pool.h"

void print() {
    std::cout << "this is threadPool test" << std::endl;
}

int add(int a, int b) {
    return a + b;
}

TEST(ThreadPoolTest, CreateFunc) {
    logger::ThreadPool t(10);
    t.Start();
    t.AddTask(print);
    EXPECT_EQ(t.AddReturnTask(add, 2, 3).get(), 5);
    EXPECT_EQ(t.AddReturnTask(add, 10, 20).get(), 30);
}