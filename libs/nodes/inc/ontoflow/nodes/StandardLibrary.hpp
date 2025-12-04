#pragma once
#include <ontoflow/domain/IGeometryBackend.hpp>

namespace of::nodes {

/**
 * \brief Facade class to register all available node libraries at once.
 */
class StandardLibrary {
   public:
    static void RegisterAll(of::domain::IGeometryBackend& backend);
};

}  // namespace of::nodes
