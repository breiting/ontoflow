#include <memory>
#include <ontoflow/core/Colors.hpp>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Query.hpp>
#include <ontoflow/editor/ViewController.hpp>
#include <ontoflow/render/DirectionalLight.hpp>
#include <ontoflow/render/FlatShadedMaterial.hpp>
#include <ontoflow/render/LineSetMaterial.hpp>
#include <ontoflow/render/PointSetMaterial.hpp>
#include <ontoflow/render/RenderingSystem.hpp>

using namespace of::domain;
using namespace of::editor;

// Default color for the main light source (Sun)
const glm::vec3 SUN_LIGHT = {1.0f, 0.95f, 0.9f};

namespace of::render {

/**
 * \brief Constructs a RenderingSystem.
 * Initializes the light source and materials for points.
 * \param r An r-value reference to a unique_ptr to the IRenderer implementation.
 */
RenderingSystem::RenderingSystem(std::unique_ptr<IRenderer> r) : m_Renderer(std::move(r)) {
    m_Light = std::make_shared<DirectionalLight>();
    m_Light->SetColor(SUN_LIGHT);
    m_PointSetMaterial = std::make_shared<PointSetMaterial>();
    m_Points = std::make_shared<PointSet>();
}

/**
 * \brief Sets the viewport size for the underlying renderer.
 * \param w Width of the viewport in pixels.
 * \param h Height of the viewport in pixels.
 */
void RenderingSystem::SetViewportSize(int w, int h) {
    m_Renderer->SetViewportSize(w, h);
}

/**
 * \brief Initializes the rendering system.
 * Registers callbacks to the ECS registry to react to component changes.
 * \param registry The central ECS registry.
 * \return True if initialization was successful, false otherwise.
 */
bool RenderingSystem::Init(Registry& registry) {
    m_Axis = std::make_unique<AxisRenderer>();
    return m_Axis->Init();
}

/**
 * \brief Sets the visibility of the coordinate axis display.
 * \param b True to show the axis, false to hide.
 */
void RenderingSystem::SetShowAxis(bool b) {
    m_Axis->SetVisible(b);
}

/**
 * \brief Updates the rendering system's internal state based on the ECS registry.
 * This method processes ECS components and prepares renderable objects.
 *
 * \param registry The central ECS registry.
 */
void RenderingSystem::Update(Registry& registry) {
    // Process MeshComponents
    {
        auto entities = HasComponentQuery<MeshComponent>().Execute(registry);
        for (Entity e : entities) {
            auto* comp = registry.GetComponent<MeshComponent>(e);
            if (!comp)
                continue;

            auto& mesh = m_Meshes[e];
            auto& material = m_Material[e];

            bool needUpdate = false;

            if (!mesh) {  // Only create if it doesn't exist
                LOG(Info) << "Creating Mesh for entity: " << e;
                mesh = std::make_shared<Mesh>();
                // Material color is hardcoded here.
                material = std::make_shared<FlatShadedMaterial>(
                    glm::vec3(core::nord::Nord11.r, core::nord::Nord11.g, core::nord::Nord11.b));

                needUpdate = true;
            }

            // Check version
            if (m_MeshVersions[e] != comp->version) {
                needUpdate = true;
                m_MeshVersions[e] = comp->version;
            }

            if (needUpdate) {
                mesh->SetVertices(comp->mesh.vertices);
                mesh->ClearTriangles();

                for (size_t i = 0; i + 2 < comp->mesh.indices.size(); i += 3) {
                    mesh->AddTriangle(comp->mesh.indices[i], comp->mesh.indices[i + 1], comp->mesh.indices[i + 2]);
                }
                mesh->RecalculateNormals();
                mesh->Upload();
            }
        }
    }
}

/**
 * \brief Renders the current scene.
 * \param cam A pointer to the active camera.
 *
 * \note The directional light's direction is currently set to the camera's view direction,
 * meaning the light always follows the camera. This might be a design choice but could
 * be made configurable.
 */
void RenderingSystem::Render(ICamera* cam) {
    if (!cam) {
        LOG(Error) << "RenderingSystem::Render called with null camera.";
        return;
    }
    if (m_Light) {
        m_Light->SetDirection(cam->GetViewDirection());  // Light always comes from camera for now.
    }
    m_Renderer->BeginFrame(cam->GetViewMatrix(), cam->GetProjectionMatrix(), m_Light);

    if (m_Axis && m_Axis->IsVisible())  // Only render axis if visible
        m_Axis->Render(glm::mat4(1.0f), cam->GetViewMatrix(), cam->GetProjectionMatrix());

    // Render Meshes
    for (auto& [e, mesh] : m_Meshes) {
        // Ensure material exists for this mesh
        auto it = m_Material.find(e);
        if (it != m_Material.end() && it->second) {
            m_Renderer->DrawMesh(mesh, it->second, glm::mat4(1.0f));
        } else {
            LOG(Warn) << "RenderingSystem: No material found for mesh entity " << e;
        }
    }
    // Render LineSets
    for (auto& [e, lines] : m_Lines) {
        // Ensure material exists for this lineset
        auto it = m_Material.find(e);
        if (it != m_Material.end() && it->second) {
            m_Renderer->DrawLineSet(lines, it->second, glm::mat4(1.0f));
        } else {
            LOG(Warn) << "RenderingSystem: No material found for lineset entity " << e;
        }
    }
    // Batch render points
    if (m_Points && m_PointSetMaterial)
        m_Renderer->DrawPoints(m_Points, m_PointSetMaterial, glm::mat4(1.0f));

    m_Renderer->EndFrame();
}

}  // namespace of::render
