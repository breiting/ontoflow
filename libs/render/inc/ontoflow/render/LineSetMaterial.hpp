#pragma once
#include <memory>
#include <ontoflow/render/Material.hpp>

namespace of::render {

class Shader; // Forward declaration

/**
 * \brief Material for rendering line sets.
 *
 * This material uses a basic shader that draws lines without any lighting calculations.
 */
class LineSetMaterial : public Material {
   public:
    /**
     * \brief Constructs a LineSetMaterial.
     * Initializes a shader program internally.
     */
    explicit LineSetMaterial();

    /**
     * \brief Applies the material's properties to the bound shader program.
     * Sets model, view, and projection matrices.
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
    std::shared_ptr<Shader> GetShader() const override;

   private:
    std::shared_ptr<Shader> m_Shader; ///< The shader program used by this material.
};
}  // namespace of::render
