#include "mmap/mmap_aux.h"

#include <string.h>
#include <iostream>

#include "utils/file_util.h"
#include "utils/sys_util.h"

namespace logger {

constexpr size_t kDafaultCapacity = 512 * 1024;  // 512KB

MMapAux::MMapAux(fpath file_path) : capacity_(0), size_(0), handle_(nullptr), file_path_(file_path) {
    size_t size = fs::GetFileSize(file_path);
    size_t new_size = std::max(size, kDafaultCapacity);
    Reserve_(new_size);
    size_ = size;
}

void MMapAux::Push(const void* data, size_t size) {
    size_t new_size = Size() + size;
    EnsureCapacity_(new_size);
    memcpy(Data() + Size(), data, size);
    size_ = new_size;
}

void MMapAux::Resize(size_t new_size) {
    EnsureCapacity_(new_size);
    size_ = new_size;
}

void MMapAux::Clear() {
    size_ = 0;
}

size_t MMapAux::Size() const {
    return size_;
}

size_t MMapAux::Capacity_() const {
    return capacity_;
}

bool MMapAux::Empty() const {
    return Size() == 0;
}

uint8_t* MMapAux::Data() const {
    return static_cast<uint8_t*>(handle_);
}

double MMapAux::GetRatio() const {
    return static_cast<double>(Size() / Capacity_());
}

static size_t GetValidCapacity(size_t size) {
    size_t page_size = GetPageSize();
    return (size + page_size - 1) / page_size * page_size;
}

void MMapAux::Reserve_(size_t new_capacity) {
    size_t real_capacity = GetValidCapacity(new_capacity);
    if (real_capacity <= capacity_) {
        return;
    }
    Unmap_();
    TryMap_(real_capacity); // todo check mmap result
    capacity_ = real_capacity;
}

void MMapAux::EnsureCapacity_(size_t new_capacity) {
    size_t real_capacity = GetValidCapacity(new_capacity);
    if (real_capacity <= capacity_) {
        return;
    }
    Reserve_(real_capacity);
}

}   //namespace logger