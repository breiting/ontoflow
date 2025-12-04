add_library(ontoflow_lua STATIC
	src/BindConstruct.cpp
	src/BindIO.cpp
	src/BindPrimitives.cpp
	src/CadAPI.cpp
	src/LuaEngine.cpp
	src/PrettyLuaError.cpp
)

target_include_directories(ontoflow_lua PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${LUA_INCLUDE_DIR}
	${sol2_SOURCE_DIR}/include
)

target_link_libraries(ontoflow_lua
PUBLIC
	ontoflow_core
	ontoflow_domain
	glm::glm
	${LUA_LIBRARIES}
	sol2
)
