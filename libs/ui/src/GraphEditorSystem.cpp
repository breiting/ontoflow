#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>

using namespace of::engine;

namespace {
// Bit layout: [1 bit: isLink] [1 bit: isOutput] [8 bits: pinIndex] [22 bits: entityID]

// Node ID = Entity ID (Direct mapping)

int GetPinID(of::domain::EntityID nodeID, int pinIndex, bool isOutput) {
    // Output bit is 30, PinIdx is 22-29, Entity is 0-21
    int id = (static_cast<int>(nodeID) & 0x003FFFFF) | ((pinIndex & 0xFF) << 22);

    if (isOutput)
        id |= (1 << 30);

    return id;
}

// Since connections live on the Input Pin, the Link ID is derived from the Input Pin ID.
// We set the 31st bit to distinguish it from the Pin itself.
int GetLinkID(of::domain::EntityID nodeID, int pinIndex) {
    int pinId = GetPinID(nodeID, pinIndex, false);  // always input logic for links
    return pinId | (1 << 31);                       // Top bit set = Link
}

struct DecodedPin {
    of::domain::EntityID nodeID;
    int pinIndex;
    bool isOutput;
};

DecodedPin DecodePinID(int id) {
    DecodedPin d;
    d.nodeID = static_cast<of::domain::EntityID>(id & 0x003FFFFF);
    d.pinIndex = (id >> 22) & 0xFF;
    d.isOutput = (id & (1 << 30)) != 0;
    return d;
}

// Helper for Link Decoding (we strip the top bit and treat as Pin)
DecodedPin DecodeLinkID(int id) {
    return DecodePinID(id & ~(1 << 31));
}
}  // namespace

