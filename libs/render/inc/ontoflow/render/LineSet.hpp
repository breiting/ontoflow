#pragma once

#include <glm/glm.hpp>
#include <ontoflow/render/BaseGeometry.hpp>

namespace of::render {

/**
 * \brief Represents a renderable set of lines.
 *
 * This class inherits from `BaseGeometry` to manage vertex data and
 * specifically handles OpenGL Vertex Array Objects (VAO) and Vertex Buffer Objects (VBO)
 * for rendering lines.
 */
class LineSet : public BaseGeometry {
   public:
    /**
     * \brief Constructs an empty LineSet object.
     * Initializes OpenGL buffer IDs to zero.
     */
    LineSet();

    /**
     * \brief Destructor. Deletes the associated OpenGL VAO and VBO.
     */
    ~LineSet() override;

    /**
     * \brief Uploads the line set's vertex data to the GPU.
     * This method creates/updates VAO and VBOs if the line set is dirty.
     */
    void Upload() override;

    /**
     * \brief Renders the line set.
     * Assumes the appropriate shader is bound and uniforms are set.
     */
    void Render() const override;

   private:
    /**
     * \brief Deletes the OpenGL buffer objects (VAO, VBO).
     */
    void deleteBuffers();

   private:
    unsigned int m_Vao, m_Vbo; ///< OpenGL Vertex Array and Vertex Buffer Object IDs.
};
}  // namespace of::render
