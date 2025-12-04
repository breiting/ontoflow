#include <algorithm>
#include <glm/ext/matrix_clip_space.hpp>  // glm::perspective
#include <glm/ext/matrix_transform.hpp>   // glm::lookat
#include <glm/glm.hpp>
#include <ontoflow/vis/Camera3D.hpp>

// Z = UP
constexpr glm::vec3 UP_VECTOR = glm::vec3(0.f, 0.f, 1.f);

namespace of::vis {

/**
 * \brief Constructs a 3D camera.
 * \param radius Initial distance from the target.
 * \param pitch Initial pitch angle in degrees.
 * \param yaw Initial yaw angle in degrees.
 */
Camera3D::Camera3D(float radius, float pitch, float yaw)
    : m_Rotation({yaw, pitch}),
      m_RotationVelocity(0.0f),
      m_Radius(radius),
      m_DampingFactor(0.90f),
      m_VelocityThreshold(0.001f),
      m_LastMouse(0.0f),
      m_FirstMouse(true) {
    UpdatePosition();
}

/**
 * \brief Called when a mouse interaction starts.
 * Resets internal mouse tracking for delta calculation.
 */
void Camera3D::OnMouseStart() {
    m_FirstMouse = true;
}

/**
 * \brief Calculates mouse movement delta.
 * \param xpos Current X-coordinate of the mouse.
 * \param ypos Current Y-coordinate of the mouse.
 * \return A glm::vec2 representing the mouse delta.
 */
glm::vec2 Camera3D::OnMouseMove(double xpos, double ypos) {
    if (m_FirstMouse) {
        m_LastMouse = glm::vec2(xpos, ypos);
        m_FirstMouse = false;
    }

    glm::vec2 currentMouse(xpos, ypos);
    glm::vec2 delta = currentMouse - m_LastMouse;
    m_LastMouse = currentMouse;

    float sensitivity = 2.0f;
    delta *= sensitivity;
    return delta;
}

/**
 * \brief Handles mouse rotation input.
 * Adjusts the camera's pitch and yaw based on mouse movement.
 * \param xpos Current X-coordinate of the mouse.
 * \param ypos Current Y-coordinate of the mouse.
 */
void Camera3D::OnMouseRotation(double xpos, double ypos) {
    auto delta = OnMouseMove(xpos, ypos);
    float speed = 1.0f;

    m_RotationVelocity.x -= delta.x * speed;  // Yaw often inverted
    m_RotationVelocity.y += delta.y * speed;
    UpdatePosition();
}

/**
 * \brief Handles mouse panning input.
 * Moves the camera's target point based on mouse movement.
 * \param xpos Current X-coordinate of the mouse.
 * \param ypos Current Y-coordinate of the mouse.
 */
void Camera3D::OnMousePan(double xpos, double ypos) {
    auto delta = OnMouseMove(xpos, ypos);

    glm::vec3 viewDir = GetViewDirection();
    glm::vec3 right = glm::normalize(glm::cross(viewDir, UP_VECTOR));
    glm::vec3 up = glm::normalize(glm::cross(right, viewDir));

    m_Target -= right * delta.x * 0.005f;
    m_Target += up * delta.y * 0.005f;
    UpdatePosition();
}

/**
 * \brief Handles mouse scroll input for zooming.
 * Adjusts the camera's distance (radius) from the target.
 * \param yoffset The scroll offset (usually vertical).
 */
void Camera3D::OnMouseScroll(double yoffset) {
    float factor = static_cast<float>(yoffset);
    float zoomSpeed = 0.2f;
    factor = copysignf(powf(std::abs(factor), 1.2f), factor);  // Non-linear zoom
    m_Radius -= factor * zoomSpeed;
    m_Radius = std::clamp(m_Radius, 1.0f, 100.0f);
    UpdatePosition();
}

/**
 * \brief Updates the camera's internal state, applying damping to rotation.
 * \param deltaTime Time delta since last frame.
 */
void Camera3D::Update(float deltaTime) {
    m_Rotation += m_RotationVelocity * deltaTime;
    m_Rotation.y = std::clamp(m_Rotation.y, -89.0f, 89.0f);
    m_RotationVelocity *= m_DampingFactor;
    if (glm::length(m_RotationVelocity) < m_VelocityThreshold) {
        m_RotationVelocity = glm::vec2(0.0f);
    }
    UpdatePosition();
}

/**
 * \brief Updates the camera's position based on its target, radius, and rotation.
 */
void Camera3D::UpdatePosition() {
    float radPitch = glm::radians(m_Rotation.y);
    float radYaw = glm::radians(m_Rotation.x);

    glm::vec3 offset;

    offset.x = m_Radius * cos(radPitch) * cos(radYaw);
    offset.y = m_Radius * cos(radPitch) * sin(radYaw);
    offset.z = m_Radius * sin(radPitch);

    m_Position = m_Target + offset;
}

/**
 * \brief Sets the absolute position of the camera.
 * \param position The new position vector.
 */
void Camera3D::SetPosition(const glm::vec3& position) {
    m_Position = position;
}

/**
 * \brief Sets the target point the camera is looking at.
 * \param target The new target point vector.
 */
void Camera3D::SetTarget(const glm::vec3& target) {
    m_Target = target;
}

/**
 * \brief Returns the current position of the camera.
 * \return A const reference to the camera's position.
 */
const glm::vec3& Camera3D::GetPosition() const {
    return m_Position;
}

/**
 * \brief Returns the current target point of the camera.
 * \return A const reference to the camera's target.
 */
const glm::vec3& Camera3D::GetTarget() const {
    return m_Target;
}

/**
 * \brief Returns the view matrix for the 3D camera.
 * \return A glm::mat4 representing the camera's view transformation.
 */
glm::mat4 Camera3D::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Target, UP_VECTOR);
}

