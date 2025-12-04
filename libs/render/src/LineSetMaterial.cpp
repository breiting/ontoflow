#include <ontoflow/render/LineSetMaterial.hpp>
#include <ontoflow/render/Shader.hpp>

#include "assets/lineset_frag.h"
#include "assets/lineset_vert.h"

namespace of::render {

/**
 * \brief Constructs a LineSetMaterial.
 * Initializes a shader program internally.
 */
LineSetMaterial::LineSetMaterial() {
    m_Shader = std::make_shared<Shader>(lineset_vert_glsl, lineset_frag_glsl);
}

/**
 * \brief Applies the material's properties to the bound shader program.
 * Sets model, view, and projection matrices.
 * \param model The model matrix.
 * \param view The view matrix.
 * \param projection The projection matrix.
 * \param light A shared pointer to the current light source (ignored by this material).
 */
void LineSetMaterial::Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                            std::shared_ptr<Light> /*light*/) {
    m_Shader->Bind();
    m_Shader->SetMat4("u_Model", model);
    m_Shader->SetMat4("u_View", view);
    m_Shader->SetMat4("u_Projection", projection);
}

/**
 * \brief Returns the shader program used by this material.
 * \return A shared pointer to the Shader object.
 */
std::shared_ptr<Shader> LineSetMaterial::GetShader() const {
    return m_Shader;
}
}  // namespace of::render