namespace of::ui {

GraphEditorSystem::GraphEditorSystem(domain::Registry& registry) : m_Registry(registry) {
}

void GraphEditorSystem::ToggleVisibility() {
    m_IsVisible = !m_IsVisible;
}

// Helper to get color for pin type
int GetPinColor(of::domain::PinType type) {
    switch (type) {
        case of::domain::PinType::FLOAT:
            return IM_COL32(200, 200, 200, 255);  // Grey
        case of::domain::PinType::INT:
            return IM_COL32(100, 200, 100, 255);  // Green
        case of::domain::PinType::GEOMETRY:
            return IM_COL32(255, 100, 100, 255);  // Red
        case of::domain::PinType::VEC3:
            return IM_COL32(100, 100, 255, 255);  // Blue
        default:
            return IM_COL32(255, 255, 255, 255);
    }
}

void GraphEditorSystem::DrawPanel() {
    if (!m_IsVisible)
        return;

    if (ImGui::Begin("OntoFlow Graph Editor", &m_IsVisible)) {
        ImNodes::BeginNodeEditor();

        // ---------------------------------------------------------
        // 1. DRAW NODES
        // ---------------------------------------------------------
        // We iterate all entities that have a NodeComponent
        std::vector<of::domain::Entity> entities = m_Registry.GetEntitiesWith<of::domain::NodeComponent>();

        for (auto entity : entities) {
            auto* node = m_Registry.GetComponent<of::domain::NodeComponent>(entity);
            auto* nameComp = m_Registry.GetComponent<of::domain::NameComponent>(entity);

            // Sync Position (First Run or External Change)
            // Note: In a real app, you might want to only set this if it changed significantly
            // or use ImNodes::SetNodeGridSpacePos only when loading a file.
            // For now, let's trust ImNodes as the source of truth for UI position after init.
            if (node->ui.x == 0.0f && node->ui.y == 0.0f) {
                ImNodes::SetNodeGridSpacePos(static_cast<int>(entity), ImVec2(100.0f * (float)entity, 100.0f));
                // Mark as initialized so we don't reset it
                node->ui.x = -1.0f;
            }

            ImNodes::BeginNode(static_cast<int>(entity));

            // Header
            ImNodes::BeginNodeTitleBar();
            std::string title = nameComp ? nameComp->name : node->definitionID;
            // Add Entity ID for debug visibility
            title += " (" + std::to_string(entity) + ")";
            ImGui::TextUnformatted(title.c_str());
            ImNodes::EndNodeTitleBar();

            // Inputs
            for (size_t i = 0; i < node->inputs.size(); ++i) {
                int pinId = GetPinID(entity, static_cast<int>(i), false);
                ImNodes::PushColorStyle(ImNodesCol_Pin, GetPinColor(node->inputs[i].type));
                ImNodes::BeginInputAttribute(pinId);

                ImGui::Text("%s", node->inputs[i].name.c_str());

                // If not connected, show a small drag widget for basic types?
                // For now, keep it simple.
                if (node->inputs[i].connection.targetNodeID == of::domain::INVALID_ENTITY_ID) {
                    if (std::holds_alternative<double>(node->inputs[i].value)) {
                        ImGui::SameLine();
                        double* val = &std::get<double>(node->inputs[i].value);
                        ImGui::PushItemWidth(50);
                        if (ImGui::DragScalar(("##val" + std::to_string(pinId)).c_str(), ImGuiDataType_Double, val,
                                              0.1f)) {
                            node->isDirty = true;  // Mark dirty on manual change
                        }
                        ImGui::PopItemWidth();
                    }
                }

                ImNodes::EndInputAttribute();
                ImNodes::PopColorStyle();
            }

            // Outputs
            for (size_t i = 0; i < node->outputs.size(); ++i) {
                int pinId = GetPinID(entity, static_cast<int>(i), true);
                ImNodes::PushColorStyle(ImNodesCol_Pin, GetPinColor(node->outputs[i].type));
                ImNodes::BeginOutputAttribute(pinId);

                // Align text to right would be nice here
                ImGui::Text("%s", node->outputs[i].name.c_str());

                ImNodes::EndOutputAttribute();
                ImNodes::PopColorStyle();
            }

            ImNodes::EndNode();

            // Update stored position from ImNodes
            ImVec2 pos = ImNodes::GetNodeGridSpacePos(static_cast<int>(entity));
            node->ui = {pos.x, pos.y};
        }

        // ---------------------------------------------------------
        // 2. DRAW LINKS
        // ---------------------------------------------------------
        for (auto entity : entities) {
            auto* node = m_Registry.GetComponent<of::domain::NodeComponent>(entity);
            for (size_t i = 0; i < node->inputs.size(); ++i) {
                const auto& conn = node->inputs[i].connection;
                if (conn.targetNodeID != of::domain::INVALID_ENTITY_ID) {
                    // Create Link ID based on the Input Pin (since 1 input can have only 1 link)
                    int linkId = GetLinkID(entity, static_cast<int>(i));

                    int startPinId = GetPinID(conn.targetNodeID, static_cast<int>(conn.targetPinIdx), true);
                    int endPinId = GetPinID(entity, static_cast<int>(i), false);

                    ImNodes::Link(linkId, startPinId, endPinId);
                }
            }
        }

        // ---------------------------------------------------------
        // 3. CONTEXT MENU (Spawn Nodes)
        // ---------------------------------------------------------
        // Right click on empty space
        if (ImNodes::IsEditorHovered() && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup("create_node_menu");
            // Capture mouse pos for spawning
            m_CurrentMouseGridPosition = GetCurrentMouseGridPosition();
        }

        if (ImGui::BeginPopup("create_node_menu")) {
            const auto& definitions = NodeRegistry::Instance().GetDefinitions();  // Assuming you add this getter

            // Simple flat list for now. Later: Categories.
            for (const auto& [id, def] : definitions) {
                if (ImGui::MenuItem(def.name.c_str())) {
                    of::domain::Entity newEntity = NodeRegistry::Instance().SpawnNode(m_Registry, id);

                    // Set position
                    ImNodes::SetNodeGridSpacePos(static_cast<int>(newEntity),
                                                 ImVec2(m_CurrentMouseGridPosition.x, m_CurrentMouseGridPosition.y));

                    // Update Component so logic knows it exists (though UI is updated next frame)
                    auto* n = m_Registry.GetComponent<of::domain::NodeComponent>(newEntity);
                    if (n)
                        n->ui = m_CurrentMouseGridPosition;
                }
            }
            ImGui::EndPopup();
        }

        ImNodes::EndNodeEditor();

        // ---------------------------------------------------------
        // 4. HANDLE INTERACTIONS
        // ---------------------------------------------------------

        // Connection Created
        int startId, endId;
        if (ImNodes::IsLinkCreated(&startId, &endId)) {
            // Decode IDs
            // Start is usually Output, End is Input
            auto startPin = DecodePinID(startId);
            auto endPin = DecodePinID(endId);

            // Sanity Check: Ensure start is Output and end is Input
            if (startPin.isOutput && !endPin.isOutput) {
                auto* targetNode = m_Registry.GetComponent<of::domain::NodeComponent>(endPin.nodeID);
                if (targetNode) {
                    // Set Connection
                    targetNode->inputs[endPin.pinIndex].connection.targetNodeID = startPin.nodeID;
                    targetNode->inputs[endPin.pinIndex].connection.targetPinIdx = startPin.pinIndex;

                    // Mark Dirty
                    targetNode->isDirty = true;
                    // Important: The Evaluator needs to know, usually via a global flag or by traversing next frame
                }
            }
        }

        // Connection Deleted
        int destroyedLinkId;
        if (ImNodes::IsLinkDestroyed(&destroyedLinkId)) {
            // Link ID was derived from the Input Pin ID
            auto info = DecodeLinkID(destroyedLinkId);

            auto* node = m_Registry.GetComponent<of::domain::NodeComponent>(info.nodeID);
            if (node) {
                // Reset Connection
                node->inputs[info.pinIndex].connection.targetNodeID = of::domain::INVALID_ENTITY_ID;
                node->inputs[info.pinIndex].connection.targetPinIdx = 0;
                node->isDirty = true;
            }
        }
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
