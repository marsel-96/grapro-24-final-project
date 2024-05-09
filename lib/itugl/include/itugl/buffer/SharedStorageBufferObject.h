#pragma once

#include <itugl/core/IndexedBufferObject.h>
#include <itugl/core/Data.h>

class SharedStorageBufferObject final: public IndexedBufferObjectBase<IndexedBufferObject::SharedStoragBuffer>
{
public:
    SharedStorageBufferObject();

    // AllocateData template method that replaces the size with the element count
    template<typename T>
    void AllocateData(size_t elementCount, Usage usage = StaticDraw);

    // AllocateData template method for any type of data span. Type must be one of the supported types
    template<typename T>
    void AllocateData(std::span<const T> data, Usage usage = StaticDraw);

    template<typename T>
    void AllocateData(std::span<T> data, Usage usage = StaticDraw) {
        AllocateData(std::span<const T>(data), usage);
    }

    // UpdateData template method for any type of data span. Type must be one of the supported types
    template<typename T>
    void UpdateData(std::span<const T> data, size_t offsetBytes = 0);
};

// Call the base implementation with the buffer size, computed with elementCount and size of T
template<typename T>
void SharedStorageBufferObject::AllocateData(const size_t elementCount, const Usage usage)
{
    IndexedBufferObject::AllocateData(elementCount * sizeof(T), usage);
}

// Call the base implementation with the span converted to bytes
template<typename T>
void SharedStorageBufferObject::AllocateData(std::span<const T> data, Usage usage)
{
    IndexedBufferObject::AllocateData(Data::GetBytes(data), usage);
}

// Call the base implementation with the span converted to bytes
template<typename T>
void SharedStorageBufferObject::UpdateData(std::span<const T> data, const size_t offsetBytes)
{
    IndexedBufferObject::UpdateData(Data::GetBytes(data), offsetBytes);
}
