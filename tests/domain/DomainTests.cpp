#include <gtest/gtest.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>

TEST(NodeSystem, PinValueAssignment) {
    of::domain::Pin inputPin;
    inputPin.type = of::domain::PinType::DOUBLE;
    inputPin.value = 42.0;

    ASSERT_TRUE(std::holds_alternative<double>(inputPin.value));
    EXPECT_DOUBLE_EQ(std::get<double>(inputPin.value), 42.0);
}

TEST(NodeSystem, NodeComponentStructure) {
    of::domain::NodeComponent node;
    node.definitionID = "MATH_ADD";

    of::domain::Pin p1;
    p1.name = "A";
    p1.type = of::domain::PinType::DOUBLE;
    of::domain::Pin p2;
    p2.name = "B";
    p2.type = of::domain::PinType::DOUBLE;

    node.inputs.push_back(p1);
    node.inputs.push_back(p2);

    EXPECT_EQ(node.inputs.size(), 2);
    EXPECT_EQ(node.inputs[0].name, "A");
}
