#include <imgui.h>
#include <imnodes.h>

#include <cctype>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>
#include <variant>

#include "ontoflow/domain/Types.hpp"

using namespace of::domain;
using namespace of::engine;
using namespace of::ui;

namespace of::editor {

/**
 * \brief Constructs an Editor instance.
 * \param ctx A reference to the ToolContext, providing access to core services.
 */
Editor::Editor(domain::Registry& registry, domain::GeometrySystem& geometrySystem)
    : m_Registry(registry), m_GeometrySystem(geometrySystem) {
    // Initialize ImNodes
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    m_Evaluator = std::make_unique<GraphEvaluator>(m_Registry);
    m_GraphEditorSystem = std::make_unique<GraphEditorSystem>(m_Registry);
}

Editor::~Editor() {
    ImNodes::DestroyContext();
}

void Editor::DrawUI() {
    if (m_GraphEditorSystem) {
        m_GraphEditorSystem->DrawPanel();
    }
}

/**
 * \brief Called once per frame (or simulation step) to update active tools and camera.
 * \param dt Time delta since last frame.
 */
void Editor::Update(double dt) {
    // 1. Interaction Loop (Dataflow Updates)
    if (m_NeedsEvaluation) {
        if (m_BoxNodeID != INVALID_ENTITY_ID && m_Evaluator) {
            LOG(Info) << "Editor: Evaluating Graph...";
            m_Evaluator->Evaluate(m_BoxNodeID);
            SyncMeshes();
        }
        m_NeedsEvaluation = false;
    }

    m_ViewController.Update(dt);
}

/**
 * \brief Sets the 2D camera for the ViewController.
 * \param cam A shared pointer to the 2D camera implementation.
 */
void Editor::SetCamera2D(std::shared_ptr<ICamera> cam) {
    m_ViewController.SetCamera2D(cam);
}

/**
 * \brief Sets the 3D camera for the ViewController.
 * \param cam A shared pointer to the 3D camera implementation.
 */
void Editor::SetCamera3D(std::shared_ptr<ICamera> cam) {
    m_ViewController.SetCamera3D(cam);
}

/**
 * \brief Sets the viewport size for the ViewController's cameras.
 * \param w Width of the viewport.
 * \param h Height of the viewport.
 */
void Editor::SetViewportSize(int w, int h) {
    m_ViewController.SetViewportSize(w, h);
}

/**
 * \brief Returns a pointer to the currently active camera.
 * \return A pointer to the ICamera.
 */
ICamera* Editor::GetActiveCamera() {
    return m_ViewController.GetActiveCamera();
}

/**
 * \brief Feeds an input event into the editor for processing.
 * Events are dispatched first to the active tool, then to the camera controller.
 * \param ev The input event.
 */
void Editor::OnInput(const InputEvent& ev) {
    // Global keyboard shortcuts
    if (auto* key = AsKey(ev)) {
        HandleKey(*key);
    }

    m_ViewController.OnInput(ev);
}

/**
 * \brief Handles keyboard events, including global shortcuts and command buffer input.
 * \param key The KeyEvent to process.
 */
void Editor::HandleKey(const KeyEvent& key) {
    if (!key.pressed)
        return;

    float diff = 0.1;

    // Interaction Logic: Modify Graph
    if (key.text == 'k' && m_WidthNodeID != INVALID_ENTITY_ID) {
        auto* node = m_Registry.GetComponent<NodeComponent>(m_WidthNodeID);
        if (node && std::holds_alternative<double>(node->outputs[0].value)) {
            double val = std::get<double>(node->outputs[0].value);
            node->outputs[0].value = val + diff;
            LOG(Info) << "Editor: Width increased to " << val + diff;
            m_NeedsEvaluation = true;
            return;  // Consumed
        }
    }

    if (key.text == 'j' && m_WidthNodeID != INVALID_ENTITY_ID) {
        auto* node = m_Registry.GetComponent<NodeComponent>(m_WidthNodeID);
        if (node && std::holds_alternative<double>(node->outputs[0].value)) {
            double val = std::get<double>(node->outputs[0].value);
            node->outputs[0].value = val - diff;
            LOG(Info) << "Editor: Width decreased to " << val - diff;
            m_NeedsEvaluation = true;
            return;  // Consumed
        }
    }

    // Ignore special keys (like Enter), tools should handle them if needed.
    if (key.text == 0)
        return;

    // Only process simple letter characters (without Ctrl/Alt modifiers)
    if (key.ctrl || key.alt)
        return;

    // Example: 'p' to dump registry (global shortcut)
    if (key.text == 'p') {
        LOG(Info) << "Editor: 'p' pressed, dumping registry...";
        m_Registry.Dump();
    }

    // 'g' to toggle graph editor
    if (key.code == KeyCode::Space && m_GraphEditorSystem) {
        m_GraphEditorSystem->ToggleVisibility();
    }

    char c = static_cast<char>(std::tolower(static_cast<unsigned char>(key.text)));
    m_CommandBuffer.push_back(c);

    // Keep only the last 2 characters in the command buffer (for commands like "ip", "il", "ic", "is")
    if (m_CommandBuffer.size() > 2)
        m_CommandBuffer.erase(0,
                              m_CommandBuffer.size() - 2);  // Erase from beginning, keeping only the last 2 characters
}

void Editor::InitializeDemoGraph() {
    auto& reg = m_Registry;
    auto& nodeReg = NodeRegistry::Instance();

    // Capture IGeometryBackend reference
    IGeometryBackend& backend = m_GeometrySystem.GetBackend();

    // Register Nodes Locally (or rely on main, but better here for context)
    // 1. Value Node
    NodeDefinition valDef;
    valDef.name = "Value (Float)";
    valDef.outputs.push_back(Pin{"Out", PinType::FLOAT, 0.0f, {}});
    valDef.compute = [](NodeComponent&, Registry&) { /* Static */ };
    nodeReg.RegisterNode("VALUE_FLOAT", valDef);

    // 2. Box Node (Smart Reuse)
    NodeDefinition boxDef;
    boxDef.name = "Box";
    boxDef.inputs.push_back(Pin{"Width", PinType::FLOAT, 0.0f, {}});
    boxDef.inputs.push_back(Pin{"Length", PinType::FLOAT, 0.0f, {}});
    boxDef.inputs.push_back(Pin{"Height", PinType::FLOAT, 0.0f, {}});
    boxDef.outputs.push_back(Pin{"Shape", PinType::GEOMETRY, 0.0f, {}});

    boxDef.compute = [&backend](NodeComponent& node, Registry& r) {
        double w = 10.0, l = 10.0, h = 10.0;

        if (std::holds_alternative<double>(node.inputs[0].value))
            w = std::get<double>(node.inputs[0].value);
        if (std::holds_alternative<double>(node.inputs[1].value))
            l = std::get<double>(node.inputs[1].value);
        if (std::holds_alternative<double>(node.inputs[2].value))
            h = std::get<double>(node.inputs[2].value);

        auto handle = backend.CreateBox(w, l, h);

        Entity bodyEnt;
        if (std::holds_alternative<GeometryHandle>(node.outputs[0].value)) {
            // Reuse
            bodyEnt = std::get<GeometryHandle>(node.outputs[0].value).id;
            // Update Handle
            if (r.HasComponent<BodyComponent>(bodyEnt)) {
                r.GetComponent<BodyComponent>(bodyEnt)->handle = handle;
            }
        } else {
            // Create New
            bodyEnt = r.CreateEntity();
            r.AddComponent<BodyComponent>(bodyEnt, BodyComponent{handle});
            r.AddComponent<NameComponent>(bodyEnt, NameComponent{"Box_Body"});
            node.outputs[0].value = GeometryHandle{bodyEnt};
        }
    };
    nodeReg.RegisterNode("GEOM_BOX", boxDef);

    // Spawn Graph
    m_WidthNodeID = nodeReg.SpawnNode(reg, "VALUE_FLOAT");
    Entity nLength = nodeReg.SpawnNode(reg, "VALUE_FLOAT");
    Entity nHeight = nodeReg.SpawnNode(reg, "VALUE_FLOAT");
    m_BoxNodeID = nodeReg.SpawnNode(reg, "GEOM_BOX");

    // Init Values
    reg.GetComponent<NodeComponent>(m_WidthNodeID)->outputs[0].value = 2.0;
    reg.GetComponent<NodeComponent>(nLength)->outputs[0].value = 3.0;
    reg.GetComponent<NodeComponent>(nHeight)->outputs[0].value = 4.0;

    // Connect
    auto* boxNode = reg.GetComponent<NodeComponent>(m_BoxNodeID);
    boxNode->inputs[0].connection = {m_WidthNodeID, 0};
    boxNode->inputs[1].connection = {nLength, 0};
    boxNode->inputs[2].connection = {nHeight, 0};

    // Initial Eval
    m_NeedsEvaluation = true;
}

void Editor::SyncMeshes() {
    auto& reg = m_Registry;
    IGeometryBackend& backend = m_GeometrySystem.GetBackend();

    // Naive Sync: Iterate all entities with BodyComponent
    // In a real system, we would track dirty flags or events.
    // Since we just updated the graph, we know things might have changed.

    auto entities = reg.Entities();  // Inefficient but works for MVP
    for (Entity e : entities) {
        if (reg.HasComponent<BodyComponent>(e)) {
            auto* body = reg.GetComponent<BodyComponent>(e);
            if (body->handle > 0) {
                // Tesselate
                Mesh meshData = backend.GetMeshFromShape(body->handle);
                auto* meshComp = reg.GetComponent<MeshComponent>(e);
                if (meshComp) {
                    meshComp->version++;
                    meshComp->mesh = std::move(meshData);
                } else {
                    reg.AddComponent<MeshComponent>(e, MeshComponent{std::move(meshData)});
                }
            }
        }
    }
}

}  // namespace of::editor
