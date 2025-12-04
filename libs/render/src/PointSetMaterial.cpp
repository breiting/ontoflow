#include <ontoflow/vis/Light.hpp>
#include <ontoflow/vis/PointSetMaterial.hpp>
#include <ontoflow/vis/Shader.hpp>

#include "assets/pointset_frag.h"
#include "assets/pointset_vert.h"

namespace of::vis {

/**
 * \brief Constructs a PointSetMaterial.
 * Initializes a shader program internally.
 */
PointSetMaterial::PointSetMaterial() {
    m_Shader = std::make_shared<Shader>(pointset_vert_glsl, pointset_frag_glsl);
}

/**
 * \brief Applies the material's properties to the bound shader program.
 * Sets view-model and projection matrices, along with point-specific uniforms.
 * \param model The model matrix.
 * \param view The view matrix.
 * \param projection The projection matrix.
 * \param light A shared pointer to the current light source (ignored by this material).
 */
void PointSetMaterial::Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                             std::shared_ptr<Light> /*light*/) {
    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewModel", view * model);
    m_Shader->SetMat4("u_Proj", projection);

    // uniforms
    m_Shader->SetFloat("u_Radius", m_Radius);
    m_Shader->SetVec4("u_Color", m_Color);
}

}  // namespace of::vis
