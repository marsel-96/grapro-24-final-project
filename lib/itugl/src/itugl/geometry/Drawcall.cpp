#include <itugl/geometry/Drawcall.h>

#include <itugl/geometry/VertexArrayObject.h>
#include <itugl/geometry/ElementBufferObject.h>
#include <cassert>

Drawcall::Drawcall()
    : m_primitive(Primitive::Invalid), m_first(0), m_count(0), m_eboType(Data::Type::None)
{
}

Drawcall::Drawcall(Primitive primitive, GLsizei count, GLint first)
    : Drawcall(primitive, count, Data::Type::None, first)
{
}

Drawcall::Drawcall(Primitive primitive, GLsizei count, Data::Type eboType, GLint first)
    : m_primitive(primitive), m_first(first), m_count(count), m_eboType(eboType)
{
    assert(primitive != Primitive::Invalid);
    assert(first >= 0);
    assert(count > 0);
}

// Execute the drawcall
void Drawcall::Draw() const
{
    assert(IsValid());
    assert(VertexArrayObject::IsAnyBound());

    GLenum primitive = static_cast<GLenum>(m_primitive);
    if (m_eboType == Data::Type::None)
    {
        // If no EBO is present, use glDrawArrays
        glDrawArrays(primitive, m_first, m_count);
    }
    else
    {
        // If there is an EBO, use glDrawElements
        assert(ElementBufferObject::IsSupportedType(m_eboType));
        const char* basePointer = nullptr; // Actual element pointer is in VAO
        glDrawElements(primitive, m_count, static_cast<GLenum>(m_eboType), basePointer + m_first);
    }
}

// Execute the drawcall
void Drawcall::Draw(const unsigned int instances) const
{
    assert(IsValid());
    assert(VertexArrayObject::IsAnyBound());

    GLenum primitive = static_cast<GLenum>(m_primitive);
    if (m_eboType == Data::Type::None)
    {
        // If no EBO is present, use glDrawArrays
        glDrawArraysInstanced(primitive, m_first, m_count, static_cast<GLsizei>(instances));
    }
    else
    {
        // If there is an EBO, use glDrawElements
        assert(ElementBufferObject::IsSupportedType(m_eboType));
        const char* basePointer = nullptr; // Actual element pointer is in VAO
        glDrawElementsInstanced(primitive, m_count, static_cast<GLenum>(m_eboType), basePointer + m_first, static_cast<GLsizei>(instances));
    }
}
