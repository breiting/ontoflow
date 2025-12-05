#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <ontoflow/core/Colors.hpp>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/NodeInspector.hpp>

namespace of::ui {

using namespace of::core;

void NodeInspector::Open(domain::Entity e, domain::Registry& reg) {
    m_Target = e;
    m_IsOpen = true;

    // 1. Transaction: Create a temporary copy of the data
    if (auto* node = reg.GetComponent<domain::NodeComponent>(e)) {
        m_TempNode = *node;
    }

    if (auto* name = reg.GetComponent<domain::NameComponent>(e)) {
        m_TempName = name->name;
    } else {
        m_TempName = "";
    }

    // Reset specific state if needed
    // m_TempNode.isDirty is not strictly needed for the temp copy, but we keep it.

    ImGui::OpenPopup("Node Inspector");
}

InspectorAction NodeInspector::Draw(domain::Registry& reg) {
    if (!m_IsOpen)
        return InspectorAction::None;

    InspectorAction action = InspectorAction::None;

    // --- Visual Style: Glass Look ---
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.95f);  // Slightly transparent

    // Center the window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 0));  // Auto height, fixed width

    // Modal flags
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    // Darken background
    // ImGui handles dimming automatically with Modal popups usually, but we can tweak ModalWindowDimBg if needed.

    ImGui::SetNextWindowFocus();

    bool p_open = true;
    if (ImGui::BeginPopupModal("Node Inspector", &p_open, flags)) {
        DrawHeader();
        ImGui::Separator();
        DrawGeneralSection();
        ImGui::Separator();
        DrawPinsSection();
        ImGui::Separator();
        DrawActionsFooter();

        // --- Shortcuts ---
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            action = InspectorAction::Cancel;
            m_IsOpen = false;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            // Only save if not inputting text (avoids conflict with multi-line, but here inputs are single line)
            // Or just check simple enter.
            if (!ImGui::IsAnyItemActive()) {
                action = InspectorAction::Save;
                // Save logic handled below
            }
        }

        // Handle Button Actions triggered inside sub-functions
        // We need a way to propagate the action out.
        // Since ImGui is immediate mode, we can check button clicks inside the sub-functions
        // or store the requested action in a member temporarily?
        // Actually, simpler: Check logic inside this scope or pass action reference?
        // Refactoring: Let's handle footer here to capture the return.

        // ... wait, I put DrawActionsFooter() as a helper. Let's assume it returns a state or modifies 'action'.
        // I'll modify the helper signature or inline it for simplicity in flow control.

        // Actually, let's do the footer logic here to keep 'action' local and simple.
        // Re-implementing DrawActionsFooter logic here:

        ImGui::Dummy(ImVec2(0, 10));

        // DELETE (Left)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.88f, 0.46f, 0.44f, 1.0f));  // Nord11 Red
        if (ImGui::Button("Delete Node", ImVec2(100, 30))) {
            action = InspectorAction::Delete;
            ImGui::CloseCurrentPopup();
            m_IsOpen = false;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Spacing to push Cancel/Save to the right
        float space = ImGui::GetContentRegionAvail().x - 140;  // 70 + 70 approx
        if (space > 0)
            ImGui::Dummy(ImVec2(space, 0));
        ImGui::SameLine();

        // CANCEL
        if (ImGui::Button("Cancel", ImVec2(70, 30))) {
            action = InspectorAction::Cancel;
            ImGui::CloseCurrentPopup();
            m_IsOpen = false;
        }

        ImGui::SameLine();

        // SAVE
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.37f, 0.51f, 0.67f, 1.0f));  // Nord10 Blue
        if (ImGui::Button("Save", ImVec2(70, 30)) ||
            (ImGui::IsKeyPressed(ImGuiKey_Enter) && !ImGui::IsAnyItemActive())) {
            action = InspectorAction::Save;
            ImGui::CloseCurrentPopup();
            m_IsOpen = false;
        }
        ImGui::PopStyleColor();

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(4);

    // --- Commit Logic ---
    if (action == InspectorAction::Save) {
        // Apply changes back to registry
        if (auto* node = reg.GetComponent<domain::NodeComponent>(m_Target)) {
            *node = m_TempNode;
            node->isDirty = true;  // Mark dirty so graph updates
        }

        if (!m_TempName.empty()) {
            if (reg.HasComponent<domain::NameComponent>(m_Target)) {
                reg.GetComponent<domain::NameComponent>(m_Target)->name = m_TempName;
            } else {
                reg.AddComponent(m_Target, domain::NameComponent{m_TempName});
            }
        }
    }
    // Delete is handled by the caller (GraphEditorSystem) to remove connections properly

    return action;
}

