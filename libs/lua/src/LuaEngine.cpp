#include <iostream>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/lua/Bindings.hpp>
#include <ontoflow/lua/LuaEngine.hpp>
#include <ontoflow/lua/PrettyLuaError.hpp>

namespace of::lua {

LuaEngine::LuaEngine(CadAPI& api) : m_API(api) {
}

bool LuaEngine::Initialize(std::string* err) {
    if (m_Initialized)
        return true;

    try {
        m_Lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::package, sol::lib::string,
                             sol::lib::io);

        sol::table cad = m_Lua.create_named_table("cad");

        BindPrimitives(cad, m_API);
        BindConstruct(cad, m_API);
        BindIO(cad, m_API);

        // The following debug functions should ideally be LOG(Debug) and not always active.
        // PrintRegisteredFunctions();
        // PrintTable("cad");

        m_Initialized = true;
        return true;
    } catch (const std::exception& e) {
        if (err)
            *err = e.what();
        LOG(Error) << "LuaEngine initialization failed: " << e.what();
        return false;
    }
}

void LuaEngine::PrintRegisteredFunctions() {
    LOG(Info) << "[Lua Registered Functions]";
    sol::table globals = m_Lua.globals();

    for (auto& kv : globals) {
        sol::object key = kv.first;
        sol::object value = kv.second;

        if (value.get_type() == sol::type::function) {
            LOG(Info) << "  • " << key.as<std::string>();
        }
    }
}

void LuaEngine::PrintTable(const std::string& tableName) {
    sol::table tbl = m_Lua[tableName];
    if (!tbl.valid()) {
        LOG(Warn) << "Table '" << tableName << "' not found.";
        return;
    }
    LOG(Info) << "Functions in table '" << tableName << "':";
    for (auto& kv : tbl) {
        sol::object key = kv.first;
        sol::object value = kv.second;
        if (value.get_type() == sol::type::function) {
            LOG(Info) << "  • " << key.as<std::string>();
        }
    }
}

bool LuaEngine::RunFile(const std::string& scriptPath) {
    if (!m_Initialized) {
        LOG(Error) << "CoreEngine is not initialized";
        return false;
    }

    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();

    sol::load_result chunk = m_Lua.load_file(scriptPath);
    if (!chunk.valid()) {
        sol::error err = chunk;
        LOG(Error) << "Lua load error: " << err.what();
        return false;
    }

    sol::protected_function_result result = chunk();
    if (!result.valid()) {
        auto pretty = FormatLuaError(m_Lua, result);
        PrintLuaErrorPretty(pretty);
        return false;
    }
    auto end = clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::filesystem::path partFile{scriptPath};
    LOG(Info) << "[LuaEngine] " << partFile.filename() << " processing time: " << ms << "ms";
    return true;
}

bool LuaEngine::RunString(const std::string& script) {
    if (!m_Initialized) {
        LOG(Error) << "CoreEngine is not initialized";
        return false;
    }
    sol::load_result chunk = m_Lua.load(script.c_str());
    if (!chunk.valid()) {
        sol::error err = chunk;
        LOG(Error) << "Lua load error: " << err.what();
        return false;
    }

    sol::protected_function_result result = chunk();
    if (!result.valid()) {
        auto pretty = FormatLuaError(m_Lua, result);
        PrintLuaErrorPretty(pretty);
        return false;
    }
    return true;
}

void LuaEngine::Reset() {
    // Recreate state & bindings
    m_Lua = sol::state{};
    m_Initialized = false;
    // Re-init with previous config
    std::string err;
    if (!Initialize(&err)) {
        LOG(Error) << "LuaEngine::Reset() init failed: " << err;
    }
}

}  // namespace of::lua
