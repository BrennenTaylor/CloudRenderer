cmake_minimum_required(VERSION 3.19)

project (CloudRenderer)

include(FetchContent)

# In this example we are picking a specific tag.
# You can also pick a specific commit, if you need to.
FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v3.1.0"
)
FetchContent_MakeAvailable(GSL)

# In this example we are picking a specific tag.
# You can also pick a specific commit, if you need to.
FetchContent_Declare(DirectXMath
    GIT_REPOSITORY "https://github.com/microsoft/DirectXMath"
    GIT_TAG "jan2021"
)
FetchContent_MakeAvailable(DirectXMath)

FetchContent_Declare(DirectXTK
    GIT_REPOSITORY "https://github.com/microsoft/DirectXTK"
    GIT_TAG "nov2021b"
)
FetchContent_MakeAvailable(DirectXTK)

FetchContent_Declare(FMath
    GIT_REPOSITORY "https://github.com/BrennenTaylor/FMath"
)
FetchContent_MakeAvailable(FMath)

FetchContent_Declare(tinyxml2
    GIT_REPOSITORY "https://github.com/leethomason/tinyxml2"
)
FetchContent_MakeAvailable(tinyxml2)

FetchContent_Declare(tinyobjloader
    GIT_REPOSITORY "https://github.com/tinyobjloader/tinyobjloader"
)
FetchContent_MakeAvailable(tinyobjloader)

find_package(Vulkan REQUIRED FATAL_ERROR)

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
    PUBLIC FarlorEngine
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

add_subdirectory(tools)