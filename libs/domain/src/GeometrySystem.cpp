#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>

#include "ontoflow/domain/Components.hpp"
#include "ontoflow/domain/Entity.hpp"

namespace of::domain {

GeometrySystem::GeometrySystem(Registry& reg, IGeometryBackend& backend) : m_Registry(reg), m_Backend(backend) {
}

void GeometrySystem::TesselateShapes() {
    auto entities = m_Registry.Entities();
    for (Entity e : entities) {
        if (m_Registry.HasComponent<BodyComponent>(e)) {
            auto* body = m_Registry.GetComponent<BodyComponent>(e);
            if (body->handle > 0) {
                Mesh meshData = m_Backend.GetMeshFromShape(body->handle);

                if (m_Registry.HasComponent<MeshComponent>(e)) {
                    auto* mc = m_Registry.GetComponent<MeshComponent>(e);
                    mc->mesh = std::move(meshData);
                    mc->version++;
                } else {
                    MeshComponent mc;
                    mc.mesh = std::move(meshData);
                    mc.version = 1;
                    m_Registry.AddComponent<MeshComponent>(e, mc);
                }
            }
        }
    }
}

}  // namespace of::domain
