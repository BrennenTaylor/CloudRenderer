add_subdirectory(D3D11Backend)
add_subdirectory(D3D11Utils)

add_library(GraphicsBackendLoader

    Camera.h
    CameraEntry.h
    CameraManager.h
    DirectionalLightComponent.h
    GenericCbs.h
    Geometry.h
    IGraphicsBackend.h
    LightSet.h
    ObjMesh.h
    PointLightComponent.h
    RenderComponent.h
    Renderer.h
    TriMesh.h
    Vertex.h
    VisibleSet.h

    Camera.cpp
    CameraManager.cpp
    DirectionalLightComponent.cpp
    Geometry.cpp
    IGraphicsBackend.cpp
    LightSet.cpp
    ObjMesh.cpp
    PointLightComponent.cpp
    RenderComponent.cpp
    Renderer.cpp
    TriMesh.cpp
    Vertex.cpp
    VisibleSet.cpp
)

target_include_directories(GraphicsBackendLoader
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GraphicsBackendLoader
    Input
    Utils
    WindowFactory
    World
    FMath::FMath
    tinyobjloader
)