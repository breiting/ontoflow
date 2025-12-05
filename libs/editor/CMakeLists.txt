add_library(ontoflow_editor STATIC
	src/Editor.cpp
	src/ViewController.cpp
)

target_include_directories(ontoflow_editor PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern
)

target_link_libraries(ontoflow_editor
PUBLIC
	project_settings
	ontoflow_core
	ontoflow_domain
	ontoflow_engine
	ontoflow_nodes
	ontoflow_command
	ontoflow_ui
)
