#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/editor/IViewportRenderer.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/StandardLibrary.hpp>

using namespace of::domain;
using namespace of::engine;
using namespace of::ui;
using namespace of::nodes;

namespace of::editor {

/**
 * @brief Constructs the Editor. Initializes ImNodes and sets up graph systems.
 */
Editor::Editor(domain::Registry& registry, domain::GeometrySystem& geometrySystem, IViewportRenderer* renderer)
    : m_Registry(registry),
      m_GeometrySystem(geometrySystem),
      m_Renderer(renderer),
      m_NodeEditorRegistry(m_UiAllocator) {
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    m_Evaluator = std::make_unique<GraphEvaluator>(m_Registry);

    // graph editor now requires registry + editor registry
    m_GraphEditorSystem = std::make_unique<GraphEditorSystem>(m_Registry, m_NodeEditorRegistry);
}

Editor::~Editor() {
    ImNodes::DestroyContext();
}

void Editor::DrawUI() {
    // Fullscreen host window for main content
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("MainViewport", nullptr, flags)) {
        ImGui::PopStyleVar(2);

        bool changed = false;
        if (m_GraphEditorSystem) {
            // Draw node editor as embedded content
            changed |= m_GraphEditorSystem->DrawEmbedded();
        }

        if (changed)
            m_NeedsEvaluation = true;
    } else {
        ImGui::PopStyleVar(2);
    }

    ImGui::End();

    if (m_ShowRenderWindow) {
        if (ImGui::Begin("3D View")) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 0.0f && avail.y > 0.0f) {
                ImVec2 pos = ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton("##3DViewCanvas", avail);

                // Get FrameBuffer size
                ImGuiIO& io = ImGui::GetIO();
                const int fbHeight = static_cast<int>(io.DisplaySize.y);

                int x = (int)pos.x;
                int y = fbHeight - (int)pos.y - (int)avail.y;  // Y-flip
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

/**
 * @brief Called every frame to update cameras and process evaluation.
 */
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

/**
 * @brief Dispatch input to tools + cameras.
 */
void Editor::OnInput(const InputEvent& ev) {
    if (auto* key = AsKey(ev))
        HandleKey(*key);

    m_ViewController.OnInput(ev);
}

/**
 * @brief Handle keyboard shortcuts.
 */
void Editor::HandleKey(const KeyEvent& key) {
    if (!key.pressed)
        return;

    float diff = 0.1;

    // Debug: adjust width value
    if (key.text == 'k' && m_WidthNodeID != INVALID_ENTITY) {
        if (auto* node = m_Registry.GetComponent<NodeComponent>(m_WidthNodeID)) {
            if (auto* val = std::get_if<double>(&node->outputs[0].value)) {
                *val += diff;
                node->isDirty = true;
                m_NeedsEvaluation = true;
                LOG(Info) << "Width increased.";
                return;
            }
        }
    }

    if (key.text == 'j' && m_WidthNodeID != INVALID_ENTITY) {
        if (auto* node = m_Registry.GetComponent<NodeComponent>(m_WidthNodeID)) {
            if (auto* val = std::get_if<double>(&node->outputs[0].value)) {
                *val -= diff;
                node->isDirty = true;
                m_NeedsEvaluation = true;
                LOG(Info) << "Width decreased.";
                return;
            }
        }
    }

    // 'p' → dump ECS registry
    if (key.text == 'p') {
        LOG(Info) << "Dumping registry...";
        m_Registry.Dump();
    }
}

/**
 * @brief Build a simple demonstration graph (Value → Box).
 */
void Editor::InitializeDemoGraph() {
    auto& registry = m_Registry;
    auto& backend = m_GeometrySystem.GetBackend();

    GraphEvaluator evaluator(registry);

    StandardLibrary::RegisterAll(backend);

    // 1. Spawn Nodes
    Entity width = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE", "Width");
    Entity length = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE", "Height");
    Entity height = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE", "Length");
    Entity box = NodeRegistry::Instance().SpawnNode(registry, "GEOM_BOX", "Box 1");
    Entity deflection = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE", "Deflection");
    Entity filename = NodeRegistry::Instance().SpawnNode(registry, "STRING_VALUE", "FileName");
    Entity exportStl = NodeRegistry::Instance().SpawnNode(registry, "SINK_SAVE_STL", "ExportSTL", {200, 200});

    // 2. Set Values
    registry.GetComponent<NodeComponent>(width)->outputs[0].value = 5.0;
    registry.GetComponent<NodeComponent>(length)->outputs[0].value = 3.0;
    registry.GetComponent<NodeComponent>(height)->outputs[0].value = 2.0;

    // 3. Connect
    auto* boxNode = registry.GetComponent<NodeComponent>(box);
    boxNode->inputs[0].connection = {width, 0};
    boxNode->inputs[1].connection = {length, 0};
    boxNode->inputs[2].connection = {height, 0};

    auto* exportNode = registry.GetComponent<NodeComponent>(exportStl);
    exportNode->inputs[0].connection = {filename, 0};
    exportNode->inputs[1].connection = {deflection, 0};
    exportNode->inputs[2].connection = {box, 0};

    // 4. Evaluate
    LOG(Info) << "Evaluating Box Node...";
    evaluator.Evaluate(exportStl);

    m_SinkNodeID = exportStl;
    m_NeedsEvaluation = true;
}

/**
 * @brief Convert all OCCT BodyComponents into MeshComponents.
 */
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
