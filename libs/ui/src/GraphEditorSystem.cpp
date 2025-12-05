#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>

namespace of::ui {

GraphEditorSystem::GraphEditorSystem(domain::Registry& reg, NodeEditorRegistry& editorReg)
    : m_Registry(reg), m_EditorReg(editorReg) {
}

glm::vec2 GraphEditorSystem::GetMouseGridPos() const {
    ImVec2 mouse = ImGui::GetMousePos();
    ImVec2 pan = ImNodes::EditorContextGetPanning();
    ImVec2 local = ImVec2(mouse.x - pan.x, mouse.y - pan.y);
    return {local.x, local.y};
}

bool GraphEditorSystem::DrawPanel() {
    bool changed = false;

    ImGui::Begin("OntoFlow Graph Editor");

    // Toolbar
    ImGui::Text("Tools:");
    ImGui::SameLine();
    if (ImGui::Button("Dump Positions"))
        DumpNodePositions();

    ImGui::Dummy(ImVec2(0, 6));

    // Node Editor inside Child
    ImGui::BeginChild("NodeRegion", ImVec2(0, 0), true);

    changed = DrawNodeEditorInternal();

    ImGui::EndChild();
    ImGui::End();

    return changed;
}

bool GraphEditorSystem::DrawEmbedded() {
    return DrawNodeEditorInternal();
}

bool GraphEditorSystem::DrawNodeEditorInternal() {
    bool graphChanged = false;

    ImNodes::BeginNodeEditor();

    auto nodes = m_Registry.GetEntitiesWith<domain::NodeComponent>();

    // ------------------------- NODES -------------------------
    for (auto e : nodes) {
        auto* node = m_Registry.GetComponent<domain::NodeComponent>(e);
        auto* name = m_Registry.GetComponent<domain::NameComponent>(e);

        if (!node)
            continue;

        bool nodeChanged = DrawSingleNode(e, *node, name);
        graphChanged = graphChanged || nodeChanged;
    }

    // ------------------------- LINKS -------------------------
    for (auto e : nodes) {
        auto* node = m_Registry.GetComponent<domain::NodeComponent>(e);

        for (std::size_t i = 0; i < node->inputs.size(); ++i) {
            const auto& conn = node->inputs[i].connection;

            if (conn.targetNodeID == of::domain::INVALID_ENTITY_ID)
                continue;

            int uiLink = m_EditorReg.GetLinkId(e, i);

            int uiStart = m_EditorReg.GetPinId(conn.targetNodeID, conn.targetPinIdx, true);
            int uiEnd = m_EditorReg.GetPinId(e, i, false);

            ImNodes::Link(uiLink, uiStart, uiEnd);
        }
    }

    // ------------------ CONTEXT MENU (Create) ----------------
    if (ImNodes::IsEditorHovered() && ImGui::IsMouseClicked(1)) {
        ImGui::OpenPopup("NodeCreatePopup");
        m_SpawnPos = GetMouseGridPos();
    }

    if (ImGui::BeginPopup("NodeCreatePopup")) {
        const auto& defs = engine::NodeRegistry::Instance().GetDefinitions();

        for (const auto& [id, def] : defs) {
            if (ImGui::MenuItem(def.name.c_str())) {
                auto newNode = engine::NodeRegistry::Instance().SpawnNode(m_Registry, id);
                ImNodes::SetNodeGridSpacePos(m_EditorReg.GetNodeId(newNode), ImVec2(m_SpawnPos.x, m_SpawnPos.y));
            }
        }

        ImGui::EndPopup();
    }

    ImNodes::EndNodeEditor();

    // ----------------- LINK CREATION/DELETION ----------------
    int startPin, endPin;
    if (ImNodes::IsLinkCreated(&startPin, &endPin)) {
        auto outPin = m_EditorReg.DecodePin(startPin);
        auto inPin = m_EditorReg.DecodePin(endPin);

        if (outPin.isOutput && !inPin.isOutput) {
            auto* target = m_Registry.GetComponent<domain::NodeComponent>(inPin.node);
            target->inputs[inPin.pinIndex].connection = {outPin.node, outPin.pinIndex};
            target->isDirty = true;
            graphChanged = true;
        }
    }

    int destroyedLink;
    if (ImNodes::IsLinkDestroyed(&destroyedLink)) {
        auto [node, pin] = m_EditorReg.DecodeLink(destroyedLink);
        if (auto* comp = m_Registry.GetComponent<domain::NodeComponent>(node)) {
            comp->inputs[pin].connection = {};
            comp->isDirty = true;
            graphChanged = true;
        }
    }

    return graphChanged;
}

bool GraphEditorSystem::DrawSingleNode(domain::Entity e, domain::NodeComponent& node, domain::NameComponent* nameComp) {
    bool changed = false;

    int uiNode = m_EditorReg.GetNodeId(e);

    const auto* def = engine::NodeRegistry::Instance().GetDefinition(node.definitionID);
    const char* roleLabel = def->category.c_str();

    std::string displayName;
    if (nameComp && !nameComp->name.empty())
        displayName = nameComp->name;
    else if (def)
        displayName = def->name;
    else
        displayName = node.definitionID;

    if (node.ui.x >= 0 && node.ui.y >= 0 && !m_EditorReg.HasSeenNode(uiNode)) {
        ImNodes::SetNodeGridSpacePos(uiNode, {node.ui.x, node.ui.y});
        m_EditorReg.MarkNodeSeen(uiNode);
    }

    ImNodes::BeginNode(uiNode);

    // ---------------- HEADER ----------------
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s    ID: %u", displayName.c_str(), e);
    ImNodes::EndNodeTitleBar();

    // ---------------- ROLE SECTION ----------------
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
        ImGui::SetWindowFontScale(0.90f);
        ImGui::Text("%s", roleLabel);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
    }

