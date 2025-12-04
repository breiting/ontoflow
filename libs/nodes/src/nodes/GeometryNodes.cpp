#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/GeometryNodes.hpp>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterGeometryNodes(of::domain::IGeometryBackend& backend) {
    auto& reg = NodeRegistry::Instance();

    // --- Box Node ---
    NodeDefinition boxDef;
    boxDef.name = "Box Generator";
    boxDef.category = "Geometry";
    boxDef.inputs = {{"Width", PinType::FLOAT}, {"Length", PinType::FLOAT}, {"Height", PinType::FLOAT}};
    boxDef.outputs = {{"Shape", PinType::GEOMETRY}};

    // Capture backend by reference
    boxDef.compute = [&backend](NodeComponent& node, Registry& r) {
        double w = 10, l = 10, h = 10;

        if (auto* v = std::get_if<double>(&node.inputs[0].value))
            w = *v;
        if (auto* v = std::get_if<double>(&node.inputs[1].value))
            l = *v;
        if (auto* v = std::get_if<double>(&node.inputs[2].value))
            h = *v;

        // 1. Get/Create Body Entity (Reuse logic)
        EntityID bodyEnt = INVALID_ENTITY_ID;

        if (auto* handle = std::get_if<GeometryHandle>(&node.outputs[0].value)) {
            if (handle->IsValid() && r.HasComponent<BodyComponent>(handle->id)) {
                bodyEnt = handle->id;
            }
        }

        if (bodyEnt == INVALID_ENTITY_ID) {
            bodyEnt = r.CreateEntity();
            r.AddComponent(bodyEnt, BodyComponent{});
            r.AddComponent(bodyEnt, NameComponent{"Box_Body"});
            node.outputs[0].value = GeometryHandle{bodyEnt};
        }

        // 2. OCCT Calculation
        auto shapeHandle = backend.CreateBox(w, l, h);

        // 3. Update Body
        r.GetComponent<BodyComponent>(bodyEnt)->handle = shapeHandle;
    };
    reg.RegisterNode("GEOM_BOX", boxDef);
}

}  // namespace of::nodes
