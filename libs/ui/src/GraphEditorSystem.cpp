#include <imgui.h>
#include <imgui_internal.h>
#include <imnodes.h>
#include <map>

#include <ontoflow/core/Colors.hpp>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>

namespace of::ui {

using namespace of::core;

GraphEditorSystem::GraphEditorSystem(domain::Registry& reg, NodeEditorRegistry& editorReg)
    : m_Registry(reg), m_EditorReg(editorReg) {
    ApplyTheme();
}

void GraphEditorSystem::ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;

    style.ItemSpacing = ImVec2(10, 8);
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(6, 4);

    // Colors (Nord)
    auto toImVec4 = [](const glm::vec4& v) { return ImVec4(v.r, v.g, v.b, v.a); };

    style.Colors[ImGuiCol_Text] = toImVec4(nord::Nord6);
    style.Colors[ImGuiCol_WindowBg] = toImVec4(nord::Nord0);
    style.Colors[ImGuiCol_ChildBg] = toImVec4(nord::Nord0);
    style.Colors[ImGuiCol_PopupBg] = toImVec4(nord::Nord1);
    style.Colors[ImGuiCol_Border] = toImVec4(nord::Nord3);
    style.Colors[ImGuiCol_FrameBg] = toImVec4(nord::Nord1);
    style.Colors[ImGuiCol_FrameBgHovered] = toImVec4(nord::Nord2);
    style.Colors[ImGuiCol_FrameBgActive] = toImVec4(nord::Nord3);
    style.Colors[ImGuiCol_TitleBg] = toImVec4(nord::Nord1);
    style.Colors[ImGuiCol_TitleBgActive] = toImVec4(nord::Nord2);
    style.Colors[ImGuiCol_Button] = toImVec4(nord::Nord3);
    style.Colors[ImGuiCol_ButtonHovered] = toImVec4(nord::Nord9);  // Frost Blue
    style.Colors[ImGuiCol_ButtonActive] = toImVec4(nord::Nord10);
    style.Colors[ImGuiCol_Header] = toImVec4(nord::Nord3);
    style.Colors[ImGuiCol_HeaderHovered] = toImVec4(nord::Nord9);
    style.Colors[ImGuiCol_HeaderActive] = toImVec4(nord::Nord10);
    style.Colors[ImGuiCol_Separator] = toImVec4(nord::Nord3);
    style.Colors[ImGuiCol_ResizeGrip] = toImVec4(nord::Nord3);
    style.Colors[ImGuiCol_ResizeGripHovered] = toImVec4(nord::Nord9);
    style.Colors[ImGuiCol_ResizeGripActive] = toImVec4(nord::Nord10);
    style.Colors[ImGuiCol_PlotLines] = toImVec4(nord::Nord14);
    style.Colors[ImGuiCol_PlotLinesHovered] = toImVec4(nord::Nord14);

    // ImNodes Style
    ImNodes::StyleColorsDark();
    ImNodesStyle& nStyle = ImNodes::GetStyle();
    nStyle.Colors[ImNodesCol_GridBackground] = IM_COL32(46, 52, 64, 255); // Nord0
    nStyle.Colors[ImNodesCol_GridLine] = IM_COL32(59, 66, 82, 255);       // Nord1
    nStyle.Colors[ImNodesCol_NodeBackground] = IM_COL32(59, 66, 82, 255); // Nord1
    nStyle.Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(67, 76, 94, 255); // Nord2
    nStyle.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(76, 86, 106, 255); // Nord3
    nStyle.Colors[ImNodesCol_TitleBar] = IM_COL32(59, 66, 82, 255);
    nStyle.Colors[ImNodesCol_TitleBarSelected] = IM_COL32(76, 86, 106, 255);
    nStyle.Colors[ImNodesCol_Link] = IM_COL32(216, 222, 233, 255); // Nord4
    nStyle.Colors[ImNodesCol_LinkSelected] = IM_COL32(248, 179, 182, 255); // Ghibli Flower (Pink)
    nStyle.Colors[ImNodesCol_Pin] = IM_COL32(136, 192, 208, 255); // Nord8
    nStyle.Colors[ImNodesCol_PinHovered] = IM_COL32(129, 161, 193, 255); // Nord9

