#include <glad.h>

#include <ontoflow/render/Mesh.hpp>

namespace of::render {

Mesh::Mesh() : m_Vao(0), m_Vbo(0), m_Ebo(0) {
}

Mesh::~Mesh() {
    deleteBuffers();
}

void Mesh::deleteBuffers() {
    if (m_Ebo)
        glDeleteBuffers(1, &m_Ebo);
    if (m_Vbo)
        glDeleteBuffers(1, &m_Vbo);
    if (m_Vao)
        glDeleteVertexArrays(1, &m_Vao);
}

void Mesh::AddIndex(unsigned int idx) {
    m_Indices.push_back(idx);
    m_Dirty = true;
}

void Mesh::AddTriangle(unsigned int v1, unsigned int v2, unsigned int v3) {
    m_Indices.push_back(v1);
    m_Indices.push_back(v2);
    m_Indices.push_back(v3);
    m_Dirty = true;
}

void Mesh::ClearTriangles() {
    m_Indices.clear();
    m_Dirty = true;
}

void Mesh::Upload() {
    if (!m_Dirty)
        return;

    if (m_Vao == 0) {
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(1, &m_Vbo);
        glGenBuffers(1, &m_Ebo);
    }

    glBindVertexArray(m_Vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(domain::Vertex), m_Vertices.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), m_Indices.data(), GL_STATIC_DRAW);

    // Attributes
    // 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(domain::Vertex), (void*)domain::Vertex::PositionOffset());
    // 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(domain::Vertex), (void*)domain::Vertex::NormalOffset());
    // 2: Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(domain::Vertex), (void*)domain::Vertex::ColorOffset());
    // 3: UV
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(domain::Vertex), (void*)domain::Vertex::TexCoordOffset());

    glBindVertexArray(0);
    m_Dirty = false;
}

void Mesh::Render() const {
    if (m_Vao == 0)
        return;

    glBindVertexArray(m_Vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

const std::vector<unsigned int>& Mesh::GetIndices() const {
    return m_Indices;
}

void Mesh::RecalculateNormals() {
    // Reset normals
    for (auto& v : m_Vertices) {
        v.SetNormal(glm::vec3(0.0f));
    }

    // Accumulate face normals
    for (size_t i = 0; i + 2 < m_Indices.size(); i += 3) {
        unsigned int i1 = m_Indices[i];
        unsigned int i2 = m_Indices[i + 1];
        unsigned int i3 = m_Indices[i + 2];

        glm::vec3 v1 = m_Vertices[i1].GetPosition();
        glm::vec3 v2 = m_Vertices[i2].GetPosition();
        glm::vec3 v3 = m_Vertices[i3].GetPosition();

        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // Check for NaN or zero length (degenerate triangle)
        if (glm::length(normal) > 0.0f) {
            m_Vertices[i1].SetNormal(m_Vertices[i1].GetNormal() + normal);
            m_Vertices[i2].SetNormal(m_Vertices[i2].GetNormal() + normal);
            m_Vertices[i3].SetNormal(m_Vertices[i3].GetNormal() + normal);
        }
    }

    // Normalize result
    for (auto& v : m_Vertices) {
        glm::vec3 n = v.GetNormal();
        if (glm::length(n) > 0.0f)
            v.SetNormal(glm::normalize(n));
    }
    m_Dirty = true;
}

}  // namespace of::render
