#include <ontoflow/core/Logger.hpp>
#include <ontoflow/engine/GraphEvaluator.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>

using namespace of::domain;

namespace of::engine {

GraphEvaluator::GraphEvaluator(Registry& registry) : m_Registry(registry) {
}

void GraphEvaluator::Evaluate(EntityID nodeID) {
    if (!m_Registry.HasComponent<NodeComponent>(nodeID))
        return;

    auto* node = m_Registry.GetComponent<NodeComponent>(nodeID);

    bool inputsChanged = PullInputs(*node);

    if (inputsChanged || node->isDirty) {
        const auto* def = NodeRegistry::Instance().GetDefinition(node->definitionID);
        if (def && def->compute) {
            LOG(Info) << "Computing Node: " << node->definitionID;  // Debug
            def->compute(*node, m_Registry);
        }

        node->isDirty = false;
    }
}

bool GraphEvaluator::PullInputs(NodeComponent& node) {
    bool anyInputChanged = false;

    for (auto& inputPin : node.inputs) {
        if (inputPin.connection.targetNodeID == INVALID_ENTITY_ID)
            continue;

        Evaluate(inputPin.connection.targetNodeID);

        auto* sourceNode = m_Registry.GetComponent<NodeComponent>(inputPin.connection.targetNodeID);
        if (!sourceNode)
            continue;

        size_t outIdx = inputPin.connection.targetPinIdx;
        if (outIdx >= sourceNode->outputs.size()) {
            LOG(Error) << "GraphEvaluator: Invalid Output Pin Index on Source Node";
            continue;
        }

        const auto& incomingValue = sourceNode->outputs[outIdx].value;
        if (inputPin.value != incomingValue) {
            inputPin.value = incomingValue;
            anyInputChanged = true;
        }
    }

    return anyInputChanged;
}

}  // namespace of::engine
