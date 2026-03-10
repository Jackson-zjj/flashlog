#pragma once

#include <string>

namespace logger {

/// @brief 内存缓冲区
class MemoryBuffer {
public:
    MemoryBuffer() = default;
    ~MemoryBuffer() = default;

    MemoryBuffer(const MemoryBuffer&) = delete;
    MemoryBuffer& operator=(const MemoryBuffer&) = delete;
    MemoryBuffer(MemoryBuffer&&) = delete;
    MemoryBuffer& operator=(MemoryBuffer&&) = delete;

    void Push(const void* data, size_t size);

    void Reserve(size_t capacity);

    void Clear();
    
    size_t Size() const;

    size_t Capacity() const;

    bool Empty() const;

    uint8_t* Data() const;
    
    double GetRatio() const;

private:
    void Reserve_(size_t size);
    void EnsureCapacity_(size_t new_size);

    std::string data_;
};

}   //namespace logger