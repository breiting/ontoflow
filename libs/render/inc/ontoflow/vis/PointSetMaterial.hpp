#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <ontoflow/vis/Material.hpp>
#include <ontoflow/vis/Shader.hpp>

namespace of::vis {

class Shader; // Forward declaration

/**
 * \brief Material for rendering point sets.
 *
 * This material renders points with a specified color and radius,
 * often used for visualizing individual vertices or control points.
 */
class PointSetMaterial : public Material {
   public:
    /**
     * \brief Constructs a PointSetMaterial.
     * Initializes a shader program internally.
     */
    PointSetMaterial();

    /**
     * \brief Applies the material's properties to the bound shader program.
     * Sets view-model and projection matrices, along with point-specific uniforms.
     * \param model The model matrix.
     * \param view The view matrix.
     * \param projection The projection matrix.
     * \param light A shared pointer to the current light source (ignored by this material).
     */
    void Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
               std::shared_ptr<Light> light) override;

    /**
     * \brief Returns the shader program used by this material.
     * \return A shared pointer to the Shader object.
     */
    std::shared_ptr<Shader> GetShader() const override {
        return m_Shader;
    }

    /**
     * \brief Sets the color for rendering points.
     * \param c The new RGBA color.
     */
    void SetColor(const glm::vec4& c) {
        m_Color = c;
    }
    /**
     * \brief Sets the radius for rendering points.
     * \param r The new radius in world units.
     */
    void SetRadius(float r) {
        m_Radius = r;
    }

   private:
    std::shared_ptr<Shader> m_Shader; ///< The shader program used by this material.
    glm::vec4 m_Color{1.0f, 0.5f, 0.0f, 1.0f};  ///< Default orange color.
    float m_Radius = 0.02f;                     ///< Default radius in world units.
};

}  // namespace of::vis
