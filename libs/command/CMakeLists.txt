add_library(ontoflow_command STATIC
	src/CommandStack.cpp
)

target_include_directories(ontoflow_command PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(ontoflow_command
PUBLIC
	glm::glm
	ontoflow_core
	ontoflow_domain
)
