#pragma once
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/IGeometryBackend.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/domain/Types.hpp>

namespace of::domain {

class GeometrySystem {
   public:
    GeometrySystem(Registry& registry, IGeometryBackend& backend);

    IGeometryBackend& GetBackend() {
        return m_Backend;
    }

    /// \brief Scans all BodyComponents, generates Meshes via backend, and updates MeshComponents.
    void TesselateShapes();

   private:
    Registry& m_Registry;
    IGeometryBackend& m_Backend;
};

}  // namespace of::domain
