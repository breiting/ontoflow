#pragma once
#include <ontoflow/domain/IGeometryBackend.hpp>

namespace of::nodes {

/**
 * \brief Registers geometry generation nodes. Needs backend access.
 */
void RegisterGeometryNodes(of::domain::IGeometryBackend& backend);

}  // namespace of::nodes
