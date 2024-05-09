#include <itugl/core/IndexedBufferObject.h>

#include <cassert>

IndexedBufferObject::IndexedBufferObject() : Object(NullHandle)
{
    auto& handle = GetHandle();
    glGenBuffers(1, &handle);
}

IndexedBufferObject::~IndexedBufferObject()
{
    const auto& handle = GetHandle();
    glDeleteBuffers(1, &handle);
}

IndexedBufferObject::IndexedBufferObject(IndexedBufferObject&& bufferObject) noexcept : Object(std::move(bufferObject)) {}

IndexedBufferObject& IndexedBufferObject::operator = (IndexedBufferObject&& bufferObject) noexcept
{
    Object::operator=(std::move(bufferObject));
    return *this;
}

// Bind the buffer handle to the specific target
void IndexedBufferObject::Bind(const Target target) const
{
    const auto handle = GetHandle();
    glBindBuffer(target, handle);
}

// Bind the null handle to the specific target
void IndexedBufferObject::Unbind(const Target target)
{
    constexpr Handle handle = NullHandle;
    glBindBuffer(target, handle);
}

// Get buffer Target and allocate buffer data
// ReSharper disable once CppMemberFunctionMayBeConst
void IndexedBufferObject::AllocateData(const size_t size, const Usage usage) {
    assert(IsBound());
    const Target target = GetTarget();
    glBufferData(target, static_cast<GLsizeiptr>(size), nullptr, usage);
}

// Get buffer Target and allocate buffer data
// ReSharper disable once CppMemberFunctionMayBeConst
void IndexedBufferObject::AllocateData(const std::span<const std::byte> data, const Usage usage) {
    assert(IsBound());
    const auto target = GetTarget();
    glBufferData(
        target,
        static_cast<GLsizeiptr>(data.size_bytes()),
        data.data(),
        usage
    );
}

// Get buffer Target and set buffer subdata
// ReSharper disable once CppMemberFunctionMayBeConst
void IndexedBufferObject::UpdateData(const std::span<const std::byte> data, const size_t offset) {
    assert(IsBound());
    const auto target = GetTarget();
    glBufferSubData(
        target,
        static_cast<GLsizeiptr>(offset),
        static_cast<GLsizeiptr>(data.size_bytes()),
        data.data()
    );
}

void IndexedBufferObject::BindToIndex(const unsigned int index) {
    const auto target = GetTarget();
    const auto handle = GetHandle();

    glBindBufferBase(target, index, handle);
}

void IndexedBufferObject::BindRangeToIndex(const unsigned int index, const size_t offset, const size_t size) {
    const auto target = GetTarget();
    const auto handle = GetHandle();

    glBindBufferRange(target, index, handle, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
}