/**
 * \brief Returns the projection matrix for the 3D camera.
 * \return A glm::mat4 representing the perspective projection.
 */
glm::mat4 Camera3D::GetProjectionMatrix() const {
    // TODO: FOV should be dynamic
    float fov = 45.0;
    return glm::perspective(glm::radians(fov), m_AspectRatio, 0.01f, 500.0f);
}

/**
 * \brief Returns the view direction of the camera.
 * \return A glm::vec3 representing the normalized vector from position to target.
 */
glm::vec3 Camera3D::GetViewDirection() const {
    return glm::normalize(m_Target - m_Position);
}

/**
 * \brief Converts screen coordinates to 3D world coordinates using ray picking.
 * This method casts a ray from the camera through the screen point and calculates
 * its intersection with the XY-plane (Z=0).
 * \param screenX X-coordinate on screen.
 * \param screenY Y-coordinate on screen.
 * \return A glm::vec3 representing the world coordinates on the XY-plane.
 */
glm::vec3 Camera3D::ScreenToWorld(double x, double y) const {
    float nx = (2.0f * x / m_VP.x) - 1.0f;
    float ny = 1.0f - (2.0f * y / m_VP.y);

    glm::vec4 cRayStart(nx, ny, -1.0f, 1.0f);
    glm::vec4 cRayEnd(nx, ny, 1.0f, 1.0f);

    glm::mat4 invVP = glm::inverse(GetProjectionMatrix() * GetViewMatrix());
    glm::vec4 rayStartWorld = invVP * cRayStart;
    rayStartWorld /= rayStartWorld.w;
    glm::vec4 rayEndWorld = invVP * cRayEnd;
    rayEndWorld /= rayEndWorld.w;

    glm::vec3 rayOrigin = glm::vec3(rayStartWorld);
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayEndWorld - rayStartWorld));

    // Intersection with XY-plane (z=0)
    float denom = rayDir.z;
    if (fabs(denom) < 1e-6f)
        return glm::vec3(0, 0, 0);  // Ray is parallel to XY-plane

    float t = -rayOrigin.z / rayDir.z;
    return rayOrigin + t * rayDir;
}

}  // namespace of::vis
