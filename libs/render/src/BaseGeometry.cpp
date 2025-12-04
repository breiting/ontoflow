// clang-format off
#include <glad.h>
// clang-format on
#include <ontoflow/vis/BaseGeometry.hpp>

using namespace of::domain;

namespace of::vis {

/**
 * \brief Constructs a BaseGeometry object.
 * Initializes the dirty flag to true.
 */
BaseGeometry::BaseGeometry() : m_Dirty(true) {
}

/**
 * \brief Adds a single vertex to the geometry's vertex buffer.
 * Marks the geometry as dirty.
 * \param v The vertex to add.
 */
void BaseGeometry::AddVertex(const Vertex& v) {
    m_Vertices.push_back(v);
    m_Dirty = true;
}

/**
 * \brief Sets the entire vertex buffer.
 * Overwrites existing vertices and marks the geometry as dirty.
 * \param vertices A vector of domain::Vertex objects.
 */
void BaseGeometry::SetVertices(const std::vector<domain::Vertex>& vertices) {
    m_Vertices = vertices;
    m_Dirty = true; // Data has changed, so mark as dirty
}

/**
 * \brief Returns the number of vertices in the geometry.
 * \return The count of vertices.
 */
size_t BaseGeometry::VertexCount() const {
    return m_Vertices.size();
}

/**
 * \brief Returns a constant reference to the internal vertex buffer.
 * \return A const reference to the vector of domain::Vertex objects.
 */
const std::vector<Vertex>& BaseGeometry::GetVertices() const {
    return m_Vertices;
}
}  // namespace of::vis
