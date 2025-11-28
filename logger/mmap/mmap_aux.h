#pragma once

#include <cstddef>
#include <filesystem>

namespace logger {

class MMapAux {
public:
    using fpath = std::filesystem::path;

    explicit MMapAux(fpath file_path);
    ~MMapAux() = default;

    MMapAux(const MMapAux&) = delete;
    MMapAux& operator=(const MMapAux&) = delete;
    MMapAux(MMapAux&&) = delete;
    MMapAux& operator=(MMapAux&&) = delete;

    void Push(const void* data, size_t size);
    void Resize(size_t new_size);
    void Clear();
    
    size_t Size() const;
    bool Empty() const;
    uint8_t* Data() const;
    double GetRatio() const;

private:
    struct MMapHeader {
        static constexpr uint32_t kMagic = 0xdeadbeef;
        uint32_t magic;
        uint32_t size;
    };

private:
    void Init_();
    void Reserve_(size_t size);
    void EnsureCapacity_(size_t new_size);
    bool TryMap_(size_t capacity);
    void Unmap_();

    MMapHeader* Header_() const;
    size_t Capacity_() const;
    bool isValid_() const;

private:
    size_t capacity_;   // 文件大小
    size_t size_;       // 内容大小（废弃）
    void* handle_;      // mmap映射
    fpath file_path_; 
};

}   //namespace logger