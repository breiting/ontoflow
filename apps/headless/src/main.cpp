#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/engine/GraphEvaluator.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>
#include <variant>

using namespace of::domain;
using namespace of::engine;
using namespace of::occt;

// --- Node Definitions (In a real app, these would be in a library) ---
void RegisterNodes(OCCTBackend& backend) {
    auto& reg = NodeRegistry::Instance();

    // 1. Value Node (Float)
    NodeDefinition valDef;
    valDef.name = "Value (Float)";
    valDef.outputs.push_back(Pin{"Out", PinType::FLOAT});
    valDef.compute = [](NodeComponent&, Registry&) { /* Static */ };
    reg.RegisterNode("VALUE_FLOAT", valDef);

    // 2. Box Node (Geometry)
    NodeDefinition boxDef;
    boxDef.name = "Box";
    boxDef.inputs.push_back(Pin{"Width", PinType::FLOAT});
    boxDef.inputs.push_back(Pin{"Length", PinType::FLOAT});
    boxDef.inputs.push_back(Pin{"Height", PinType::FLOAT});
    boxDef.outputs.push_back(Pin{"Shape", PinType::GEOMETRY});

    // Capture backend by reference. Ensure backend outlives the registry/lambda usage.
    boxDef.compute = [&backend](NodeComponent& node, Registry& r) {
        double w = 10.0, l = 10.0, h = 10.0;

        // Safely get inputs or defaults
        if (std::holds_alternative<double>(node.inputs[0].value))
            w = std::get<double>(node.inputs[0].value);
        if (std::holds_alternative<double>(node.inputs[1].value))
            l = std::get<double>(node.inputs[1].value);
        if (std::holds_alternative<double>(node.inputs[2].value))
            h = std::get<double>(node.inputs[2].value);

        // Create Geometry using the shared backend
        auto handle = backend.CreateBox(w, l, h);

        // Store in BodyComponent of a NEW entity
        Entity bodyEnt = r.CreateEntity();
        r.AddComponent<BodyComponent>(bodyEnt, BodyComponent{handle});
        r.AddComponent<NameComponent>(bodyEnt, NameComponent{"Box_Body"});

        node.outputs[0].value = GeometryHandle{bodyEnt};
        LOG(Info) << "Box Node Computed. Created Body Entity: " << bodyEnt << " with Handle: " << handle;
    };
    reg.RegisterNode("GEO_BOX", boxDef);
}

int main() {
    LOG(Info) << "================================";
    LOG(Info) << "OntoFlow headless (Dataflow)";
    LOG(Info) << "================================";

    Registry registry;
    GraphEvaluator evaluator(registry);

    OCCTBackend backend;

    RegisterNodes(backend);

    // 1. Spawn Nodes
    Entity nWidth = NodeRegistry::Instance().SpawnNode(registry, "VALUE_FLOAT");
    Entity nLength = NodeRegistry::Instance().SpawnNode(registry, "VALUE_FLOAT");
    Entity nHeight = NodeRegistry::Instance().SpawnNode(registry, "VALUE_FLOAT");
    Entity nBox = NodeRegistry::Instance().SpawnNode(registry, "GEO_BOX");

    // 2. Set Values
    registry.GetComponent<NodeComponent>(nWidth)->outputs[0].value = 50.0;
    registry.GetComponent<NodeComponent>(nLength)->outputs[0].value = 30.0;
    registry.GetComponent<NodeComponent>(nHeight)->outputs[0].value = 20.0;

    // 3. Connect
    auto* boxNode = registry.GetComponent<NodeComponent>(nBox);
    boxNode->inputs[0].connection = {nWidth, 0};
    boxNode->inputs[1].connection = {nLength, 0};
    boxNode->inputs[2].connection = {nHeight, 0};

    // 4. Evaluate
    LOG(Info) << "Evaluating Box Node...";
    evaluator.Evaluate(nBox);

    // 5. Verify
    if (std::holds_alternative<GeometryHandle>(boxNode->outputs[0].value)) {
        auto handle = std::get<GeometryHandle>(boxNode->outputs[0].value);
        LOG(Info) << "Success! Box Node produced Geometry Entity ID: " << handle.id;

        // Verify the created entity has a BodyComponent
        if (registry.HasComponent<BodyComponent>(handle.id)) {
            LOG(Info) << "BodyComponent found on result entity.";

            // Try to export for proof using the SAME backend
            auto* body = registry.GetComponent<BodyComponent>(handle.id);
            if (backend.ExportShapeToSTEP(body->handle, "headless_box.step")) {
                LOG(Info) << "Exported headless_box.step";
            }
        } else {
            LOG(Error) << "BodyComponent missing on result entity!";
        }
    } else {
        LOG(Error) << "Box Node did not produce geometry.";
    }

    return 0;
}
