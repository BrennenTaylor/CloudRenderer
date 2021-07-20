cmake_minimum_required(VERSION 3.11)

project (CloudRenderer)

add_subdirectory(DirectXTK)
add_subdirectory(FMath)
add_subdirectory(tinyobjloader)
add_subdirectory(tinyxml2)

add_subdirectory(Farlor)

add_subdirectory(D3D11SpatiotemporalFilter)

add_executable(CloudRenderer
    Main.cpp

    Core/Game.cpp
    Core/FixedUpdate.cpp
    Core/Threading/JobSystem.cpp

    Core/Mesh.cpp
    Core/Mesh.h
    Core/Scene.cpp
    Core/Scene.h

    Utils/TinyXmlUtil.cpp
)

target_include_directories(CloudRenderer
    PUBLIC ${GLM_INCLUDE_DIRS}
)

target_link_libraries(CloudRenderer
    PUBLIC FMath::FMath
    PUBLIC Farlor::Input
    PUBLIC Farlor::Utils
    PUBLIC Farlor::WindowFactory
    PUBLIC Farlor::World
    PUBLIC D3D11SpatiotemporalFilter
    PUBLIC tinyxml2::tinyxml2
    PUBLIC tinyobjloader
)

target_compile_options(CloudRenderer
    PRIVATE /std:c++17
)