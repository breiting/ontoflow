#pragma once

#include <glm/glm.hpp>
#include <ontoflow/vis/BaseGeometry.hpp>
#include <vector>

namespace of::vis {

/**
 * \brief Represents a renderable 3D mesh composed of vertices and indexed triangles.
 *
 * This class inherits from `BaseGeometry` to manage vertex data and
 * additionally handles index buffer objects (EBO) for efficient rendering of
 * triangle meshes. It also provides functionality for recalculating normals.
 */
class Mesh : public BaseGeometry {
   public:
    /**
     * \brief Constructs an empty Mesh object.
     * Initializes OpenGL buffer IDs to zero.
     */
    Mesh();

    /**
     * \brief Destructor. Deletes the associated OpenGL VAO, VBO, and EBO.
     */
    ~Mesh() override;

    /**
     * \brief Adds an index to the mesh's index buffer.
     * Marks the mesh as dirty, requiring re-upload to GPU.
     * \param idx The vertex index to add.
     */
    void AddIndex(unsigned int idx);

    /**
     * \brief Adds a triangle to the mesh using three vertex indices.
     * Marks the mesh as dirty, requiring re-upload to GPU.
     * \param v1 Index of the first vertex.
     * \param v2 Index of the second vertex.
     * \param v3 Index of the third vertex.
     */
    void AddTriangle(unsigned int v1, unsigned int v2, unsigned int v3);

    /**
     * \brief Clears all triangles (indices) from the mesh.
     * Marks the mesh as dirty.
     */
    void ClearTriangles();

    /**
     * \brief Uploads the mesh's vertex and index data to the GPU.
     * This method creates/updates VAO, VBO, and EBOs if the mesh is dirty.
     */
    void Upload() override;

    /**
     * \brief Renders the mesh using indexed triangles.
     * Assumes the appropriate shader is bound and uniforms are set.
     */
    void Render() const override;

    /**
     * \brief Returns a constant reference to the mesh's index buffer.
     * \return A const vector of unsigned integers representing the indices.
     */
    const std::vector<unsigned int>& GetIndices() const;

    /**
     * \brief Recalculates the vertex normals for the mesh.
     * This method computes face normals and assigns them to vertices.
     * Marks the mesh as dirty.
     */
    void RecalculateNormals();

   private:
    /**
     * \brief Deletes the OpenGL buffer objects (VAO, VBO, EBO).
     */
    void deleteBuffers();

   private:
    unsigned int m_Vao, m_Vbo, m_Ebo; ///< OpenGL Vertex Array, Vertex Buffer, and Element Buffer Object IDs.
    std::vector<unsigned int> m_Indices; ///< Local storage for vertex indices.
};
}  // namespace of::vis
