#pragma once
/**
 * @file GraphEditorSystem.hpp
 * @brief UI system responsible for drawing and interacting with the
 *        node graph using ImGui + ImNodes.
 */

#include <functional>
#include <glm/vec2.hpp>
#include <imgui.h>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/ui/NodeEditorRegistry.hpp>
#include <string>

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
     * @brief Draws the full editor layout including Toolbar, Node Library, and Graph.
     * @param onCommandCallback Function to execute when a command is submitted.
     */
    void DrawLayout(std::function<void(const std::string&)> onCommandCallback);

    void RequestCommandFocus() {
        m_FocusCommand = true;
    }

   private:
    void DrawToolbar();
    void DrawNodeLibrary();
    void DrawStatusBar();
    void DrawCommandPalette(std::function<void(const std::string&)> onCommandCallback);
    void ApplyTheme();

    bool DrawNodeEditorInternal();
    glm::vec2 GetMouseGridPos() const;
    bool DrawSingleNode(domain::Entity e, domain::NodeComponent& node, domain::NameComponent* nameComp);
    void DrawThinSeparator(float thickness = 1.0f);
    void DumpNodePositions() const;

   private:
    domain::Registry& m_Registry;
    NodeEditorRegistry& m_EditorReg;

    glm::vec2 m_SpawnPos{0.f, 0.f};
    ImGuiTextFilter m_NodeFilter;
    char m_CommandBuffer[256] = "";
    bool m_FocusCommand = false;
    std::string m_LastStatusMessage = "Ready";
};

}  // namespace of::ui
