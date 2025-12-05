#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/GeometryNodes.hpp>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void ExportStl(of::domain::IGeometryBackend& backend) {
    auto& reg = NodeRegistry::Instance();

    NodeDefinition def;
    def.name = "Export STL";
    def.category = "Geometry";
    def.inputs = {                                       //
                  {"Filename", PinType::STRING},         //
                  {"Deflection", PinType::DOUBLE, 0.2},  //
                  {"Shape", PinType::GEOMETRY}};

    def.outputs = {};

    def.compute = [&backend](NodeComponent& node, Registry& r) {
        const Pin& namePin = node.inputs[0];
        const Pin& deflectionPin = node.inputs[1];
        const Pin& geoPin = node.inputs[2];

        GeometryHandle gh;
        if (auto ghPtr = std::get_if<GeometryHandle>(&geoPin.value)) {
            gh = *ghPtr;
        } else {
            LOG(Error) << "SaveSTL: Geometry input not a GeometryHandle.";
            return;
        }

        if (!gh.IsValid()) {
            LOG(Error) << "SaveSTL: Invalid geometry handle.";
            return;
        }

        // Get BackendShapeHandle
        auto* body = r.GetComponent<BodyComponent>(gh.id);
        if (!body || body->handle == 0) {
            LOG(Error) << "SaveSTL: BodyComponent or backend handle missing.";
            return;
        }

        std::string filename = "output.stl";
        if (auto s = std::get_if<std::string>(&namePin.value)) {
            if (!s->empty())
                filename = *s;
        }

        double deflection = 0.5;
        if (auto* v = std::get_if<double>(&node.inputs[0].value))
            deflection = *v;

        backend.ExportShapeToSTL(body->handle, filename, deflection);
        LOG(Info) << "SaveSTL: wrote file '" << filename << "'";
    };
    reg.RegisterNode("SINK_SAVE_STL", def);
}

void BoxPrimitive(of::domain::IGeometryBackend& backend) {
    auto& reg = NodeRegistry::Instance();

    NodeDefinition boxDef;
    boxDef.name = "Box Generator";
    boxDef.category = "Geometry";
    boxDef.inputs = {
        {"Width", PinType::DOUBLE, 1.0f}, {"Length", PinType::DOUBLE, 1.0f}, {"Height", PinType::DOUBLE, 1.0f}};
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

void RegisterGeometryNodes(of::domain::IGeometryBackend& backend) {
    BoxPrimitive(backend);
    ExportStl(backend);
}

}  // namespace of::nodes
