#include <gtest/gtest.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>

using namespace of::domain;

// Define a Test Component
struct TestComponent {
    int value;
};

class TestRegistry : public ::testing::Test {
   protected:
    Registry registry;
};

TEST_F(TestRegistry, CreateEntity) {
    Entity e1 = registry.CreateEntity();
    Entity e2 = registry.CreateEntity();
    EXPECT_NE(e1, e2);
    EXPECT_NE(e1, INVALID_ENTITY);
}

TEST_F(TestRegistry, AddGetComponent) {
    Entity e = registry.CreateEntity();
    TestComponent comp{42};

    registry.AddComponent(e, comp);

    EXPECT_TRUE(registry.HasComponent<TestComponent>(e));

    TestComponent* retrieved = registry.GetComponent<TestComponent>(e);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->value, 42);
}

TEST_F(TestRegistry, RemoveComponent) {
    Entity e = registry.CreateEntity();
    registry.AddComponent<TestComponent>(e, {100});

    EXPECT_TRUE(registry.HasComponent<TestComponent>(e));

    registry.RemoveComponent<TestComponent>(e);

    EXPECT_FALSE(registry.HasComponent<TestComponent>(e));
    EXPECT_EQ(registry.GetComponent<TestComponent>(e), nullptr);
}

TEST_F(TestRegistry, Callbacks) {
    Entity e = registry.CreateEntity();
    bool added = false;
    bool removed = false;

    registry.OnComponentAdded<TestComponent>([&](Entity entity) {
        if (entity == e)
            added = true;
    });

    registry.OnComponentRemoved<TestComponent>([&](Entity entity) {
        if (entity == e)
            removed = true;
    });

    registry.AddComponent<TestComponent>(e, {10});
    EXPECT_TRUE(added);
    EXPECT_FALSE(removed);

    registry.RemoveComponent<TestComponent>(e);
    EXPECT_TRUE(removed);
}

TEST_F(TestRegistry, GetAllEntities) {
    Entity e1 = registry.CreateEntity();
    Entity e2 = registry.CreateEntity();

    registry.AddComponent<TestComponent>(e1, {1});
    registry.AddComponent<NameComponent>(e2, {"E2"});

    auto entities = registry.Entities();
    EXPECT_EQ(entities.size(), 2);

    // Entities list should be sorted and unique
    EXPECT_EQ(entities[0], std::min(e1, e2));
    EXPECT_EQ(entities[1], std::max(e1, e2));
}

TEST_F(TestRegistry, DumpDoesNotCrash) {
    Entity e = registry.CreateEntity();
    registry.AddComponent<NameComponent>(e, {"TestEntity"});
    registry.Dump();  // Should log to Info
}
