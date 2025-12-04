add_library(ontoflow_core STATIC
	src/Logger.cpp
)

target_include_directories(ontoflow_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(ontoflow_core
PUBLIC
	glm::glm
)

