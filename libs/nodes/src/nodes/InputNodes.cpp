#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/nodes/InputNodes.hpp>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterInputNodes() {
    auto& reg = NodeRegistry::Instance();

    // --- Float Value Node ---
    NodeDefinition valDef;
    valDef.name = "Value (Float)";
    valDef.category = "Input";
    valDef.outputs.push_back(Pin{"Out", PinType::DOUBLE});
    valDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("FLOAT_VALUE", valDef);

    // --- Int Value Node ---
    NodeDefinition intDef;
    intDef.name = "Value (Int)";
    intDef.category = "Input";
    intDef.outputs.push_back(Pin{"Out", PinType::INT});
    intDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("INT_VALUE", intDef);

    // --- String Value Node ---
    NodeDefinition strDef;
    strDef.name = "Value (String)";
    strDef.category = "Input";
    strDef.outputs.push_back(Pin{"Out", PinType::STRING});
    strDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("STRING_VALUE", strDef);
}

}  // namespace of::nodes
