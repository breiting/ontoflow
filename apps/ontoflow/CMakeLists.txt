add_executable(ontoflow
	src/main.cpp
)

target_link_libraries(ontoflow
PRIVATE
	glm::glm
	ontoflow_core
	ontoflow_domain
	ontoflow_occt
	ontoflow_command
	ontoflow_editor
	ontoflow_ui
	ontoflow_render
	CLI11::CLI11
)
