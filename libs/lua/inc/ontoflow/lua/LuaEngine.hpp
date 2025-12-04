#pragma once
#include <ontoflow/lua/CadAPI.hpp>
#include <sol/sol.hpp>

namespace of::lua {

class LuaEngine {
   public:
    LuaEngine(CadAPI& api);
    bool Initialize(std::string* errorMsg = nullptr);

    bool RunFile(const std::string& scriptPath);
    bool RunString(const std::string& script);
    void Reset();

    sol::state& GetState() {
        return m_Lua;
    }

   private:
    void PrintRegisteredFunctions();
    void PrintTable(const std::string& tableName);

   private:
    sol::state m_Lua;
    bool m_Initialized = false;
    CadAPI& m_API;
};

}  // namespace of::lua
