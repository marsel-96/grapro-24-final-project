#include "itugl/compute/Compute.h"

ComputeCall::ComputeCall(const std::shared_ptr<ShaderProgram>& shaderProgram, const glm::uvec3 workGroup):
    ShaderUniformCollection(shaderProgram),
    m_workGroup(workGroup) {}

void ComputeCall::AddBufferBinding(const std::shared_ptr<IndexedBufferObject> &bufferObject, const unsigned int bindingIndex) {
    m_indexedBufferObjects.emplace_back(bufferObject, bindingIndex);
}

void ComputeCall::BindBuffers() const {
    for (const auto&[buffer, index]: m_indexedBufferObjects) {
        buffer->BindToIndex(index);
    }
}

void ComputeCall::Compute() const {
    BindBuffers();

    m_shaderProgram->Use();

    SetUniforms();

    glDispatchCompute(m_workGroup.x, m_workGroup.y, m_workGroup.z);
}
