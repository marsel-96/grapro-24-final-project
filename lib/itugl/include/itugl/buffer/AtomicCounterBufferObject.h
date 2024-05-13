#pragma once

#include <itugl/core/Data.h>
#include "itugl/core/IndexedBufferObject.h"

class AtomicCounterBufferObject final: public IndexedBufferObjectBase<IndexedBufferObject::AtomicCounterBuffer>
{
public:
    AtomicCounterBufferObject();

    // AllocateData template method that replaces the size with the element count
    template<typename T>
    void AllocateData(size_t elementCount, Usage usage = StreamDraw);

    // AllocateData template method for any type of data span. Type must be one of the supported types
    template<typename T>
    void AllocateData(std::span<const T> data, Usage usage = StreamDraw);

    template<typename T>
    void AllocateData(std::span<T> data, Usage usage = StreamDraw) {
        AllocateData(std::span<const T>(data), usage);
    }

    // UpdateData template method for any type of data span. Type must be one of the supported types
    template<typename T>
    void UpdateData(std::span<const T> data, size_t offsetBytes = 0);
};

// Call the base implementation with the buffer size, computed with elementCount and size of T
template<typename T>
void AtomicCounterBufferObject::AllocateData(const size_t elementCount, const Usage usage)
{
    IndexedBufferObject::AllocateData(elementCount * sizeof(T), usage);
}

// Call the base implementation with the span converted to bytes
template<typename T>
void AtomicCounterBufferObject::AllocateData(std::span<const T> data, Usage usage)
{
    IndexedBufferObject::AllocateData(Data::GetBytes(data), usage);
}

// Call the base implementation with the span converted to bytes
template<typename T>
void AtomicCounterBufferObject::UpdateData(std::span<const T> data, const size_t offsetBytes)
{
    IndexedBufferObject::UpdateData(Data::GetBytes(data), offsetBytes);
}
