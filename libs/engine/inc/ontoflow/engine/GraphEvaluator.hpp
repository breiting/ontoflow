#pragma once
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>

namespace of::engine {

class GraphEvaluator {
   public:
    explicit GraphEvaluator(domain::Registry& registry);

    void Evaluate(domain::EntityID nodeID);

   private:
    bool PullInputs(domain::NodeComponent& node);

   private:
    domain::Registry& m_Registry;
};

}  // namespace of::engine
