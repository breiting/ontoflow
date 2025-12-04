add_executable(headless
	src/main.cpp
)

target_link_libraries(headless
PRIVATE
	glm::glm
	ontoflow_core
	ontoflow_domain
	ontoflow_engine
	ontoflow_occt
	ontoflow_command
)
