#pragma once

#include <filesystem>

namespace logger {
namespace fs {

size_t GetFileSize(std::filesystem::path&);

}   // namespace fs
}   // namespace logger