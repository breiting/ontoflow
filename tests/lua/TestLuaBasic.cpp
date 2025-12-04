#include <gtest/gtest.h>

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/lua/CadAPI.hpp>
#include <ontoflow/lua/LuaEngine.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>

using namespace of::domain;
using namespace of::occt;
using namespace of::lua;

TEST(LuaBasic, Simple) {
    Registry reg;
    OCCTBackend backend;
    GeometrySystem geom(reg, backend);

    CadAPI api(reg, geom);

    LuaEngine lua(api);
    std::string err;

    ASSERT_EQ(lua.Initialize(&err), true);
}
