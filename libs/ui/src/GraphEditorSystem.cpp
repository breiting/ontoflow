#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>

namespace of::ui {

GraphEditorSystem::GraphEditorSystem(domain::Registry& reg, NodeEditorRegistry& editorReg)
    : m_registry(reg), m_editorReg(editorReg) {
}

glm::vec2 GraphEditorSystem::GetMouseGridPos() const {
    ImVec2 mouse = ImGui::GetMousePos();
    ImVec2 pan = ImNodes::EditorContextGetPanning();
    ImVec2 local = ImVec2(mouse.x - pan.x, mouse.y - pan.y);
    return {local.x, local.y};
}

bool GraphEditorSystem::DrawPanel() {
    bool changed = false;

    if (ImGui::Begin("OntoFlow Graph Editor")) {
        changed = DrawNodeEditorInternal();
    }

    ImGui::End();
    return changed;
}
bool GraphEditorSystem::DrawEmbedded() {
    // Do not open/close any ImGui window here.
    // Just render ImNodes content into the currently active window.
    return DrawNodeEditorInternal();
}

bool GraphEditorSystem::DrawNodeEditorInternal() {
    bool graphChanged = false;

    ImNodes::BeginNodeEditor();

    auto nodes = m_registry.GetEntitiesWith<domain::NodeComponent>();

    // ------------------------- NODES -------------------------
    for (auto e : nodes) {
        auto* node = m_registry.GetComponent<domain::NodeComponent>(e);
        auto* name = m_registry.GetComponent<domain::NameComponent>(e);

        int uiNode = m_editorReg.GetNodeId(e);

        ImNodes::BeginNode(uiNode);

        ImNodes::BeginNodeTitleBar();
        std::string title = name ? name->name : node->definitionID;
        ImGui::Text("%s (%u)", title.c_str(), e);
        ImNodes::EndNodeTitleBar();

        // Inputs
        for (std::size_t i = 0; i < node->inputs.size(); ++i) {
            int pin = m_editorReg.GetPinId(e, i, false);
            ImNodes::BeginInputAttribute(pin);
            ImGui::TextUnformatted(node->inputs[i].name.c_str());
            ImNodes::EndInputAttribute();
        }

        // Outputs
        for (std::size_t i = 0; i < node->outputs.size(); ++i) {
            int pin = m_editorReg.GetPinId(e, i, true);
            ImNodes::BeginOutputAttribute(pin);
            ImGui::TextUnformatted(node->outputs[i].name.c_str());
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();

        // Sync position back to NodeComponent
        ImVec2 pos = ImNodes::GetNodeGridSpacePos(uiNode);
        node->ui = {pos.x, pos.y};
    }

    // ------------------------- LINKS -------------------------
    for (auto e : nodes) {
        auto* node = m_registry.GetComponent<domain::NodeComponent>(e);

        for (std::size_t i = 0; i < node->inputs.size(); ++i) {
            const auto& conn = node->inputs[i].connection;

            if (conn.targetNodeID == of::domain::INVALID_ENTITY_ID)
                continue;

            int uiLink = m_editorReg.GetLinkId(e, i);

            int uiStart = m_editorReg.GetPinId(conn.targetNodeID, conn.targetPinIdx, true);
            int uiEnd = m_editorReg.GetPinId(e, i, false);

            ImNodes::Link(uiLink, uiStart, uiEnd);
        }
    }

    // ------------------ CONTEXT MENU (Create) ----------------
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

    // ----------------- LINK CREATION/DELETION ----------------
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

    int destroyedLink;
    if (ImNodes::IsLinkDestroyed(&destroyedLink)) {
        auto [node, pin] = m_editorReg.DecodeLink(destroyedLink);
        if (auto* comp = m_registry.GetComponent<domain::NodeComponent>(node)) {
            comp->inputs[pin].connection = {};
            comp->isDirty = true;
            graphChanged = true;
        }
    }

    return graphChanged;
}

}  // namespace of::ui