    DrawThinSeparator();

    // ---------------------------------------------------------
    // VALUE-Float Node has special layout
    // ---------------------------------------------------------
    bool isValueFloat =
        node.inputs.empty() && node.outputs.size() == 1 && node.outputs[0].type == domain::PinType::DOUBLE;

    if (isValueFloat) {
        int outPin = m_EditorReg.GetPinId(e, 0, true);
        ImNodes::BeginOutputAttribute(outPin);

        ImGui::Text("Value");
        ImGui::SameLine();

        double v = 0.0;
        if (auto* pv = std::get_if<double>(&node.outputs[0].value))
            v = *pv;

        ImGui::PushItemWidth(80.0f);
        if (ImGui::DragScalar("##val", ImGuiDataType_Double, &v, 0.1f)) {
            node.outputs[0].value = v;
            node.isDirty = true;
            changed = true;
        }
        ImGui::PopItemWidth();

        ImNodes::EndOutputAttribute();
    } else {
        // ---------------------------------------------------------
        // INPUT PINS
        // ---------------------------------------------------------
        for (size_t i = 0; i < node.inputs.size(); ++i) {
            auto& pin = node.inputs[i];
            int pinId = m_EditorReg.GetPinId(e, i, false);

            ImNodes::BeginInputAttribute(pinId);
            ImGui::Text("%s", pin.name.c_str());
            ImNodes::EndInputAttribute();
        }

        // ---------------------------------------------------------
        // OUTPUT PINS
        // ---------------------------------------------------------
        for (size_t i = 0; i < node.outputs.size(); ++i) {
            auto& pin = node.outputs[i];
            int pinId = m_EditorReg.GetPinId(e, i, true);

            ImNodes::BeginOutputAttribute(pinId);
            ImGui::Text("%s", pin.name.c_str());
            ImNodes::EndOutputAttribute();
        }
    }

    ImNodes::EndNode();

    // Sync UI position
    ImVec2 pos = ImNodes::GetNodeGridSpacePos(uiNode);
    node.ui = {pos.x, pos.y};

    return changed;
}

void GraphEditorSystem::DrawThinSeparator(float thickness) {
    ImVec2 min = ImGui::GetCursorScreenPos();
    ImVec2 max = {min.x + ImGui::CalcTextSize("W").x * 4.0f, min.y + thickness};

    // A short light line
    ImGui::GetWindowDrawList()->AddLine({min.x, min.y}, {min.x + 80.0f, min.y}, IM_COL32(150, 150, 150, 100),
                                        thickness);

    ImGui::Dummy({80.0f, thickness + 2.0f});
}

void GraphEditorSystem::DumpNodePositions() const {
    LOG(Info) << "---- Node Positions ----";

    auto entities = m_Registry.GetEntitiesWith<domain::NodeComponent>();
    for (auto e : entities) {
        const auto* node = m_Registry.GetComponent<domain::NodeComponent>(e);
        const auto* name = m_Registry.GetComponent<domain::NameComponent>(e);

        if (!node)
            continue;

        std::string displayName = name ? name->name : "(unnamed)";

        LOG(Info) << "Node ID=" << e << " Name=\"" << displayName << "\""
                  << " Pos=(" << node->ui.x << ", " << node->ui.y << ")";
    }

    LOG(Info) << "------------------------";
}

}  // namespace of::ui
