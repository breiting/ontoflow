// clang-format off
#include <glad.h>
// clang-format on
#include <glm/gtc/constants.hpp>
#include <ontoflow/render/LineSet.hpp>

using namespace of::domain;

namespace of::render {

/**
 * \brief Constructs an empty LineSet object.
 * Initializes OpenGL buffer IDs to zero.
 */
LineSet::LineSet() : BaseGeometry(), m_Vao(0), m_Vbo(0) {
}

/**
 * \brief Destructor. Deletes the associated OpenGL VAO and VBO.
 */
LineSet::~LineSet() {
    deleteBuffers();
}

/**
 * \brief Uploads the line set's vertex data to the GPU.
 * This method creates/updates VAO and VBOs if the line set is dirty.
 */
void LineSet::Upload() {
    // ignore if the data has not changed
    if (!m_Dirty)
        return;

    // Clean up old objects if necessary
    deleteBuffers();

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    glGenBuffers(1, &m_Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::PositionOffset());
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::ColorOffset());
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    m_Dirty = false;
}

/**
 * \brief Renders the line set.
 * Assumes the appropriate shader is bound and uniforms are set.
 * Renders using GL_LINE_LOOP, connecting the last vertex to the first.
 */
void LineSet::Render() const {
    glBindVertexArray(m_Vao);
    glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(m_Vertices.size()));
}

/**
 * \brief Deletes the OpenGL buffer objects (VAO, VBO).
 */
void LineSet::deleteBuffers() {
    if (m_Vao) {
        glDeleteVertexArrays(1, &m_Vao);
        m_Vao = 0; // Reset to 0 after deletion
    }
    if (m_Vbo) {
        glDeleteBuffers(1, &m_Vbo);
        m_Vbo = 0; // Reset to 0 after deletion
    }
}
}  // namespace of::render
