#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/engine/GraphEvaluator.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/StandardLibrary.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>
#include <variant>

using namespace of::domain;
using namespace of::engine;
using namespace of::occt;

int main() {
    LOG(Info) << "================================";
    LOG(Info) << "OntoFlow headless (Dataflow)";
    LOG(Info) << "================================";

    Registry registry;
    GraphEvaluator evaluator(registry);

    OCCTBackend backend;

    of::nodes::StandardLibrary::RegisterAll(backend);

    // 1. Spawn Nodes
    Entity nWidth = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE");
    Entity nLength = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE");
    Entity nHeight = NodeRegistry::Instance().SpawnNode(registry, "FLOAT_VALUE");
    Entity nBox = NodeRegistry::Instance().SpawnNode(registry, "GEOM_BOX");

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
