add_library(ontoflow_ui STATIC
	src/Window.cpp
	src/GraphEditorSystem.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


target_include_directories(ontoflow_ui
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${imnodes_SOURCE_DIR}
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends
)

target_link_libraries(ontoflow_ui
PUBLIC
	glfw
	glad
	glm::glm
	ontoflow_core
	ontoflow_domain
	ontoflow_command
	ontoflow_engine
	imgui
	imnodes
)
