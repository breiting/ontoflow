#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/InputNodes.hpp>
#include <variant>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterInputNodes() {
    auto& reg = NodeRegistry::Instance();

    // --- Float Value Node ---
    NodeDefinition valDef;
    valDef.name = "Value (Float)";
    valDef.category = "Input";
    valDef.outputs.push_back(Pin{"Out", PinType::FLOAT});
    valDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("FLOAT_VALUE", valDef);

    // --- Int Value Node ---
    NodeDefinition intDef;
    intDef.name = "Value (Int)";
    intDef.category = "Input";
    intDef.outputs.push_back(Pin{"Out", PinType::INT});
    intDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("INT_VALUE", intDef);
}

}  // namespace of::nodes
