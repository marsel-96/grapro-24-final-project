#pragma once

#include "itugl/core/IndexedBufferObject.h"
#include "ituGL/shader/ShaderUniformCollection.h"

class ComputeCall: public ShaderUniformCollection {

    glm::uvec3 m_workGroup;
    std::vector<std::pair<std::shared_ptr<IndexedBufferObject>, unsigned int>> m_indexedBufferObjects;

public:

    explicit ComputeCall(const std::shared_ptr<ShaderProgram> &shaderProgram, glm::uvec3 workGroup);

    void ChangeWorkGroupSize(const glm::uvec3 workGroup) { m_workGroup = workGroup;}
    [[nodiscard]] glm::uvec3 GetWorkGroupSize() const { return m_workGroup; }

    void AddBufferBinding(const std::shared_ptr<IndexedBufferObject> &bufferObject, unsigned int bindingIndex);

    void BindBuffers() const;

    void Compute() const;

};
