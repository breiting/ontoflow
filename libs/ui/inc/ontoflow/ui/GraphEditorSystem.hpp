#pragma once

#include <glm/vec2.hpp>

#include "ontoflow/command/CommandStack.hpp"
#include "ontoflow/domain/Registry.hpp"

// Forward declaration for ImGui context and ImNodes
namespace ImNodes {
using Context = void;
}

namespace of::ui {

/**
 * \brief System responsible for drawing the node graph editor.
 *
 * Manages the visualization and interaction with the parametric graph
 * using ImNodes. It handles rendering of parameter and feature nodes,
 * as well as the connections (links) between them.
 */
class GraphEditorSystem final {
   public:
    /**
     * \brief Constructs the GraphEditorSystem.
     * \param registry Reference to the ECS registry containing the model data.
     */
    explicit GraphEditorSystem(domain::Registry& registry);

    /**
     * \brief Draws the main graph editor panel.
     * Should be called inside an ImGui frame.
     */
    void DrawPanel();

    /**
     * \brief Toggles the visibility of the graph editor window.
     */
    void ToggleVisibility();

    /**
     * \brief Checks if the graph editor window is currently visible.
     * \return True if visible, false otherwise.
     */
    bool IsVisible() const {
        return m_IsVisible;
    }

   private:
    domain::Registry& m_Registry;
    bool m_IsVisible = true;

    // Helper to store position of newly created nodes
    glm::vec2 m_CurrentMouseGridPosition{0.0f, 0.0f};

    /**
     * \brief Updates and returns the current mouse position in grid space.
     * \return The mouse position in the ImNodes grid coordinate system.
     */
    glm::vec2 GetCurrentMouseGridPosition();
};

}  // namespace of::ui
