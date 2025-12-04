#include <gtest/gtest.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <variant>

using namespace of::domain;

TEST(OntoFlowDomain, PinVariantTest) {
    Pin p;
    p.type = PinType::FLOAT;
    p.value = 3.14;

    ASSERT_TRUE(std::holds_alternative<double>(p.value));
    EXPECT_DOUBLE_EQ(std::get<double>(p.value), 3.14);
}

TEST(OntoFlowDomain, NodeComponentTest) {
    NodeComponent node;
    node.definitionID = "TEST_OP";

    node.inputs.push_back(Pin{"In1", of::domain::PinType::FLOAT, 0.0f, {}});
    node.inputs.push_back(Pin{"In2", of::domain::PinType::FLOAT, 0.0f, {}});

    EXPECT_EQ(node.inputs.size(), 2);
}

TEST(OntoFlowDomain, BodyComponentTest) {
    BodyComponent body;
    // Just verifying it compiles and exists
    EXPECT_EQ(body.handle, 0);
}
