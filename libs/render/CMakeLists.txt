add_library(ontoflow_render STATIC
	src/AxisRenderer.cpp
	src/BaseGeometry.cpp
	src/Camera2D.cpp
	src/Camera3D.cpp
	src/DirectionalLight.cpp
	src/FlatShadedMaterial.cpp
	src/FontLoaderSTB.cpp
	src/FontSystem.cpp
	src/GlyphAtlas.cpp
	src/LineSet.cpp
	src/LineSetMaterial.cpp
	src/Mesh.cpp
	src/PointSet.cpp
	src/PointSetMaterial.cpp
	src/OpenGLRenderer.cpp
	src/RenderingSystem.cpp
	src/Shader.cpp
	src/StlReader.cpp
	src/TextRenderer2D.cpp
	src/Texture.cpp
)

target_include_directories(ontoflow_render
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../../extern
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/stb
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/glad/include
	${GEN_INC_DIR}
)

target_link_libraries(ontoflow_render
PUBLIC
	project_settings
	glad
	glm::glm
	ontoflow_core
	ontoflow_editor
	ontoflow_domain
)
