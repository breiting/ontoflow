#include <gtest/gtest.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/engine/GraphEvaluator.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <variant>

using namespace of::domain;
using namespace of::engine;

class EngineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Register Value Node
        NodeDefinition valueNode;
        valueNode.name = "Value";
        valueNode.category = "Math";
        valueNode.outputs.push_back(Pin{"Out", PinType::DOUBLE, std::monostate{}, {}});
        valueNode.compute = [](NodeComponent&, Registry&) {
            // Passthrough
        };
        NodeRegistry::Instance().RegisterNode("MATH_VALUE", valueNode);

        // Register Add Node
        NodeDefinition addNode;
        addNode.name = "Add";
        addNode.category = "Math";
        addNode.inputs.push_back(Pin{"A", PinType::DOUBLE, std::monostate{}, {}});
        addNode.inputs.push_back(Pin{"B", PinType::DOUBLE, std::monostate{}, {}});
        addNode.outputs.push_back(Pin{"Result", PinType::DOUBLE, std::monostate{}, {}});
        addNode.compute = [](NodeComponent& node, Registry&) {
            double a = 0.0;
            double b = 0.0;

            if (std::holds_alternative<double>(node.inputs[0].value))
                a = std::get<double>(node.inputs[0].value);

            if (std::holds_alternative<double>(node.inputs[1].value))
                b = std::get<double>(node.inputs[1].value);

            node.outputs[0].value = a + b;
        };
        NodeRegistry::Instance().RegisterNode("MATH_ADD", addNode);
    }
};

TEST_F(EngineTest, MathAdditionFlow) {
    Registry reg;
    GraphEvaluator evaluator(reg);

    // 1. Build Graph
    Entity n1 = NodeRegistry::Instance().SpawnNode(reg, "MATH_VALUE");
    Entity n2 = NodeRegistry::Instance().SpawnNode(reg, "MATH_VALUE");
    Entity nAdd = NodeRegistry::Instance().SpawnNode(reg, "MATH_ADD");

    // Set static values
    auto* c1 = reg.GetComponent<NodeComponent>(n1);
    c1->outputs[0].value = 10.0;

    auto* c2 = reg.GetComponent<NodeComponent>(n2);
    c2->outputs[0].value = 32.0;

    // Connect
    auto* cAdd = reg.GetComponent<NodeComponent>(nAdd);
    cAdd->inputs[0].connection = {n1, 0};
    cAdd->inputs[1].connection = {n2, 0};

    // 2. Execute
    evaluator.Evaluate(nAdd);

    // 3. Assert
    ASSERT_TRUE(std::holds_alternative<double>(cAdd->outputs[0].value));
    EXPECT_DOUBLE_EQ(std::get<double>(cAdd->outputs[0].value), 42.0);
}
