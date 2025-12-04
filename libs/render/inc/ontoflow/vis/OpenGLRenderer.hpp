#pragma once
#include <glm/mat4x4.hpp>
#include <memory>
#include <ontoflow/vis/IRenderer.hpp>
#include <ontoflow/vis/Material.hpp>

namespace of::vis {

class OpenGLRenderer : public IRenderer {
   public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    void SetViewportSize(int w, int h) override;

    void BeginFrame(const glm::mat4& view, const glm::mat4& proj, std::shared_ptr<Light> light) override;
    void EndFrame() override;

    void DrawMesh(const std::shared_ptr<BaseGeometry> mesh, std::shared_ptr<Material> mat,
                  const glm::mat4& model) override;
    void DrawLineSet(const std::shared_ptr<BaseGeometry> lines, std::shared_ptr<Material> mat,
                     const glm::mat4& model) override;
    void DrawPoints(const std::shared_ptr<BaseGeometry> points, std::shared_ptr<Material> mat,
                    const glm::mat4& model) override;

    void ToggleWireframe() override;

   private:
    std::shared_ptr<Light> m_Light;
    glm::mat4 m_View{1.0f};
    glm::mat4 m_Proj{1.0f};
    bool m_Wireframe;
};

}  // namespace of::vis
