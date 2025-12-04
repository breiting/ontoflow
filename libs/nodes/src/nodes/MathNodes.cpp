#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/MathNodes.hpp>
#include <variant>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterMathNodes() {
    auto& reg = NodeRegistry::Instance();

    // --- Add Node ---
    NodeDefinition addDef;
    addDef.name = "Add";
    addDef.category = "Math";
    addDef.inputs.push_back(Pin{"A", PinType::FLOAT});
    addDef.inputs.push_back(Pin{"B", PinType::FLOAT});
    addDef.outputs.push_back(Pin{"Result", PinType::FLOAT});

    addDef.compute = [](NodeComponent& node, Registry&) {
        double a = 0.0;
        double b = 0.0;

        if (auto* v = std::get_if<double>(&node.inputs[0].value))
            a = *v;
        if (auto* v = std::get_if<double>(&node.inputs[1].value))
            b = *v;

        node.outputs[0].value = a + b;
    };
    reg.RegisterNode("MATH_ADD", addDef);
}

}  // namespace of::nodes