void NodeInspector::DrawHeader() {
    ImGui::Dummy(ImVec2(0, 5));

    // Get Definition Name
    std::string typeName = m_TempNode.definitionID;
    if (const auto* def = engine::NodeRegistry::Instance().GetDefinition(typeName)) {
        typeName = def->name;
    }

    // Centered Header
    float width = ImGui::GetWindowWidth();
    float textW = ImGui::CalcTextSize(typeName.c_str()).x;
    ImGui::SetCursorPosX((width - textW) * 0.5f);
    ImGui::TextColored(ImVec4(0.56f, 0.84f, 0.90f, 1.0f), "%s", typeName.c_str());  // Nord9

    ImGui::Dummy(ImVec2(0, 5));
}

void NodeInspector::DrawGeneralSection() {
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextDisabled("General");

    ImGui::BeginTable("GeneralTable", 2, ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Value");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Name");

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##Name", &m_TempName);

    ImGui::EndTable();
    ImGui::Dummy(ImVec2(0, 5));
}

void NodeInspector::DrawPinsSection() {
    if (m_TempNode.inputs.empty() && m_TempNode.outputs.empty())
        return;

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextDisabled("Signals & Slots");

    if (ImGui::BeginTable("PinsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);        // Left: Name + Editor
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 20.0f);  // Right: Status Dot

        // Inputs
        for (auto& pin : m_TempNode.inputs) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", pin.name.c_str());

            bool connected = (pin.connection.targetNodeID != domain::INVALID_ENTITY_ID);

            if (!connected) {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1);
                DrawPinEditor(pin);
            } else {
                ImGui::SameLine();
                ImGui::TextDisabled("(Linked)");
            }

            ImGui::TableSetColumnIndex(1);
            DrawConnectionStatus(connected);
        }

        // Outputs (Just listing them for info, usually not editable in Inspector unless specific logic)
        for (auto& pin : m_TempNode.outputs) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s (Out)", pin.name.c_str());

            // Check if this output drives anything?
            // Our graph is stored Input-centric. Finding if output is connected requires query.
            // For now, just show green/active.
            ImGui::SameLine();
            DrawPinEditor(pin);  // Allow editing static output values (like Value nodes)

            ImGui::TableSetColumnIndex(1);
            DrawConnectionStatus(true);  // Assume output is available
        }

        ImGui::EndTable();
    }
}

void NodeInspector::DrawConnectionStatus(bool connected) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    float r = 4.0f;
    p.x += 10.0f;  // Center in column approx
    p.y += ImGui::GetTextLineHeight() * 0.5f;

    ImU32 col = connected ? IM_COL32(163, 190, 140, 255) : IM_COL32(76, 86, 106, 255);  // Nord14 (Green) : Nord3 (Grey)
    ImGui::GetWindowDrawList()->AddCircleFilled(p, r, col);
}

void NodeInspector::DrawPinEditor(domain::Pin& pin) {
    switch (pin.type) {
        case domain::PinType::DOUBLE: {
            double v = 0.0;
            if (auto* pv = std::get_if<double>(&pin.value))
                v = *pv;
            if (ImGui::InputDouble(("##" + pin.name).c_str(), &v, 0.0, 0.0, "%.3f")) {
                pin.value = v;
            }
            break;
        }
        case domain::PinType::INT: {
            int v = 0;
            if (auto* pv = std::get_if<int>(&pin.value))
                v = *pv;
            if (ImGui::InputInt(("##" + pin.name).c_str(), &v)) {
                pin.value = v;
            }
            break;
        }
        case domain::PinType::BOOL: {
            bool v = false;
            if (auto* pv = std::get_if<bool>(&pin.value))
                v = *pv;
            if (ImGui::Checkbox(("##" + pin.name).c_str(), &v)) {
                pin.value = v;
            }
            break;
        }
        case domain::PinType::STRING: {
            std::string v = "";
            if (auto* pv = std::get_if<std::string>(&pin.value))
                v = *pv;
            if (ImGui::InputText(("##" + pin.name).c_str(), &v)) {
                pin.value = v;
            }
            break;
        }
        case domain::PinType::VEC3: {
            glm::vec3 v(0.f);
            if (auto* pv = std::get_if<glm::vec3>(&pin.value))
                v = *pv;
            float fv[3] = {v.x, v.y, v.z};
            if (ImGui::InputFloat3(("##" + pin.name).c_str(), fv, "%.3f")) {
                pin.value = glm::vec3(fv[0], fv[1], fv[2]);
            }
            break;
        }
        default:
            ImGui::TextDisabled("N/A");
            break;
    }
}

void NodeInspector::DrawActionsFooter() {
    // Implemented inside Draw() for control flow
}

}  // namespace of::ui
