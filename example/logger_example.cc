
#include <iostream>
#include <chrono>
#include <thread>

#include "sinks/console_sink.h"
#include "sinks/simple_sink.h"
#include "sinks/mmap_sink.h"
#include "sinks/protobuf_sink.h"
#include "sinks/compress_sink.h"
#include "sinks/crypto_sink.h"
#include "sinks/async_sink.h"
#include "sinks/effective_sink.h"
#include "handler/log_variadic_handle.h"
#include "logger.h"

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
    LOGGER_INIT(handler);
    LOG_INFO("this is macro function");

    logger::detail::Conf conf;
    conf.file_dir = "/Users/jackson/MyProject/flashlog/test";
    conf.file_name = "loggerdemo";
    conf.server_pub_key =
        "04827405069030E26A211C973C8710E6FBE79B5CAA364AC111FB171311902277537F8852EADD17EB339EB7CD0BA2490A58CDED2C702DFC1EFC7EDB544B869F039C";
        // private key FAA5BBE9017C96BF641D19D0144661885E831B5DDF52539EF1AB4790C05E665E

    {
        std::cout << "---- file sink ----" << std::endl;
        // std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::SimpleSink>(conf);
        // std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::MMapSink>(conf);
        // std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::ProtobufSink>(conf);
        // std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::CompressSink>(conf);
        // std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::CryptoSink>(conf);
        // std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::AsyncSink>(conf);
        std::shared_ptr<logger::LogSink> file_sink = std::make_shared<logger::EffectiveSink>(conf);
        logger::LogHandle handle({file_sink});

        std::string str = GenerateRandomString(2000);
        auto begin = std::chrono::system_clock::now();
        for (int i = 0; i < 500000; ++i) {
            if (i % 100000 == 0) {
                std::cout << "i " << i << std::endl;
            }
            handle.Log(logger::LogLevel::kInfo, logger::LogSourceLoc(), str);
        }
        handle.Flush();
        auto end = std::chrono::system_clock::now();
        std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        std::cout << "logger time count: " << diff.count() << "ms" << std::endl;
    }

    return 0;
}