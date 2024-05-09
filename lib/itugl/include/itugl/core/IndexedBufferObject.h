#pragma once

#include <itugl/core/Object.h>
#include <span>

class IndexedBufferObject : public Object
{
public:

    enum Target : GLenum
    {
        SharedStoragBuffer = GL_SHADER_STORAGE_BUFFER,
        UniformBuffer = GL_UNIFORM_BUFFER,
        TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
        AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER
    };

public:

    IndexedBufferObject();
    ~IndexedBufferObject() override;

    // (C++) 8
    // Move semantics
    IndexedBufferObject(IndexedBufferObject&& bufferObject) noexcept;
    IndexedBufferObject& operator = (IndexedBufferObject&& bufferObject) noexcept;

    // (C++) 3
    // Use the same Bind method from the base class
    // It was not inherited implicitly because we defined another method named Bind (below)
    using Object::Bind;

    // Each derived class will return its Target
    [[nodiscard]] virtual Target GetTarget() const = 0;

    // Allocate the buffer, specifying the size and the usage. We can also provide initial contents
    void AllocateData(size_t size, Usage usage);
    void AllocateData(std::span<const std::byte> data, Usage usage);

    // Modify the contents of the buffer, starting at offset
    void UpdateData(std::span<const std::byte> data, size_t offset = 0);

    void BindToIndex(unsigned int index);

    void BindRangeToIndex(unsigned int index, size_t offset, size_t size);

protected:

    // Bind the specific target. Used by the Bind() method in derived classes
    void Bind(Target target) const;

    // Unbind the specific target. It is static because we don´t need any objects to do it
    static void Unbind(Target target);
};

// (C++) 5
// Templated BufferObject derived class based on the Target type
// Implements methods that are common to all targets
template<IndexedBufferObject::Target T>
class IndexedBufferObjectBase : public IndexedBufferObject
{
public:
    IndexedBufferObjectBase() = default;

    // Return the templated enum value T
    [[nodiscard]] Target GetTarget() const override { return T; }

    // When binding this object, we bind it to the corresponding Target
    void Bind() const override;

    // When unbinding this class, we unbind the corresponding Target
    static void Unbind();

#ifndef NDEBUG
    static bool IsAnyBound() { return s_boundHandle != NullHandle; }
#endif

protected:

#ifndef NDEBUG
    [[nodiscard]] bool IsBound() const override { return s_boundHandle == GetHandle(); }

    static Handle s_boundHandle;
#endif
};

#ifndef NDEBUG
template<IndexedBufferObject::Target T>
Object::Handle IndexedBufferObjectBase<T>::s_boundHandle = NullHandle;
#endif

template<IndexedBufferObject::Target T>
void IndexedBufferObjectBase<T>::Bind() const
{
    IndexedBufferObject::Bind(T);
#ifndef NDEBUG
    s_boundHandle = GetHandle();
#endif
}

template<IndexedBufferObject::Target T>
void IndexedBufferObjectBase<T>::Unbind()
{
    IndexedBufferObject::Unbind(T);
#ifndef NDEBUG
    s_boundHandle = NullHandle;
#endif
}