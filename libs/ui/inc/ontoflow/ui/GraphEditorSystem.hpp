#pragma once
/**
 * @file GraphEditorSystem.hpp
 * @brief UI system responsible for drawing and interacting with the
 *        node graph using ImGui + ImNodes.
 */

#include <glm/vec2.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/ui/NodeEditorRegistry.hpp>

namespace of::ui {

/**
 * @class GraphEditorSystem
 * @brief Renders the node graph and handles all interactions such as
 *        creating nodes, linking pins, deleting links, and moving nodes.
 */
class GraphEditorSystem {
   public:
    GraphEditorSystem(domain::Registry& registry, NodeEditorRegistry& editorRegistry);

    /**
     * @brief Toggle visibility of the graph editor window.
     */
    void ToggleVisibility();

    /**
     * @brief Draw the entire node editor panel.
     * @return True if the graph topology changed (links added/removed).
     */
    bool DrawPanel();

   private:
    bool m_visible = true;

    domain::Registry& m_registry;
    NodeEditorRegistry& m_editorReg;

    glm::vec2 m_spawnPos{0.f, 0.f};

    glm::vec2 GetMouseGridPos() const;
};

}  // namespace of::ui
