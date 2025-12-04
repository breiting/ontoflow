#include <imgui.h>
#include <imnodes.h>

#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>

using namespace of::domain;
using namespace of::engine;
using namespace of::ui;

namespace of::editor {

/**
 * @brief Constructs the Editor. Initializes ImNodes and sets up graph systems.
 */
Editor::Editor(domain::Registry& registry, domain::GeometrySystem& geometrySystem)
    : m_Registry(registry), m_GeometrySystem(geometrySystem), m_NodeEditorRegistry(m_UiAllocator) {
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    m_Evaluator = std::make_unique<GraphEvaluator>(m_Registry);

    // graph editor now requires registry + editor registry
    m_GraphEditorSystem = std::make_unique<GraphEditorSystem>(m_Registry, m_NodeEditorRegistry);
}

Editor::~Editor() {
    ImNodes::DestroyContext();
}

/**
 * @brief Draws UI windows (graph editor + other panels).
 */
void Editor::DrawUI() {
    if (m_GraphEditorSystem && m_GraphEditorSystem->DrawPanel()) {
        m_NeedsEvaluation = true;
    }
}

/**
 * @brief Called every frame to update cameras and process evaluation.
 */
void Editor::Update(double dt) {
    if (m_NeedsEvaluation) {
        if (m_BoxNodeID != INVALID_ENTITY) {
            LOG(Info) << "Editor: Evaluating Dataflow Graph...";
            m_Evaluator->Evaluate(m_BoxNodeID);
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

    // Space toggles visibility of node editor
    if (key.code == KeyCode::Space && m_GraphEditorSystem) {
        m_GraphEditorSystem->ToggleVisibility();
    }
}

/**
 * @brief Build a simple demonstration graph (Value → Box).
 */
void Editor::InitializeDemoGraph() {
    auto& reg = m_Registry;
    auto& nodeReg = NodeRegistry::Instance();
    auto& backend = m_GeometrySystem.GetBackend();

    // -------------------------
    // Register VALUE node
    // -------------------------
    NodeDefinition valDef;
    valDef.name = "Value (Float)";
    valDef.outputs.push_back(Pin{"Out", PinType::FLOAT, 0.0, {}});
    valDef.compute = [](NodeComponent&, Registry&) {
    };
    nodeReg.RegisterNode("VALUE_FLOAT", valDef);

    // -------------------------
    // Register BOX node
    // -------------------------
    NodeDefinition boxDef;
    boxDef.name = "Box";
    boxDef.inputs.emplace_back("Width", PinType::FLOAT, 1.0, Connection{});
    boxDef.inputs.emplace_back("Length", PinType::FLOAT, 1.0, Connection{});
    boxDef.inputs.emplace_back("Height", PinType::FLOAT, 1.0, Connection{});
    boxDef.outputs.emplace_back("Shape", PinType::GEOMETRY, PinValue{}, Connection{});

    boxDef.compute = [&backend](NodeComponent& node, Registry& r) {
        double w = std::get<double>(node.inputs[0].value);
        double l = std::get<double>(node.inputs[1].value);
        double h = std::get<double>(node.inputs[2].value);

        auto shape = backend.CreateBox(w, l, h);

        // GeometryHandle output reuse
        if (auto* gh = std::get_if<GeometryHandle>(&node.outputs[0].value)) {
            if (gh->IsValid()) {
                auto ent = gh->id;
                if (auto* comp = r.GetComponent<BodyComponent>(ent))
                    comp->handle = shape;
                return;
            }
        }

        // Create new body
        Entity newBody = r.CreateEntity();
        r.AddComponent(newBody, BodyComponent{shape});
        r.AddComponent(newBody, NameComponent{"Box_Body"});

        node.outputs[0].value = GeometryHandle{newBody};
    };

    nodeReg.RegisterNode("GEOM_BOX", boxDef);

    // -------------------------
    // Create the graph
    // -------------------------
    m_WidthNodeID = nodeReg.SpawnNode(reg, "VALUE_FLOAT");
    auto nLength = nodeReg.SpawnNode(reg, "VALUE_FLOAT");
    auto nHeight = nodeReg.SpawnNode(reg, "VALUE_FLOAT");

    m_BoxNodeID = nodeReg.SpawnNode(reg, "GEOM_BOX");

    // Assign values
    reg.GetComponent<NodeComponent>(m_WidthNodeID)->outputs[0].value = 2.0;
    reg.GetComponent<NodeComponent>(nLength)->outputs[0].value = 3.0;
    reg.GetComponent<NodeComponent>(nHeight)->outputs[0].value = 4.0;

    // Connect
    auto* boxNode = reg.GetComponent<NodeComponent>(m_BoxNodeID);
    boxNode->inputs[0].connection = {m_WidthNodeID, 0};
    boxNode->inputs[1].connection = {nLength, 0};
    boxNode->inputs[2].connection = {nHeight, 0};

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
