#pragma once
#include <glad.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <ontoflow/vis/Shader.hpp>

namespace of::vis {

/**
 * \brief Renders a 3D coordinate axis gizmo (X, Y, Z axes).
 *
 * This class handles the creation and rendering of a simple visual aid
 * to show the orientation of the coordinate system in a 3D scene.
 */
class AxisRenderer {
   public:
    /**
     * \brief Constructs an AxisRenderer.
     * Initializes the axis as visible by default.
     */
    AxisRenderer() = default;

    /**
     * \brief Destructor. Calls Shutdown() to release OpenGL resources.
     */
    ~AxisRenderer();

    /**
     * \brief Initializes the AxisRenderer.
     * Compiles shaders and builds the internal vertex buffers.
     * \return True if initialization was successful, false otherwise.
     */
    bool Init();

    /**
     * \brief Shuts down the AxisRenderer and releases its OpenGL resources.
     */
    void Shutdown();

    /**
     * \brief Sets the visibility of the axis gizmo.
     * \param v True to show the axis, false to hide.
     */
    void SetVisible(bool v) {
        m_IsVisible = v;
    }

    /**
     * \brief Checks if the axis gizmo is currently visible.
     * \return True if visible, false otherwise.
     */
    bool IsVisible() const {
        return m_IsVisible;
    }

    /**
     * \brief Renders the axis gizmo.
     * \param model The model matrix for positioning the axis (usually identity for world origin).
     * \param view The camera's view matrix.
     * \param proj The camera's projection matrix.
     */
    void Render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj) const;

   private:
    /**
     * \brief Builds the vertex and color data for the axis lines and uploads to GPU.
     */
    void BuildBuffers();

   private:
    unsigned int m_Vao = 0; ///< OpenGL Vertex Array Object ID.
    unsigned int m_Vbo = 0; ///< OpenGL Vertex Buffer Object ID.
    int m_VertexCount = 0; ///< Number of vertices to draw (for GL_LINES).

    bool m_IsVisible = true; ///< Flag to control visibility.

    std::unique_ptr<Shader> m_Shader; ///< Shader program used for rendering the axis.
};

}  // namespace of::vis
