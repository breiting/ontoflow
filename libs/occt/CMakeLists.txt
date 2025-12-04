add_library(ontoflow_occt STATIC
	src/OCCTBackend.cpp
	src/STEPImporter.cpp
)

target_include_directories(ontoflow_occt PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/../domain/inc
)

# ----- OPEN CASCADE HANDLING -----
find_package(OpenCASCADE REQUIRED)

set(OCCT_WANTED
	TKernel TKMath TKG2d TKG3d
	TKGeomBase TKGeomAlgo
	TKBRep TKTopAlgo TKPrim TKBO TKShHealing
	TKMesh TKXMesh TKService
	TKSTL
	TKXDESTEP
)

set(OCCT_LIBS "")
foreach(name IN LISTS OCCT_WANTED)
	if(TARGET OpenCASCADE::${name})
		list(APPEND OCCT_LIBS OpenCASCADE::${name})
	endif()
endforeach()

# Fallback (macOS)
if(OCCT_LIBS STREQUAL "")
	set(OCCT_LIBS ${OpenCASCADE_LIBRARIES})
	list(FILTER OCCT_LIBS EXCLUDE REGEX "Draw|DCAF|XSDRAW|TopTest|ViewerTest|QADraw|TObjDRAW|XDEDRAW|TKTObj")
endif()

message(STATUS "OpenCASCADE: USING ${OpenCASCADE_VERSION}")

target_link_libraries(ontoflow_occt PRIVATE ${OCCT_LIBS})
target_include_directories(ontoflow_occt PUBLIC ${OpenCASCADE_INCLUDE_DIR})

target_link_libraries(ontoflow_occt
PUBLIC
	ontoflow_core
	ontoflow_domain
)
