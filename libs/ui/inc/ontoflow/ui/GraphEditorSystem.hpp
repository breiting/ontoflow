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
     * @brief Draw the entire node editor panel.
     * @return True if the graph topology changed (links added/removed).
     */
    bool DrawPanel();

    /**
     * @brief Draws the node editor into the *currently active* ImGui window.
     *
     * This does not open or close any ImGui window. It expects that the caller
     * already called ImGui::Begin() on some window. This is ideal for embedding
     * the node editor as the main content region.
     *
     * @return True if the graph topology changed (links added/removed).
     */
    bool DrawEmbedded();

   private:
    bool DrawNodeEditorInternal();
    glm::vec2 GetMouseGridPos() const;

   private:
    domain::Registry& m_registry;
    NodeEditorRegistry& m_editorReg;

    glm::vec2 m_spawnPos{0.f, 0.f};
};

}  // namespace of::ui
