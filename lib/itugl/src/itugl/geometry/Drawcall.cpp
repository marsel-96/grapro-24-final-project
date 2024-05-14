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

void Drawcall::Draw() const
{
    switch(m_command) {
        case DrawCommand::Standard:
            DrawStandard();
            break;
        case DrawCommand::Instanced:
            DrawInstanced(m_instances);
            break;
        case DrawCommand::Indirect:
            DrawIndirect();
            break;

    }
}

// Execute the drawcall
void Drawcall::DrawStandard() const
{
    assert(IsValid());
    assert(VertexArrayObject::IsAnyBound());

    const auto primitive = static_cast<GLenum>(m_primitive);
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
void Drawcall::DrawInstanced(const unsigned int instances) const
{
    assert(IsValid());
    assert(VertexArrayObject::IsAnyBound());

    const auto primitive = static_cast<GLenum>(m_primitive);
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

void Drawcall::DrawIndirect() const {
    assert(IsValid());
    assert(VertexArrayObject::IsAnyBound());

    const auto primitive = static_cast<GLenum>(m_primitive);
    if (m_eboType == Data::Type::None)
    {
        // If no EBO is present, use glDrawArrays
        glDrawArraysIndirect(primitive, nullptr);
    }
    else
    {
        // If there is an EBO, use glDrawElements
        assert(ElementBufferObject::IsSupportedType(m_eboType));
        glDrawElementsIndirect(primitive, static_cast<GLenum>(m_eboType), nullptr);
    }
}
