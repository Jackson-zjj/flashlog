#include <iostream>
#include <chrono>
#include <thread>

#include "sinks/console_sink.h"
#include "sinks/effective_sink.h"
#include "handler/log_variadic_handle.h"

std::string GenerateRandomString(int length) {
  std::string str;
  str.reserve(length);
  for (int i = 0; i < length; ++i) {
    str.push_back('a' + rand() % 26);
  }
  return str;
}

int main() {
    std::cout << "========= logger_example =========" << std::endl;
    std::cout << "---- console sink ----" << std::endl;
    std::shared_ptr<logger::LogSink> sink = std::make_shared<logger::ConsoleSink>();
    auto handler = std::make_shared<logger::VariadicLogHandle>(sink);
    handler->Log(logger::LogLevel::kInfo, "Hello, World!");


    std::cout << "---- effective sink ----" << std::endl;
    logger::EffectiveSink::Conf conf;
    conf.file_dir = "/Users/jackson/MyProject/flashlog/test";
    conf.file_name = "loggerdemo";
    conf.server_pub_key =
        "04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1E"
        "FC7EDB544B869F039C";

    {
        std::shared_ptr<logger::LogSink> effective_sink = std::make_shared<logger::EffectiveSink>(conf);
        logger::LogHandle handle({effective_sink});
        std::string str = GenerateRandomString(2000);
        auto begin = std::chrono::system_clock::now();

        auto task = [&]() {
            for (int i = 0; i < 500000; ++i) {
                // if (i % 100000 == 0) {
                //     std::cout << "i " << i << std::endl;
                // }
                handle.Log(logger::LogLevel::kInfo, logger::LogSourceLoc(), str);
            }
            effective_sink->Flush();
        };
        std::vector<std::thread*> threads;
        for (int i = 0; i < 3; ++i) {
            std::thread* th = new std::thread((task));
            threads.emplace_back(th);
        }
        for (auto& th : threads) {
            if (th->joinable()) {
                th->join();
            }
        }

        auto end = std::chrono::system_clock::now();
        std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "logger time count: " << diff.count() << "ms" << std::endl;
    }

    return 0;
}