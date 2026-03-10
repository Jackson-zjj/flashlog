#include "memory_buffer.h"

#include "utils/file_util.h"


namespace logger {

void MemoryBuffer::Push(const void* data, size_t size) {
    size_t new_size = Size() + size;
    EnsureCapacity_(new_size);
    memcpy(Data() + Size(), data, size);
    data_.resize(new_size);
}

void MemoryBuffer::Reserve(size_t capacity) {
    data_.reserve(capacity);
}

void MemoryBuffer::Clear() {
    data_.resize(0);
}

size_t MemoryBuffer::Size() const{
    return data_.size();
}

size_t MemoryBuffer::Capacity() const{
    return data_.capacity();
}

bool MemoryBuffer::Empty() const{
    return data_.empty();
}

uint8_t* MemoryBuffer::Data() const{
    return const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data_.data()));
}

double MemoryBuffer::GetRatio() const{
    return static_cast<double>(Size()) / static_cast<double>(Capacity());
}

void MemoryBuffer::EnsureCapacity_(size_t new_size){
    if (new_size <= Capacity()) {
        return;
    }
    Reserve_(new_size);
}

void MemoryBuffer::Reserve_(size_t size) {
    while(Capacity() <= size) {
        data_.reserve(Capacity() * 1.5);
    }
}

}// namespace logger