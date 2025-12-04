#include <algorithm>
#include <ontoflow/render/Camera2D.hpp>

namespace of::render {

/**
 * \brief Constructs a default 2D camera.
 * Initializes position to origin, zoom to 5.0, and aspect ratio to 1.0.
 */
Camera2D::Camera2D() : m_Position(0.0f), m_FirstMouse(true), m_Zoom(5.0f), m_AspectRatio(1.0f) {
}

/**
 * \brief Called when a mouse interaction starts.
 * Resets internal mouse tracking for delta calculation.
 */
void Camera2D::OnMouseStart() {
    m_FirstMouse = true;
}

/**
 * \brief Handles mouse panning input.
 * Adjusts the camera's position based on mouse movement.
 * \param xpos Current X-coordinate of the mouse.
 * \param ypos Current Y-coordinate of the mouse.
 */
void Camera2D::OnMousePan(double xpos, double ypos) {
    if (m_FirstMouse) {
        m_LastMouse = {xpos, ypos};
        m_FirstMouse = false;
        return;
    }

    glm::vec2 newMouse(xpos, ypos);
    glm::vec2 delta = newMouse - m_LastMouse;
    m_LastMouse = newMouse;

    float vpHeight = (m_VP.y > 0) ? m_VP.y : 800.0f; // Fallback for vpHeight
    float unitsPerPixel = (2.0f * m_Zoom) / vpHeight;
    m_Position.x -= delta.x * unitsPerPixel;
    m_Position.y += delta.y * unitsPerPixel;
}

/**
 * \brief Handles mouse scroll input for zooming.
 * Adjusts the camera's zoom level.
 * \param yoffset The scroll offset (usually vertical).
 */
void Camera2D::OnMouseScroll(double yoffset) {
    m_Zoom -= yoffset * 0.5f;
    m_Zoom = std::max(0.5f, m_Zoom);  // Minimum zoom level
}

/**
 * \brief Returns the view matrix for the 2D camera.
 * \return A glm::mat4 representing the camera's view transformation.
 */
glm::mat4 Camera2D::GetViewMatrix() const {
    return glm::lookAt(glm::vec3(m_Position.x, m_Position.y, 10.0f),  // Position (looking from 10 units above XY plane)
                       glm::vec3(m_Position.x, m_Position.y, 0.0f),   // Target (looking at the origin of the view plane)
                       glm::vec3(0, 1, 0)                             // Up (Y-axis points up in screen space)
    );
}

/**
 * \brief Returns the projection matrix for the 2D camera.
 * \return A glm::mat4 representing the orthographic projection.
 */
glm::mat4 Camera2D::GetProjectionMatrix() const {
    float half = m_Zoom;
    return glm::ortho(-half * m_AspectRatio, +half * m_AspectRatio, -half, +half, -100.0f, +100.0f);
}

/**
 * \brief Sets the 2D position of the camera.
 * \param pos The new 2D position.
 */
void Camera2D::SetPosition(const glm::vec2& pos) {
    m_Position = pos;
}

/**
 * \brief Converts screen coordinates to 3D world coordinates (on the XY-plane).
 * This performs an unprojection, effectively "picking" a point on the Z=0 plane.
 * \param screenX X-coordinate on screen.
 * \param screenY Y-coordinate on screen.
 * \return A glm::vec3 representing the world coordinates.
 */
glm::vec3 Camera2D::ScreenToWorld(double x, double y) const {
    //  Coordinates in NDC (-1 .. 1)
    float ndcX = (2.0f * x / m_VP.x) - 1.0f;
    float ndcY = 1.0f - (2.0f * y / m_VP.y); // Y-axis is inverted for screen coordinates (top-left is 0,0)

    glm::vec4 ndc(ndcX, ndcY, 0.0f, 1.0f); // Z=0 in NDC for the plane of interest

    glm::mat4 inv = glm::inverse(GetProjectionMatrix() * GetViewMatrix());
    glm::vec4 world = inv * ndc;

    glm::vec3 worldPos = glm::vec3(world) / world.w;

    // TODO: We currently hard reset to z=0 (This comment should be removed if it's the intended behavior)
    // Given the orthographic projection onto a 2D plane at Z=0, this is the expected behavior.
    worldPos.z = 0.0f; 

    return worldPos;
}

}  // namespace of::render
