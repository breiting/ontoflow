#include <algorithm>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>

namespace of::domain {

std::vector<Entity> Registry::Entities() const {
    std::vector<Entity> result;

    for (const auto& kv : m_Storages) {
        // kv.second is unique_ptr<IComponentStorage>
        if (kv.second) {
            std::vector<Entity> ents = kv.second->GetEntities();
            result.insert(result.end(), ents.begin(), ents.end());
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

void Registry::Dump() const {
    LOG(Info) << "\n=== REGISTRY DUMP ===";

    for (Entity e : Entities()) {
        LOG(Info) << "Entity " << e;

        if (HasComponent<NameComponent>(e)) {
            auto* c = GetComponent<NameComponent>(e);
            LOG(Info) << "  Name        = " << c->name;
        }

        if (HasComponent<NodeComponent>(e)) {
            auto* c = GetComponent<NodeComponent>(e);
            LOG(Info) << "  Node        = " << c->definitionID << " inputs=" << c->inputs.size()
                      << " outputs=" << c->outputs.size();
        }

        if (HasComponent<MeshComponent>(e)) {
            auto* c = GetComponent<MeshComponent>(e);
            LOG(Info) << "  Mesh        = vtx=" << c->mesh.vertices.size() << ", idx=" << c->mesh.indices.size();
        }

        if (HasComponent<BodyComponent>(e)) {
            LOG(Info) << "  Body        = [OCCT handle present]";
        }
    }

    LOG(Info) << "=== END REGISTRY ===\n";
}

}  // namespace of::domain
