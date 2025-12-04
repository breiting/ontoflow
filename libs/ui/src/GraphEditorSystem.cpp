#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>

namespace of::ui {

GraphEditorSystem::GraphEditorSystem(domain::Registry& registry, of::cmd::CommandStack& commandStack)
    : m_Registry(registry), m_CommandStack(commandStack) {
}

void GraphEditorSystem::ToggleVisibility() {
    m_IsVisible = !m_IsVisible;
}

void GraphEditorSystem::DrawPanel() {
    if (!m_IsVisible)
        return;

    // Logic commented out for refactor
    if (ImGui::Begin("OntoFlow Graph Editor", &m_IsVisible)) {
         ImNodes::BeginNodeEditor();
         // Node drawing logic will be reimplemented with NodeComponent
         ImNodes::EndNodeEditor();
    }
    ImGui::End();
}

glm::vec2 GraphEditorSystem::GetCurrentMouseGridPosition() {
    ImVec2 mouse_pos_screen = ImGui::GetMousePos();
    ImVec2 editor_panning = ImNodes::EditorContextGetPanning();
    ImVec2 mouse_pos_editor_space = mouse_pos_screen - editor_panning;
    return {mouse_pos_editor_space.x, mouse_pos_editor_space.y};
}

}  // namespace of::ui