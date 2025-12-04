#pragma once
#include <memory>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/editor/ICamera.hpp>
#include <ontoflow/editor/ViewController.hpp> // Not directly used in this header, but might be part of the concept
#include <ontoflow/vis/AxisRenderer.hpp>
#include <ontoflow/vis/DirectionalLight.hpp>
#include <ontoflow/vis/IRenderer.hpp>
#include <ontoflow/vis/LineSet.hpp>
#include <ontoflow/vis/Mesh.hpp>
#include <ontoflow/vis/PointSet.hpp>
#include <unordered_map>

namespace of::vis {

/**
 * \brief Orchestrates the rendering process for the entire CAD scene.
 *
 * The RenderingSystem connects the ECS `Registry` with a concrete `IRenderer`
 * implementation. It manages rendering-specific objects like lights, materials,
 * and geometry (meshes, lines, points), updates them based on ECS changes,
 * and issues draw calls to the renderer.
 */
class RenderingSystem {
   public:
    /**
     * \brief Constructs a RenderingSystem.
     * \param r An r-value reference to a unique_ptr to the IRenderer implementation.
     */
    RenderingSystem(std::unique_ptr<IRenderer> r);

    /**
     * \brief Sets the viewport size for the underlying renderer.
     * \param w Width of the viewport in pixels.
     * \param h Height of the viewport in pixels.
     */
    void SetViewportSize(int w, int h);

    /**
     * \brief Initializes the rendering system.
     * Registers callbacks to the ECS registry to react to component changes.
     * \param registry The central ECS registry.
     * \return True if initialization was successful, false otherwise.
     */
    bool Init(domain::Registry& registry);

    /**
     * \brief Updates the rendering system's internal state based on the ECS registry.
     * This method processes ECS components and prepares renderable objects.
     * \param registry The central ECS registry.
     */
    void Update(domain::Registry& registry);

    /**
     * \brief Renders the current scene.
     * \param cam A pointer to the active camera.
     */
    void Render(editor::ICamera* cam);

    /**
     * \brief Sets the visibility of the coordinate axis display.
     * \param b True to show the axis, false to hide.
     */
    void SetShowAxis(bool b);

   private:
    std::unique_ptr<IRenderer> m_Renderer; ///< The concrete renderer implementation.
    std::shared_ptr<DirectionalLight> m_Light; ///< The primary directional light source.
    std::unique_ptr<AxisRenderer> m_Axis; ///< Renderer for the coordinate axis.
    
    // Maps to store renderable geometry and their materials, keyed by ECS Entity ID.
    std::unordered_map<domain::Entity, std::shared_ptr<Mesh>> m_Meshes;
    std::unordered_map<domain::Entity, uint32_t> m_MeshVersions;
    std::unordered_map<domain::Entity, std::shared_ptr<LineSet>> m_Lines;
    std::unordered_map<domain::Entity, std::shared_ptr<Material>> m_Material;

    bool m_PointsDirty = true; ///< Flag indicating if the point set needs to be re-uploaded.
    std::shared_ptr<PointSet> m_Points; ///< Renderable object for all position components.
    std::shared_ptr<Material> m_PointSetMaterial; ///< Material for rendering point sets.
};

}  // namespace of::vis