    // Make selected nodes pop with a border/outline change if supported, 
    // or just via TitleBar/BackgroundSelected.
    // Let's use the Ghibli Flower color for selection highlights to make it distinct.
    nStyle.Colors[ImNodesCol_TitleBarSelected] = IM_COL32(248, 179, 182, 255); // Ghibli Flower
    nStyle.Colors[ImNodesCol_NodeOutline] = IM_COL32(46, 52, 64, 255); // Nord0
    
    nStyle.NodeCornerRounding = 5.0f;
    nStyle.NodePadding = ImVec2(12, 8);
    nStyle.PinCircleRadius = 4.0f;
}

glm::vec2 GraphEditorSystem::GetMouseGridPos() const {
    ImVec2 mouse = ImGui::GetMousePos();
    ImVec2 pan = ImNodes::EditorContextGetPanning();
    ImVec2 local = ImVec2(mouse.x - pan.x, mouse.y - pan.y);
    return {local.x, local.y};
}

EditorAction GraphEditorSystem::DrawLayout(StatusBar& statusBar) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    // Fixed dimensions
    const float toolbarWidth = 64.0f;
    const float libraryWidth = 250.0f;
    const float bottomHeight = 40.0f; // Increased height
    const float centerWidth = workSize.x - toolbarWidth - libraryWidth;
    const float centerHeight = workSize.y - bottomHeight;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;

    EditorAction action = EditorAction::None;

    // 1. Left Toolbar
    ImGui::SetNextWindowPos({workPos.x, workPos.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({toolbarWidth, centerHeight}, ImGuiCond_Always);
    if (ImGui::Begin("Toolbar", nullptr, windowFlags)) {
        action = DrawToolbar();
    }
    ImGui::End();

    // 2. Right Library
    ImGui::SetNextWindowPos({workPos.x + workSize.x - libraryWidth, workPos.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({libraryWidth, centerHeight}, ImGuiCond_Always);
    if (ImGui::Begin("Library", nullptr, windowFlags)) {
        DrawNodeLibrary();
    }
    ImGui::End();

    // 3. Center Graph (Drawn before Status Bar to act as background)
    ImGui::SetNextWindowPos({workPos.x + toolbarWidth, workPos.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({centerWidth, centerHeight}, ImGuiCond_Always);
    if (ImGui::Begin("GraphRegion", nullptr, windowFlags)) {
        DrawNodeEditorInternal();
    }
    ImGui::End();

    // 4. Bottom Status (Drawn last to stay on top)
    ImGui::SetNextWindowPos({workPos.x, workPos.y + centerHeight}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({workSize.x, bottomHeight}, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(46, 52, 64, 255)); // Nord0
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f)); // Adjusted padding
    if (ImGui::Begin("StatusBar", nullptr, windowFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        statusBar.Draw(ImGui::GetIO().DeltaTime);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    return action;
}

EditorAction GraphEditorSystem::DrawToolbar() {
    ImVec2 btnSize(40, 40);
    float availX = ImGui::GetContentRegionAvail().x;
    float offsetX = (availX - btnSize.x) * 0.5f;

    auto toolBtn = [&](const char* label, EditorAction act) -> bool {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        bool clicked = ImGui::Button(label, btnSize);
        ImGui::Dummy(ImVec2(0, 10));
        return clicked;
    };

    EditorAction result = EditorAction::None;

    ImGui::Dummy(ImVec2(0, 10));
    if (toolBtn("[+]", EditorAction::None)) { /* New */ }
    if (toolBtn("[S]", EditorAction::Save)) result = EditorAction::Save;
    if (toolBtn("[L]", EditorAction::Load)) result = EditorAction::Load;
    if (toolBtn("[C]", EditorAction::Clear)) result = EditorAction::Clear;

    // Spacer
    ImGui::Dummy(ImVec2(0, 20));
    ImGui::TextDisabled("|");
    ImGui::Dummy(ImVec2(0, 20));

    if (toolBtn("[E]", EditorAction::Evaluate)) result = EditorAction::Evaluate;
    if (toolBtn("[D]", EditorAction::Dump)) result = EditorAction::Dump;
    
    return result;
}

void GraphEditorSystem::DrawNodeLibrary() {
    ImGui::TextDisabled("NODE LIBRARY");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    m_NodeFilter.Draw("##Search", ImGui::GetContentRegionAvail().x);
    ImGui::Dummy(ImVec2(0, 10));

    const auto& defs = engine::NodeRegistry::Instance().GetDefinitions();
    
    std::map<std::string, std::vector<const engine::NodeDefinition*>> categorized;
    for(const auto& [id, def] : defs) {
        if (m_NodeFilter.PassFilter(def.name.c_str())) {
            categorized[def.category.empty() ? "General" : def.category].push_back(&def);
        }
    }

    for (const auto& [cat, nodes] : categorized) {
        if (ImGui::CollapsingHeader(cat.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const auto* def : nodes) {
                ImGui::PushID(def);
                ImGui::Button(def->name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    std::string opID;
                    for(const auto& [k, v] : defs) {
                        if (&v == def) { opID = k; break; }
                    }

                    ImGui::SetDragDropPayload("DND_NODE_DEF", opID.c_str(), opID.size() + 1);
                    ImGui::Text("Spawn %s", def->name.c_str());
                    ImGui::EndDragDropSource();
                }
                ImGui::PopID();
            }
        }
    }
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

    ImNodes::EndNodeEditor();

    // ----------------- DRAG & DROP TARGET --------------------
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_NODE_DEF")) {
            std::string opID = (const char*)payload->Data;
            glm::vec2 mPos = GetMouseGridPos();
            auto newNode = engine::NodeRegistry::Instance().SpawnNode(m_Registry, opID);
            ImNodes::SetNodeGridSpacePos(m_EditorReg.GetNodeId(newNode), ImVec2(mPos.x, mPos.y));
        }
        ImGui::EndDragDropTarget();
    }

    // ----------------- LINK CREATION ----------------
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
    
    // ----------------- LINK DELETION ----------------
    // 1. Dropped in void
    int destroyedLink;
    if (ImNodes::IsLinkDestroyed(&destroyedLink)) {
        auto [node, pin] = m_EditorReg.DecodeLink(destroyedLink);
        if (auto* comp = m_Registry.GetComponent<domain::NodeComponent>(node)) {
            comp->inputs[pin].connection = {};
            comp->isDirty = true;
            graphChanged = true;
        }
    }
    
    // 2. Explicit Deletion via DELETE key (LINKS)
    const int numSelectedLinks = ImNodes::NumSelectedLinks();
    if (numSelectedLinks > 0 && ImGui::IsKeyReleased(ImGuiKey_Delete)) {
        std::vector<int> selectedLinks(numSelectedLinks);
        ImNodes::GetSelectedLinks(selectedLinks.data());
        
        for (int linkId : selectedLinks) {
            auto [node, pin] = m_EditorReg.DecodeLink(linkId);
            if (auto* comp = m_Registry.GetComponent<domain::NodeComponent>(node)) {
                comp->inputs[pin].connection = {};
                comp->isDirty = true;
                graphChanged = true;
            }
        }
    }

    // 3. Explicit Deletion via DELETE key (NODES)
    const int numSelectedNodes = ImNodes::NumSelectedNodes();
    if (numSelectedNodes > 0 && ImGui::IsKeyReleased(ImGuiKey_Delete)) {
        std::vector<int> selectedNodes(numSelectedNodes);
        ImNodes::GetSelectedNodes(selectedNodes.data());

        // 1. Collect all nodes to check for connections *to* the deleted nodes
        auto allNodes = m_Registry.GetEntitiesWith<domain::NodeComponent>();

        for (int nodeId : selectedNodes) {
            // Decode the entity ID from the ImNodes ID
            // (Assuming 1:1 mapping or stored in user pointer, but here we rely on NodeEditorRegistry reverse lookup?
            //  Actually NodeEditorRegistry::GetNodeId gives us Entity -> int. We need int -> Entity.
            //  But wait! We don't have a reverse map in NodeEditorRegistry public API easily.
            //  Let's check if we can get it. If not, we might need to iterate all nodes to find the match,
            //  OR we can iterate 'allNodes' and check if GetNodeId(e) is in selectedNodes.)
            
            // Optimization: Instead of full reverse lookup, let's iterate all entities once and check against selection.
        }

        // Better approach:
        std::vector<domain::Entity> nodesToDelete;
        for (auto e : allNodes) {
            int id = m_EditorReg.GetNodeId(e);
            // Check if 'id' is in 'selectedNodes'
            for (int sel : selectedNodes) {
                if (sel == id) {
                    nodesToDelete.push_back(e);
                    break;
                }
            }
        }

        for (auto eToDelete : nodesToDelete) {
            // A. Remove connections TO this node (inputs of OTHER nodes pointing here)
            //    Actually, our graph stores connections in the INPUTS.
            //    So we need to check every node's inputs to see if they point to 'eToDelete'.
            for (auto otherE : allNodes) {
                if (otherE == eToDelete) continue; // Skip self (will be destroyed anyway)
                auto* otherNode = m_Registry.GetComponent<domain::NodeComponent>(otherE);
                if (!otherNode) continue;

                for (auto& pin : otherNode->inputs) {
                    if (pin.connection.targetNodeID == eToDelete) {
                        pin.connection = {}; // Sever connection
                        otherNode->isDirty = true;
                    }
                }
            }

            // B. Destroy the entity
            m_Registry.DestroyEntity(eToDelete);
            graphChanged = true;
        }
    }

    return graphChanged;
}

// Custom Toggle Switch Helper
bool ToggleSwitch(const char* label, bool* v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(label, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(label)) { // Simple animation state logic
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = ImGui::GetColorU32(*v ? ImVec4(0.56f, 0.84f, 0.90f, 1.0f) : ImVec4(0.35f, 0.39f, 0.48f, 1.0f)); // Nord9 : Nord3
    else
        col_bg = ImGui::GetColorU32(*v ? ImVec4(0.53f, 0.75f, 0.82f, 1.0f) : ImVec4(0.29f, 0.34f, 0.42f, 1.0f)); // Nord8 : Nord2

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
    
    // Label
    ImGui::SameLine();
    ImGui::Text("%s", ""); // Use empty label to align, render label manually if needed or let caller handle

    return *v; 
}

bool GraphEditorSystem::DrawSingleNode(domain::Entity e, domain::NodeComponent& node, domain::NameComponent* nameComp) {
    bool changed = false;

    int uiNode = m_EditorReg.GetNodeId(e);

    const auto* def = engine::NodeRegistry::Instance().GetDefinition(node.definitionID);
    
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
    ImGui::Text("%s", displayName.c_str());
    ImNodes::EndNodeTitleBar();

    // Force a fixed width for the node
    const float NODE_WIDTH = 200.0f;
    ImGui::Dummy(ImVec2(NODE_WIDTH, 0.0f));

    bool isValueNode = node.inputs.empty() && node.outputs.size() == 1;

    if (isValueNode) {
        int outPin = m_EditorReg.GetPinId(e, 0, true);
        ImNodes::BeginOutputAttribute(outPin);

        ImGui::Text("%s", node.outputs[0].name.c_str());
        ImGui::SameLine();

        auto& pin = node.outputs[0];
        ImGui::PushItemWidth(100.0f);

        switch (pin.type) {
            case domain::PinType::DOUBLE: {
                double v = 0.0;
                if (auto* pv = std::get_if<double>(&pin.value))
                    v = *pv;
                if (ImGui::InputDouble("##val", &v, 0.0, 0.0, "%.3f")) {
                    pin.value = v;
                    node.isDirty = true;
                    changed = true;
                }
                break;
            }
            case domain::PinType::INT: {
                int v = 0;
                if (auto* pv = std::get_if<int>(&pin.value))
                    v = *pv;
                if (ImGui::InputInt("##val", &v)) {
                    pin.value = v;
                    node.isDirty = true;
                    changed = true;
                }
                break;
            }
            case domain::PinType::BOOL: {
                bool v = false;
                if (auto* pv = std::get_if<bool>(&pin.value))
                    v = *pv;
                
                // Use Custom Toggle Switch
                bool oldV = v;
                ToggleSwitch("##val", &v); 
                if (v != oldV) {
                    pin.value = v;
                    node.isDirty = true;
                    changed = true;
                }
                break;
            }
            case domain::PinType::STRING: {
                std::string v = "";
                if (auto* pv = std::get_if<std::string>(&pin.value))
                    v = *pv;
                char buffer[256];
                strncpy(buffer, v.c_str(), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = 0;
                if (ImGui::InputText("##val", buffer, sizeof(buffer))) {
                    pin.value = std::string(buffer);
                    node.isDirty = true;
                    changed = true;
                }
                break;
            }
            case domain::PinType::VEC3: {
                glm::vec3 v(0.0f);
                if (auto* pv = std::get_if<glm::vec3>(&pin.value))
                    v = *pv;
                float fv[3] = {v.x, v.y, v.z};
                if (ImGui::InputFloat3("##val", fv, "%.3f")) {
                    pin.value = glm::vec3(fv[0], fv[1], fv[2]);
                    node.isDirty = true;
                    changed = true;
                }
                break;
            }
            default:
                ImGui::Text("???");
                break;
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
        // OUTPUT PINS (Right Aligned)
        // ---------------------------------------------------------
        for (size_t i = 0; i < node.outputs.size(); ++i) {
            auto& pin = node.outputs[i];
            int pinId = m_EditorReg.GetPinId(e, i, true);

            ImNodes::BeginOutputAttribute(pinId);
            
            // Use fixed width for alignment to avoid expansion loop
            float textWidth = ImGui::CalcTextSize(pin.name.c_str()).x;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            
            // Calculate offset based on the fixed width
            float offsetX = NODE_WIDTH - textWidth - spacing;
            
            if (offsetX > 0) {
                 ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
            }
            ImGui::Text("%s", pin.name.c_str());
            
            ImNodes::EndOutputAttribute();
        }
    }

    ImNodes::EndNode();

    ImVec2 pos = ImNodes::GetNodeGridSpacePos(uiNode);
    node.ui = {pos.x, pos.y};

    return changed;
}

void GraphEditorSystem::DrawThinSeparator(float thickness) {
    ImVec2 min = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine({min.x, min.y}, {min.x + 80.0f, min.y}, IM_COL32(150, 150, 150, 100),
                                        thickness);
    ImGui::Dummy({80.0f, thickness + 2.0f});
}

void GraphEditorSystem::DumpNodePositions() const {
    LOG(Info) << "---- Node Positions ----";
    auto entities = m_Registry.GetEntitiesWith<domain::NodeComponent>();
    for (auto e : entities) {
        const auto* node = m_Registry.GetComponent<domain::NodeComponent>(e);
        LOG(Info) << "Node ID=" << e << " Pos=(" << node->ui.x << ", " << node->ui.y << ")";
    }
    LOG(Info) << "------------------------";
}

}  // namespace of::ui
