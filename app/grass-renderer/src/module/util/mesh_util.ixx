module;

#include <_pch.h>

#include "ituGL/geometry/Mesh.h"
#include "ituGL/geometry/VertexFormat.h"

export module app.util.mesh;

struct TerrainMeshVertex {
    TerrainMeshVertex() = default;

    TerrainMeshVertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 texCoord
    ): position(position), normal(normal), texCoord(texCoord) {
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

export void CreateTerrainMesh(Mesh &mesh, const unsigned int gridX, const unsigned int gridY) {
    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;

    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    std::vector<TerrainMeshVertex> vertices;
    std::vector<unsigned int> indices;

    // Grid scale to convert the entire grid to size 1x1
    const glm::vec2 scale(1.0f / static_cast<float>(gridX - 1), 1.0f / static_cast<float>(gridY - 1));

    // Number of columns and rows
    const unsigned int columnCount = gridX;
    const unsigned int rowCount = gridY;

    // Iterate over each VERTEX
    for (unsigned int j = 0; j < rowCount; ++j) {
        for (unsigned int i = 0; i < columnCount; ++i) {
            // Vertex data for this vertex only
            glm::vec3 position(static_cast<float>(i) * scale.x, 0.0f, static_cast<float>(j) * scale.y);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec2 texCoord(i, j);

            vertices.emplace_back(position, normal, texCoord);

            // Index data for quad formed by previous vertices and current
            if (i > 0 && j > 0) {
                unsigned int top_right = j * columnCount + i; // Current vertex
                unsigned int top_left = top_right - 1;
                unsigned int bottom_right = top_right - columnCount;
                unsigned int bottom_left = bottom_right - 1;

                //Triangle 1
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
                indices.push_back(top_left);

                //Triangle 2
                indices.push_back(bottom_right);
                indices.push_back(top_left);
                indices.push_back(top_right);
            }
        }
    }

    mesh.AddSubmesh<TerrainMeshVertex, unsigned int, VertexFormat::LayoutIterator>(
        Drawcall::Primitive::Triangles, vertices,
        indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()),true),
        vertexFormat.LayoutEnd());
}
