#pragma once
#include <ontoflow/lua/CadAPI.hpp>
#include <sol/sol.hpp>

namespace of::lua {

void BindPrimitives(sol::table& cad, CadAPI& api);
void BindConstruct(sol::table& cad, CadAPI& api);
void BindIO(sol::table& cad, CadAPI& api);
}  // namespace of::lua
