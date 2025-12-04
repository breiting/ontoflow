#include <ontoflow/nodes/GeometryNodes.hpp>
#include <ontoflow/nodes/InputNodes.hpp>
#include <ontoflow/nodes/MathNodes.hpp>
#include <ontoflow/nodes/StandardLibrary.hpp>

namespace of::nodes {

void StandardLibrary::RegisterAll(of::domain::IGeometryBackend& backend) {
    RegisterInputNodes();
    RegisterMathNodes();
    RegisterGeometryNodes(backend);
    // Future: RegisterLogicNodes(), RegisterAgentNodes()...
}

}  // namespace of::nodes
