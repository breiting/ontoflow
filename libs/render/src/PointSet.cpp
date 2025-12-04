// clang-format off
#include <glad.h>
// clang-format on
#include <glm/gtc/constants.hpp>
#include <ontoflow/vis/PointSet.hpp>
#include <ontoflow/vis/Shader.hpp>

using namespace of::domain;

namespace of::vis {

/**
 * \brief Constructs an empty PointSet object.
 * Initializes internal state.
 */
PointSet::PointSet() : BaseGeometry() {
}

/**
 * \brief Destructor. Deletes the associated OpenGL VAO and VBO.
 */
PointSet::~PointSet() {
    deleteBuffers();
}

/**
 * \brief Deletes the OpenGL buffer objects (VAO, VBO).
 */
void PointSet::deleteBuffers() {
    if (m_Vbo) {
        glDeleteBuffers(1, &m_Vbo);
        m_Vbo = 0;
    }
    if (m_Vao) {
        glDeleteVertexArrays(1, &m_Vao);
        m_Vao = 0;
    }
}

/**
 * \brief Uploads the point set's vertex data to the GPU.
 * This method creates/updates VAO and VBOs.
 */
void PointSet::Upload() {
    // ignore if the data has not changed
    if (!m_Dirty && m_Vao != 0) // Only upload if dirty or first time
        return;

    if (m_Vertices.empty()) {
        deleteBuffers(); // Clean up if no vertices
        m_Dirty = false;
        return;
    }

    // Create buffers if they don't exist
    if (m_Vao == 0) {
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(1, &m_Vbo);
    }

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_DYNAMIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::PositionOffset());
    // Instancing: This tells OpenGL to advance the vertex attribute to the next element
    // only once per instance, effectively making it per-instance data.
    // However, the current rendering setup uses glDrawArraysInstanced with a fixed quad.
    // If the shader is meant to sample from a point, this divisor might be 0, or needs explicit per-vertex color.
    // For now, assume position is per-vertex for instanced quad center.
    glVertexAttribDivisor(0, 1); 

    // Color (assuming color is also per-vertex)
    glEnableVertexAttribArray(1); // Assuming layout(location=1) for color in PointSet shader
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::ColorOffset());
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
    m_Dirty = false;
}

/**
 * \brief Renders the point set.
 * Assumes the appropriate shader is bound and uniforms are set.
 * Uses instanced rendering.
 */
void PointSet::Render() const {
    if (!m_Vao || m_Vertices.empty())
        return;

    glBindVertexArray(m_Vao);
    // Draw a single quad (4 vertices for a triangle strip) for each point in m_Vertices
    // The shader will position and size these quads.
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(m_Vertices.size()));
    glBindVertexArray(0);
}

}  // namespace of::vis
