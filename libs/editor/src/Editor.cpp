#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/editor/IViewportRenderer.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/StandardLibrary.hpp>

// Relative path to extern as libs/editor doesn't include it in CMake
#include "../../../../extern/roboto_regular.h"

using namespace of::domain;
using namespace of::engine;
using namespace of::ui;
using namespace of::nodes;

namespace of::editor {

Editor::Editor(domain::Registry& registry, domain::GeometrySystem& geometrySystem, IViewportRenderer* renderer)
    : m_Registry(registry),
      m_GeometrySystem(geometrySystem),
      m_Renderer(renderer),
      m_NodeEditorRegistry(m_UiAllocator) {
    
    // ImNodes Context
    ImNodes::CreateContext();
    
    // Initialize Engine Systems
    m_Evaluator = std::make_unique<GraphEvaluator>(m_Registry);
    
    // Initialize UI Systems
    m_GraphEditorSystem = std::make_unique<GraphEditorSystem>(m_Registry, m_NodeEditorRegistry);

    Initialize();
}

Editor::~Editor() {
    ImNodes::DestroyContext();
}

void Editor::Initialize() {
    // Load Fonts
    // We use AddFontFromMemoryTTF because the header contains raw TTF data, not compressed.
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false; // The data is const static, no need to copy/free
    io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 16.0f, &cfg);
    
    // Note: Theme is applied by GraphEditorSystem constructor
}

void Editor::DrawUI() {
    // We don't create a window here anymore, we let GraphEditorSystem handle the layout
    // into the main viewport.
    
    if (m_GraphEditorSystem) {
        m_GraphEditorSystem->DrawLayout([this](const std::string& cmd) {
            ExecuteCommand(cmd);
        });
    }

    // Render Window (Overlay or separate?)
    // The layout task specified full screen layout. 
    // If we want to see the 3D view, we might need a specific node or toggle.
    // Existing code had m_ShowRenderWindow. Let's keep it optional or integrate it?
    // The task didn't specify where the 3D view goes. 
    // But "GraphEditorSystem" takes the center.
    // Maybe we can make the "Graph" panel switchable to "Viewport".
    // For now, let's keep the 3D view as a separate window if enabled, 
    // but strictly following the layout, it might overlap.
    
    if (m_ShowRenderWindow) {
        if (ImGui::Begin("3D View")) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 0.0f && avail.y > 0.0f) {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton("##3DViewCanvas", avail);

                ImGuiIO& io = ImGui::GetIO();
                const int fbHeight = static_cast<int>(io.DisplaySize.y);
                int x = (int)pos.x;
                int y = fbHeight - (int)pos.y - (int)avail.y;
                int w = (int)avail.x;
                int h = (int)avail.y;

                if (ICamera* cam = GetActiveCamera()) {
                    m_Renderer->Render(cam, {x, y, w, h});
                }
            }
        }
        ImGui::End();
    }
}

void Editor::Update(double dt) {
    if (m_NeedsEvaluation) {
        if (m_SinkNodeID != INVALID_ENTITY) {
            LOG(Info) << "Editor: Evaluating Dataflow Graph...";
            m_Evaluator->Evaluate(m_SinkNodeID);
            SyncMeshes();
        }
        m_NeedsEvaluation = false;
    }

    m_ViewController.Update(dt);
}

void Editor::ExecuteCommand(const std::string& cmd) {
    LOG(Info) << "Command: " << cmd;

    if (cmd == ":box") {
        // Spawn Box
        auto e = NodeRegistry::Instance().SpawnNode(m_Registry, "GEOM_BOX");
        // Place it near center? GraphEditorSystem handles placement via "SpawnPos" logic usually,
        // but here we might just let it be at 0,0 or update position.
        // We can access internal registry to move it?
        // The UI system syncs position from component.
        if (auto* node = m_Registry.GetComponent<NodeComponent>(e)) {
             node->ui = {0.0f, 0.0f}; // Reset to center
        }
    } 
    else if (cmd == ":val") {
        auto e = NodeRegistry::Instance().SpawnNode(m_Registry, "FLOAT_VALUE");
        if (auto* node = m_Registry.GetComponent<NodeComponent>(e)) {
             node->ui = {0.0f, 0.0f};
        }
    }
    else if (cmd == ":evaluate") {
        m_NeedsEvaluation = true;
    }
    else if (cmd == ":clear") {
         // Basic clear?
         // m_Registry.Clear(); // Logic to clear nodes
    }
    else if (cmd == ":view") {
        m_ShowRenderWindow = !m_ShowRenderWindow;
    }
}

void Editor::SetCamera2D(std::shared_ptr<ICamera> cam) {
    m_ViewController.SetCamera2D(cam);
}

void Editor::SetCamera3D(std::shared_ptr<ICamera> cam) {
    m_ViewController.SetCamera3D(cam);
}

void Editor::SetViewportSize(int w, int h) {
    m_ViewController.SetViewportSize(w, h);
}

ICamera* Editor::GetActiveCamera() {
    return m_ViewController.GetActiveCamera();
}

void Editor::OnInput(const InputEvent& ev) {
    if (auto* key = AsKey(ev))
        HandleKey(*key);

    m_ViewController.OnInput(ev);
}

void Editor::HandleKey(const KeyEvent& key) {
    if (!key.pressed)
        return;
        
    // Shortcuts can still work
    if (key.text == ':' && m_GraphEditorSystem) {
        m_GraphEditorSystem->RequestCommandFocus();
    }
}

void Editor::SyncMeshes() {
    auto& backend = m_GeometrySystem.GetBackend();

    for (auto e : m_Registry.Entities()) {
        if (auto* body = m_Registry.GetComponent<BodyComponent>(e)) {
            if (body->handle > 0) {
                Mesh mesh = backend.GetMeshFromShape(body->handle);

                if (auto* mc = m_Registry.GetComponent<MeshComponent>(e)) {
                    mc->mesh = std::move(mesh);
                    mc->version++;
                } else {
                    m_Registry.AddComponent(e, MeshComponent{std::move(mesh), 0});
                }
            }
        }
    }
}

}  // namespace of::editor