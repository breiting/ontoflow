add_library(ontoflow_nodes
    src/nodes/InputNodes.cpp
    src/nodes/MathNodes.cpp
    src/nodes/GeometryNodes.cpp
    src/nodes/StandardLibrary.cpp
)

target_include_directories(ontoflow_nodes PUBLIC inc)

target_link_libraries(ontoflow_nodes PUBLIC
    ontoflow_core
    ontoflow_domain
    ontoflow_engine
)