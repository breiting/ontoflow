add_library(ontoflow_engine STATIC
	src/NodeRegistry.cpp
	src/GraphEvaluator.cpp
	src/GraphSerializer.cpp
)

target_include_directories(ontoflow_engine PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(ontoflow_engine
PUBLIC
	ontoflow_core
	ontoflow_domain
	nlohmann_json::nlohmann_json
)
