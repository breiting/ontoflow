#include <ontoflow/lua/CadAPI.hpp>

namespace of::lua {

CadAPI::CadAPI(domain::Registry& reg, domain::GeometrySystem& geom) 
    : m_Registry(reg), m_GeometrySystem(geom) {
}

}  // namespace of::lua