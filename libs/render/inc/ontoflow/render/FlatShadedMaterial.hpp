#pragma once
#include <memory>
#include <ontoflow/render/Material.hpp>
#include <ontoflow/render/Texture.hpp>

namespace of::render {

class Shader; // Forward declaration

/**
 * \brief Material for flat-shaded 3D objects.
 *
 * This material uses a simple Phong-like lighting model with a single directional
 * light source. It supports a base material color and an optional texture.
 */
class FlatShadedMaterial : public Material {
   public:
    /**
     * \brief Constructs a FlatShadedMaterial with a given base color.
     * Initializes a shader program internally.
     * \param materialColor The base color of the material.
     */
    explicit FlatShadedMaterial(glm::vec3 materialColor = {0.1f, 0.1f, 0.1f});

    /**
     * \brief Applies the material's properties to the bound shader program.
     * Sets model, view, projection matrices, light properties, and material color.
     * Binds texture if present.
     * \param model The model matrix.
     * \param view The view matrix.
     * \param projection The projection matrix.
     * \param light A shared pointer to the current light source in the scene.
     */
    void Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
               std::shared_ptr<Light> light) override;

    /**
     * \brief Returns the shader program used by this material.
     * \return A shared pointer to the Shader object.
     */
    std::shared_ptr<Shader> GetShader() const override;

    /**
     * \brief Sets the base color of the material.
     * \param color The new material color.
     */
    void SetMaterialColor(const glm::vec3& color);

    /**
     * \brief Sets a texture for this material.
     * If a texture is set, the shader will use it, otherwise it uses the material color.
     * \param texture A shared pointer to the Texture object.
     */
    void SetTexture(std::shared_ptr<Texture> texture);

   private:
    std::shared_ptr<Shader> m_Shader; ///< The shader program used by this material.
    glm::vec3 m_MaterialColor; ///< The base color of the material.
    std::shared_ptr<Texture> m_Texture; ///< Optional texture for the material.
    int m_TextureLocation; ///< The uniform location for the texture sampler.
};
}  // namespace of::render
