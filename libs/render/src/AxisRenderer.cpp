#include <ontoflow/vis/AxisRenderer.hpp>
#include <ontoflow/vis/Shader.hpp>
#include <vector>

#include "assets/unlit_frag.h"
#include "assets/unlit_vert.h"

namespace of::vis {

/**
 * \brief Destructor. Calls Shutdown() to release OpenGL resources.
 */
AxisRenderer::~AxisRenderer() {
    Shutdown();
}

/**
 * \brief Initializes the AxisRenderer.
 * Compiles shaders and builds the internal vertex buffers.
 * \return True if initialization was successful, false otherwise.
 */
bool AxisRenderer::Init() {
    m_Shader = std::make_unique<Shader>(unlit_vert_glsl, unlit_frag_glsl);
    // TODO: Error check for shader creation
    if (!m_Shader) {
        // Log an error if shader creation failed.
        return false;
    }

    BuildBuffers();
    // TODO: Error check for buffer creation
    if (m_Vao == 0) {
        return false;
    }
    return true;
}

/**
 * \brief Shuts down the AxisRenderer and releases its OpenGL resources.
 */
void AxisRenderer::Shutdown() {
    if (m_Vbo) {
        glDeleteBuffers(1, &m_Vbo);
        m_Vbo = 0;
    }
    if (m_Vao) {
        glDeleteVertexArrays(1, &m_Vao);
        m_Vao = 0;
    }
    // m_Shader (unique_ptr) will be destroyed automatically
}

/**
 * \brief Builds the vertex and color data for the axis lines and uploads to GPU.
 */
void AxisRenderer::BuildBuffers() {
    struct V {
        float x, y, z, r, g, b;
    };
    std::vector<V> v;

    // Helper lambda to push line segments for axis
    auto PushLine = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        v.push_back({a.x, a.y, a.z, c.r, c.g, c.b});
        v.push_back({b.x, b.y, b.z, c.r, c.g, c.b});
    };

    const float len = 10.0f;    // Length of the axis lines
    const float head = 2.0f;    // Length of the arrow head segments
    const float width = 1.0f;   // Width offset for arrow heads

    // Colors for X, Y, Z axes
    glm::vec3 colX(1, 0.2, 0.2); // Reddish
    glm::vec3 colY(0.2, 0.9, 0.2); // Greenish
    glm::vec3 colZ(0.2, 0.5, 1.0); // Blueish

    // X-axis (red) with arrow head
    PushLine({0, 0, 0}, {len, 0, 0}, colX);
    PushLine({len, 0, 0}, {len - head, width, 0}, colX);
    PushLine({len, 0, 0}, {len - head, -width, 0}, colX);

    // Y-axis (green) with arrow head
    PushLine({0, 0, 0}, {0, len, 0}, colY);
    PushLine({0, len, 0}, {width, len - head, 0}, colY);
    PushLine({0, len, 0}, {-width, len - head, 0}, colY);

    // Z-axis (blue) with arrow head
    PushLine({0, 0, 0}, {0, 0, len}, colZ);
    PushLine({0, 0, len}, {width, 0, len - head}, colZ);
    PushLine({0, 0, len}, {-width, 0, len - head}, colZ);

    m_VertexCount = static_cast<int>(v.size());

    glGenVertexArrays(1, &m_Vao);
    glGenBuffers(1, &m_Vbo);
    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(V), v.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3 * sizeof(float))); // Offset for color data

    glBindVertexArray(0);
}

/**
 * \brief Renders the axis gizmo.
 * \param model The model matrix for positioning the axis (usually identity for world origin).
 * \param view The camera's view matrix.
 * \param proj The camera's projection matrix.
 */
void AxisRenderer::Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj) const {
    if (!m_IsVisible || !m_Vao) // Render only if visible and initialized
        return;

    m_Shader->Bind();
    glm::mat4 mvp = proj * view * model; // Combine matrices
    m_Shader->SetMat4("uMVP", mvp);

    glBindVertexArray(m_Vao);
    glLineWidth(2.0f); // Set line width for axis
    glDrawArrays(GL_LINES, 0, m_VertexCount);
}

}  // namespace of::vis
