#include <ontoflow/ui/StatusBar.hpp>
#include <imgui.h>

namespace of::ui {

void StatusBar::ShowMessage(const std::string& message, float duration) {
    m_Message = message;
    m_Timer = duration;
}

void StatusBar::Draw(float dt) {
    if (m_Timer > 0.0f) {
        m_Timer -= dt;
        if (m_Timer <= 0.0f) {
            m_Message.clear();
        }
    }

    // Draw background and text
    // Assuming this is called inside a window or we draw a window here.
    // The parent (GraphEditorSystem) creates the "StatusBar" window.
    
    if (!m_Message.empty()) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", m_Message.c_str());
    } else {
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Ready");
    }
}

}  // namespace of::ui
