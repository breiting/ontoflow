#pragma once
#include <glm/glm.hpp>
#include <ontoflow/domain/Vertex.hpp>
#include <vector>

namespace of::vis {

/**
 * \brief Abstract base class for all renderable geometry in the visualization system.
 *
 * This class provides common functionality for managing CPU-side vertex data
 * and declares pure virtual functions for uploading data to the GPU and rendering.
 * It also includes a dirty flag to optimize GPU data uploads.
 */
class BaseGeometry {
   public:
    /**
     * \brief Pure virtual function to upload vertex data to the GPU.
     * Implementations should handle buffer creation and data transfer.
     */
    virtual void Upload() = 0;

    /**
     * \brief Pure virtual function to render the geometry.
     * Implementations should handle binding appropriate VAOs and issuing draw calls.
     */
    virtual void Render() const = 0;

    /**
     * \brief Constructs a BaseGeometry object.
     * Initializes the dirty flag to true.
     */
    BaseGeometry();

    /**
     * \brief Virtual destructor to ensure proper cleanup of derived geometry classes.
     */
    virtual ~BaseGeometry() = default;

    /**
     * \brief Adds a single vertex to the geometry's vertex buffer.
     * Marks the geometry as dirty.
     * \param v The vertex to add.
     */
    void AddVertex(const domain::Vertex& v);

    /**
     * \brief Sets the entire vertex buffer.
     * Overwrites existing vertices and marks the geometry as dirty.
     * \param vertices A vector of domain::Vertex objects.
     */
    void SetVertices(const std::vector<domain::Vertex>& vertices);

    /**
     * \brief Returns the number of vertices in the geometry.
     * \return The count of vertices.
     */
    size_t VertexCount() const;

    /**
     * \brief Returns a constant reference to the internal vertex buffer.
     * \return A const reference to the vector of domain::Vertex objects.
     */
    const std::vector<domain::Vertex>& GetVertices() const;

   protected:
    std::vector<domain::Vertex> m_Vertices; ///< CPU-side storage for vertex data.
    bool m_Dirty; ///< Flag indicating if the vertex data has changed and needs re-upload to GPU.
};
}  // namespace of::vis
