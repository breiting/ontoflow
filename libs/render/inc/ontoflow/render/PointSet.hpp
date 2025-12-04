#pragma once
#include <glm/glm.hpp>
#include <ontoflow/render/BaseGeometry.hpp>

namespace of::render {

/**
 * \brief Represents a renderable set of points.
 *
 * This class inherits from `BaseGeometry` to manage vertex data and
 * specifically handles OpenGL Vertex Array Objects (VAO) and Vertex Buffer Objects (VBO)
 * for rendering points, typically using instancing for point sprites.
 */
class PointSet : public BaseGeometry {
   public:
    /**
     * \brief Constructs an empty PointSet object.
     * Initializes OpenGL buffer IDs to zero.
     */
    PointSet();

    /**
     * \brief Destructor. Deletes the associated OpenGL VAO and VBO.
     */
    ~PointSet() override;

    /**
     * \brief Uploads the point set's vertex data to the GPU.
     * This method creates/updates VAO and VBOs.
     */
    void Upload() override;

    /**
     * \brief Renders the point set.
     * Assumes the appropriate shader is bound and uniforms are set.
     * Uses instanced rendering.
     */
    void Render() const override;

   private:
    /**
     * \brief Deletes the OpenGL buffer objects (VAO, VBO).
     */
    void deleteBuffers();

   private:
    unsigned int m_Vao = 0; ///< OpenGL Vertex Array Object ID.
    unsigned int m_Vbo = 0; ///< OpenGL Vertex Buffer Object ID.
};
}  // namespace of::render
