#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>

namespace of::ui {

GraphEditorSystem::GraphEditorSystem(domain::Registry& reg, NodeEditorRegistry& editorReg)
    : m_registry(reg), m_editorReg(editorReg) {
}

void GraphEditorSystem::ToggleVisibility() {
    m_visible = !m_visible;
}

glm::vec2 GraphEditorSystem::GetMouseGridPos() const {
    ImVec2 mouse = ImGui::GetMousePos();
    ImVec2 pan = ImNodes::EditorContextGetPanning();
    ImVec2 local = ImVec2(mouse.x - pan.x, mouse.y - pan.y);
    return {local.x, local.y};
}

bool GraphEditorSystem::DrawPanel() {
    if (!m_visible)
        return false;

    bool graphChanged = false;

    if (ImGui::Begin("OntoFlow Graph Editor", &m_visible)) {
        ImNodes::BeginNodeEditor();

        auto nodes = m_registry.GetEntitiesWith<domain::NodeComponent>();

        // ------------------------------------------------------------
        // Draw nodes
        // ------------------------------------------------------------
        for (auto e : nodes) {
            auto* node = m_registry.GetComponent<domain::NodeComponent>(e);
            auto* name = m_registry.GetComponent<domain::NameComponent>(e);

            int uiNode = m_editorReg.GetNodeId(e);

            ImNodes::BeginNode(uiNode);

            // Title bar
            ImNodes::BeginNodeTitleBar();
            std::string title = name ? name->name : node->definitionID;
            ImGui::Text("%s (%u)", title.c_str(), e);
            ImNodes::EndNodeTitleBar();

            // --------- INPUTS ----------
            for (size_t i = 0; i < node->inputs.size(); ++i) {
                int pin = m_editorReg.GetPinId(e, i, false);
                ImNodes::BeginInputAttribute(pin);
                ImGui::TextUnformatted(node->inputs[i].name.c_str());
                ImNodes::EndInputAttribute();
            }

            // --------- OUTPUTS ----------
            for (size_t i = 0; i < node->outputs.size(); ++i) {
                int pin = m_editorReg.GetPinId(e, i, true);
                ImNodes::BeginOutputAttribute(pin);
                ImGui::TextUnformatted(node->outputs[i].name.c_str());
                ImNodes::EndOutputAttribute();
            }

            ImNodes::EndNode();

            // Sync UI Position
            ImVec2 pos = ImNodes::GetNodeGridSpacePos(uiNode);
            node->ui = {pos.x, pos.y};
        }

        // ------------------------------------------------------------
        // Draw links
        // ------------------------------------------------------------
        for (auto e : nodes) {
            auto* node = m_registry.GetComponent<domain::NodeComponent>(e);

            for (size_t i = 0; i < node->inputs.size(); ++i) {
                auto& conn = node->inputs[i].connection;

                if (conn.targetNodeID == of::domain::INVALID_ENTITY_ID)
                    continue;

                int uiLink = m_editorReg.GetLinkId(e, i);

                int uiStart = m_editorReg.GetPinId(conn.targetNodeID, conn.targetPinIdx, true);

                int uiEnd = m_editorReg.GetPinId(e, i, false);

                ImNodes::Link(uiLink, uiStart, uiEnd);
            }
        }

        // ------------------------------------------------------------
        // Context menu for creating nodes
        // ------------------------------------------------------------
        if (ImNodes::IsEditorHovered() && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup("NodeCreatePopup");
            m_spawnPos = GetMouseGridPos();
        }

        if (ImGui::BeginPopup("NodeCreatePopup")) {
            const auto& defs = engine::NodeRegistry::Instance().GetDefinitions();

            for (const auto& [id, def] : defs) {
                if (ImGui::MenuItem(def.name.c_str())) {
                    auto newNode = engine::NodeRegistry::Instance().SpawnNode(m_registry, id);
                    ImNodes::SetNodeGridSpacePos(m_editorReg.GetNodeId(newNode), ImVec2(m_spawnPos.x, m_spawnPos.y));
                }
            }

            ImGui::EndPopup();
        }

        ImNodes::EndNodeEditor();

        // ------------------------------------------------------------
        // Handle link creation
        // ------------------------------------------------------------
        int startPin, endPin;
        if (ImNodes::IsLinkCreated(&startPin, &endPin)) {
            auto outPin = m_editorReg.DecodePin(startPin);
            auto inPin = m_editorReg.DecodePin(endPin);

            if (outPin.isOutput && !inPin.isOutput) {
                auto* target = m_registry.GetComponent<domain::NodeComponent>(inPin.node);

                target->inputs[inPin.pinIndex].connection = {outPin.node, outPin.pinIndex};
                target->isDirty = true;
                graphChanged = true;
            }
        }

        // ------------------------------------------------------------
        // Handle link deletion
        // ------------------------------------------------------------
        int destroyedLink;
        if (ImNodes::IsLinkDestroyed(&destroyedLink)) {
            auto [node, pin] = m_editorReg.DecodeLink(destroyedLink);

            auto* comp = m_registry.GetComponent<domain::NodeComponent>(node);
            comp->inputs[pin].connection = {};
            comp->isDirty = true;
            graphChanged = true;
        }
    }

    ImGui::End();
    return graphChanged;
}

}  // namespace of::ui
