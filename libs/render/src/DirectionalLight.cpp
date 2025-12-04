#include <ontoflow/vis/DirectionalLight.hpp>

namespace of::vis {

/**
 * \brief Returns the color of the light.
 * \return The RGB color vector.
 */
glm::vec3 DirectionalLight::GetColor() const {
    return m_Color;
}

/**
 * \brief Returns the direction of the light.
 * \return The direction vector.
 */
glm::vec3 DirectionalLight::GetDirection() const {
    return m_Direction;
}

/**
 * \brief Returns the position of the light.
 * For a directional light, this is always (0,0,0) as position is irrelevant.
 * \return A zero vector.
 */
glm::vec3 DirectionalLight::GetPosition() const {
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

/**
 * \brief Sets the color of the directional light.
 * \param color The RGB color vector.
 */
void DirectionalLight::SetColor(const glm::vec3 &color) {
    m_Color = color;
}

/**
 * \brief Sets the direction of the directional light.
 * \param dir The direction vector.
 */
void DirectionalLight::SetDirection(const glm::vec3 &dir) {
    m_Direction = dir;
}
}  // namespace of::vis
