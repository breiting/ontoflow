#pragma once

#include <functional>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <vector>

namespace of::domain {

// Type: (Entity, Registry) → bool
using QueryFilter = std::function<bool(Entity, const Registry&)>;

class Query {
   public:
    // Chainable filters
    Query& Where(QueryFilter filter) {
        m_Filters.push_back(std::move(filter));
        return *this;
    }

    /// Execute query, return matching entities
    std::vector<Entity> Execute(const Registry& registry) const {
        std::vector<Entity> result;

        // use new method from Registry: GetAllEntities()
        for (Entity e : registry.Entities()) {
            bool matches = true;
            for (const auto& filter : m_Filters) {
                if (!filter(e, registry)) {
                    matches = false;
                    break;
                }
            }
            if (matches)
                result.push_back(e);
        }
        return result;
    }

   private:
    std::vector<QueryFilter> m_Filters;
};

/// --------------------------------------------------
///  Convenience Helpers
/// --------------------------------------------------

template <typename T>
inline Query HasComponentQuery() {
    Query q;
    q.Where([](Entity e, const Registry& r) {
        return r.HasComponent<T>(e);
    });
    return q;
}

inline bool IsBody(Entity e, const Registry& r) {
    return r.HasComponent<BodyComponent>(e);
}

}  // namespace of::domain
