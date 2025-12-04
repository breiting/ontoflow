#pragma once

#include <functional>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/NodeData.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace of::engine {

/// Function signature for node logic execution
using NodeComputeFn = std::function<void(domain::NodeComponent& node, domain::Registry& reg)>;

struct NodeDefinition {
    std::string name;      ///< Display Name
    std::string category;  ///< Palette Category
    std::vector<domain::Pin> inputs;
    std::vector<domain::Pin> outputs;
    NodeComputeFn compute;  ///< The logic lambda
};

/**
 * @brief Singleton Registry for all available Node Types.
 * Stores "Prototypes" (NodeDefinitions) and handles spawning logic.
 */
class NodeRegistry {
   public:
    static NodeRegistry& Instance();

    // Delete copy/move
    NodeRegistry(const NodeRegistry&) = delete;
    NodeRegistry& operator=(const NodeRegistry&) = delete;

    /**
     * @brief Registers a new node type.
     * @param opID Unique string ID (e.g., "MATH_ADD").
     * @param def The definition struct.
     */
    void RegisterNode(const std::string& opID, const NodeDefinition& def);

    /**
     * @brief Spawns a node entity in the ECS registry.
     * @param registry The ECS registry.
     * @param opID The operation ID to spawn.
     * @return The EntityID of the new node.
     */
    domain::Entity SpawnNode(domain::Registry& registry, const std::string& opID);

    /**
     * @brief Retrieves a node definition.
     */
    const NodeDefinition* GetDefinition(const std::string& opID) const;

   private:
    NodeRegistry() = default;
    std::unordered_map<std::string, NodeDefinition> m_Definitions;
};

}  // namespace of::engine
