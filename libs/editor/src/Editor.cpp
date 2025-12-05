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
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 16.0f, &cfg);
}

void Editor::DrawUI() {
    if (m_GraphEditorSystem) {
        EditorAction action = m_GraphEditorSystem->DrawLayout(m_StatusBar);
        if (action != EditorAction::None) {
            HandleAction(action);
        }
    }

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

void Editor::HandleAction(EditorAction action) {
    switch (action) {
        case EditorAction::Evaluate:
            ExecuteCommand(":evaluate");
            break;
        case EditorAction::Clear:
             // TODO: Implement Clear
             m_StatusBar.ShowMessage("Clear not implemented");
             break;
        case EditorAction::Dump:
             m_Registry.Dump();
             m_StatusBar.ShowMessage("Registry dumped to log");
             break;
        default:
             break;
    }
}

void Editor::Update(double dt) {
    if (m_NeedsEvaluation) {
        if (m_SinkNodeID != INVALID_ENTITY) {
            LOG(Info) << "Editor: Evaluating Dataflow Graph...";
            m_Evaluator->Evaluate(m_SinkNodeID);
            SyncMeshes();
            m_StatusBar.ShowMessage("Graph Evaluated");
        }
        m_NeedsEvaluation = false;
    }

    m_ViewController.Update(dt);
}

void Editor::ExecuteCommand(const std::string& cmd) {
    LOG(Info) << "Command: " << cmd;

    if (cmd == ":box") {
        auto e = NodeRegistry::Instance().SpawnNode(m_Registry, "GEOM_BOX");
        if (auto* node = m_Registry.GetComponent<NodeComponent>(e)) {
             node->ui = {0.0f, 0.0f};
        }
        m_StatusBar.ShowMessage("Spawned Box");
    } 
    else if (cmd == ":val") {
        auto e = NodeRegistry::Instance().SpawnNode(m_Registry, "FLOAT_VALUE");
        if (auto* node = m_Registry.GetComponent<NodeComponent>(e)) {
             node->ui = {0.0f, 0.0f};
        }
         m_StatusBar.ShowMessage("Spawned Value");
    }
    else if (cmd == ":evaluate") {
        m_NeedsEvaluation = true;
         m_StatusBar.ShowMessage("Evaluation Queued...");
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
    
    // Shortcuts
    if (key.text == 'e') {
        m_NeedsEvaluation = true;
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
