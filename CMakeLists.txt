cmake_minimum_required(VERSION 3.11)

project (CloudRenderer)

find_package(Vulkan REQUIRED FATAL_ERROR)

if (DEFINED VCPKG_TOOLCHAIN)
    set (VCPKG_TOOLCHAIN_SAVED ${VCPKG_TOOLCHAIN})
    unset(VCPKG_TOOLCHAIN)
endif()

add_subdirectory(DirectXMath)
add_subdirectory(DirectXTK)

if (DEFINED VCPKG_TOOLCHAIN_SAVED)
    set (VCPKG_TOOLCHAIN ${VCPKG_TOOLCHAIN_SAVED})
    unset(VCPKG_TOOLCHAIN_SAVED)
endif()

add_subdirectory(FMath)
add_subdirectory(tinyobjloader)
add_subdirectory(tinyxml2)

add_subdirectory(Farlor)

add_subdirectory(D3D11Renderer)

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
    PRIVATE Vulkan::Vulkan
)

target_compile_options(CloudRenderer
    PRIVATE /std:c++17
)