add_executable(test_ontoflow
	command/TestCommandStack.cpp
	core/TestLogger.cpp
	domain/DomainTests.cpp
	domain/OntoFlowDomainTests.cpp
	domain/TestRegistry.cpp
	engine/TestEngine.cpp
	lua/TestLuaBasic.cpp
	occt/TestOCCTBackend.cpp
)

target_include_directories(test_ontoflow PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../inc
)

target_link_libraries(test_ontoflow PRIVATE
	GTest::gtest_main
	project_settings
	ontoflow_core
	ontoflow_domain
	ontoflow_engine
	ontoflow_lua
	ontoflow_occt
	ontoflow_editor
	ontoflow_command
	ontoflow_render
)

add_test(
	NAME test_ontoflow
	COMMAND $<TARGET_FILE:test_ontoflow>
)
