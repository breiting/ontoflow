#include <ontoflow/core/Logger.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>

namespace of::engine {

NodeRegistry& NodeRegistry::Instance() {
    static NodeRegistry instance;
    return instance;
}

void NodeRegistry::RegisterNode(const std::string& opID, const NodeDefinition& def) {
    if (m_Definitions.find(opID) != m_Definitions.end()) {
        LOG(Warn) << "NodeRegistry: Overwriting definition for " << opID;
    }
    m_Definitions[opID] = def;
    LOG(Info) << "NodeRegistry: Registered " << opID;
}

domain::Entity NodeRegistry::SpawnNode(domain::Registry& registry, const std::string& opID, const std::string& name,
                                       const glm::vec2& ui) {
    auto it = m_Definitions.find(opID);
    if (it == m_Definitions.end()) {
        LOG(Error) << "NodeRegistry: Cannot spawn unknown node type " << opID;
        return domain::INVALID_ENTITY_ID;
    }

    const auto& def = it->second;
    domain::Entity e = registry.CreateEntity();

    domain::NodeComponent nodeComp;
    nodeComp.definitionID = opID;
    nodeComp.inputs = def.inputs;
    nodeComp.outputs = def.outputs;
    nodeComp.isDirty = true;
    nodeComp.ui = ui;

    auto componentName = name;
    if (componentName.empty()) {
        componentName = def.name;
    }

    registry.AddComponent<domain::NodeComponent>(e, nodeComp);
    registry.AddComponent<domain::NameComponent>(e, domain::NameComponent{componentName});

    return e;
}

const NodeDefinition* NodeRegistry::GetDefinition(const std::string& opID) const {
    auto it = m_Definitions.find(opID);
    if (it != m_Definitions.end()) {
        return &it->second;
    }
    return nullptr;
}

}  // namespace of::engine
