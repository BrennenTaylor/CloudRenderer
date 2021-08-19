add_library(World
    AreaLight.h
    AreaLight.cpp
    Mesh.h
    Mesh.cpp
    SceneGraph.h
    SceneGraph.cpp
    Transform.h
    Transform.cpp
    World.h
    World.cpp
)

target_include_directories(World
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(World
    FMath::FMath
)

add_library(Farlor::World ALIAS World)