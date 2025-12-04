#pragma once
#include <glm/glm.hpp>
#include <ontoflow/vis/Light.hpp>

namespace of::vis {

/**
 * \brief Represents a directional light source in the scene.
 *
 * Directional lights have no position, only a direction, and their rays
 * are considered parallel throughout the scene.
 */
class DirectionalLight : public Light {
   public:
    /**
     * \brief Sets the color of the directional light.
     * \param color The RGB color vector.
     */
    void SetColor(const glm::vec3 &color);

    /**
     * \brief Sets the direction of the directional light.
     * \param dir The direction vector.
     */
    void SetDirection(const glm::vec3 &dir);

    /**
     * \brief Returns the color of the light.
     * \return The RGB color vector.
     */
    virtual glm::vec3 GetColor() const override;

    /**
     * \brief Returns the direction of the light.
     * \return The direction vector.
     */
    virtual glm::vec3 GetDirection() const override;

    /**
     * \brief Returns the position of the light.
     * For a directional light, this is always (0,0,0) as position is irrelevant.
     * \return A zero vector.
     */
    virtual glm::vec3 GetPosition() const override;

   private:
    glm::vec3 m_Direction; ///< The direction of the light.
    glm::vec3 m_Color;     ///< The color of the light.
};
}  // namespace of::vis
