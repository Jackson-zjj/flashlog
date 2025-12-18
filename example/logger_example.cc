#include <iostream>
#include <chrono>

#include "sinks/console_sink.h"
#include "handler/log_variadic_handle.h"

int main() {
    std::cout << "========= logger_example =========" << std::endl;
    std::shared_ptr<logger::LogSink> sink = std::make_shared<logger::ConsoleSink>();
    auto handler = std::make_shared<logger::VariadicLogHandle>(sink);
    handler->Log(logger::LogLevel::kInfo, "Hello, World!");
    return 0;
}