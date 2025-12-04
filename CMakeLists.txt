cmake_minimum_required(VERSION 3.31.1)
project(OntoFlow VERSION 0.1 LANGUAGES CXX)

# Produce json file for neovim clangd completion
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)
# Required for clang-uml to work properly on macOS
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Load compiler settings target
include(cmake/CompilerSettings.cmake)

# Load embed text functionality and create assets folder
include(cmake/EmbedText.cmake)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/assets")

# Dependencies via FetchContent
include(FetchContent)

# CLI11
FetchContent_Declare(
		CLI11
		GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
		GIT_TAG v2.4.2   # stable version
)
FetchContent_MakeAvailable(CLI11)

# ----------------------------------------------------------
# GLM
# ----------------------------------------------------------
FetchContent_Declare(
		glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# ----------------------------------------------------------
# GLFW
# ----------------------------------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
		glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# ----------------------------------------------------------
# GLAD
# ----------------------------------------------------------
add_library(glad STATIC extern/glad/src/gl.cpp)
target_include_directories(glad PUBLIC extern/glad/include)

# ----------------------------------------------------------
# Google Test
# ----------------------------------------------------------
FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
		DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# ----------------------------------------------------------
# Eigen
# ----------------------------------------------------------
FetchContent_Declare(
		eigen
		GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
		GIT_TAG 5.0.1
)
FetchContent_MakeAvailable(eigen)

# ----------------------------------------------------------
# sol2 (lua)
# ----------------------------------------------------------
FetchContent_Declare(
		sol2
		GIT_REPOSITORY https://github.com/ThePhD/sol2.git
		GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(sol2)

# ----------------------------------------------------------
# ImNodes
# ----------------------------------------------------------
FetchContent_Declare(
		imnodes
		GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
		GIT_TAG master
)
FetchContent_GetProperties(imnodes)
if(NOT imnodes_POPULATED)
	cmake_policy(SET CMP0169 OLD) # Allow FetchContent_Populate
	FetchContent_Populate(imnodes)
	add_library(imnodes STATIC
								${imnodes_SOURCE_DIR}/imnodes.cpp
				)
	target_include_directories(imnodes PUBLIC ${imnodes_SOURCE_DIR})
	target_link_libraries(imnodes PUBLIC imgui)
	target_compile_definitions(imnodes PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
endif()

# ----------------------------------------------------------
# ImGui
# ----------------------------------------------------------
FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# ----------------------------------------------------------
# Lua 5.4
# ----------------------------------------------------------
find_package(Lua 5.4 QUIET)
if(NOT Lua_FOUND)
	find_package(Lua 5.3 QUIET)
endif()
if(NOT Lua_FOUND)
	message(FATAL_ERROR "Lua 5.3+ not found. Install lua (e.g., apt install liblua5.4-dev) or set Lua_DIR.")
endif()
message(STATUS "Lua include: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libs: ${LUA_LIBRARIES}")

# -------------------------
# Shaders (embed to headers)
# -------------------------
set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
set(GEN_INC_DIR    "${CMAKE_BINARY_DIR}/generated")

embed_text_to_header("${SHADER_SRC_DIR}/flat.vert" "${GEN_INC_DIR}/assets/flat_vert.h" flat_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/flat.frag" "${GEN_INC_DIR}/assets/flat_frag.h" flat_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/grid.vert" "${GEN_INC_DIR}/assets/grid_vert.h" grid_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/grid.frag" "${GEN_INC_DIR}/assets/grid_frag.h" grid_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/lineset.vert" "${GEN_INC_DIR}/assets/lineset_vert.h" lineset_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/lineset.frag" "${GEN_INC_DIR}/assets/lineset_frag.h" lineset_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/pointset.vert" "${GEN_INC_DIR}/assets/pointset_vert.h" pointset_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/pointset.frag" "${GEN_INC_DIR}/assets/pointset_frag.h" pointset_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/unlit.vert" "${GEN_INC_DIR}/assets/unlit_vert.h" unlit_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/unlit.frag" "${GEN_INC_DIR}/assets/unlit_frag.h" unlit_frag_glsl)

add_subdirectory(libs)
add_subdirectory(apps/headless)
add_subdirectory(apps/ontoflow)

enable_testing()
add_subdirectory(tests)
