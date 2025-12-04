#include <ontoflow/vis/DirectionalLight.hpp>
#include <ontoflow/vis/FlatShadedMaterial.hpp>
#include <ontoflow/vis/Shader.hpp>
#include <string>

#include "assets/flat_frag.h"  // flat_frag_glsl
#include "assets/flat_vert.h"  // flat_vert_glsl

namespace of::vis {

FlatShadedMaterial::FlatShadedMaterial(glm::vec3 materialColor) : m_MaterialColor(materialColor) {
    m_Shader = std::make_shared<Shader>(flat_vert_glsl, flat_frag_glsl);
}

void FlatShadedMaterial::Apply(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                               std::shared_ptr<Light> light) {
    m_Shader->Bind();
    m_Shader->SetMat4("u_Model", model);
    m_Shader->SetMat4("u_View", view);
    m_Shader->SetMat4("u_Projection", projection);

    m_Shader->SetVec3("u_LightDir", light->GetDirection());
    m_Shader->SetVec3("u_LightColor", light->GetColor());
    m_Shader->SetVec3("u_MaterialColor", m_MaterialColor);

    if (m_Texture != nullptr) {
        m_Shader->SetBool("u_HasTexture", true);
        m_Texture->Bind(m_TextureLocation);

    } else {
        m_Shader->SetBool("u_HasTexture", false);
    }
}

std::shared_ptr<Shader> FlatShadedMaterial::GetShader() const {
    return m_Shader;
}

void FlatShadedMaterial::SetMaterialColor(const glm::vec3& color) {
    m_MaterialColor = color;
}

void FlatShadedMaterial::SetTexture(std::shared_ptr<Texture> texture) {
    m_Texture = texture;
    m_TextureLocation = m_Shader->GetUniformLocation("u_Texture");
}
}  // namespace of::vis
