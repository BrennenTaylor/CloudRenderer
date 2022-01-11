add_library(D3D11SpatiotemporalFilter
    D3D11SpatiotemporalFilter.cpp
    D3D11SpatiotemporalFilter.h
    PathTracerCbs.h
    RenderPasses.h
    ShaderStructures.h
)

target_compile_options(D3D11SpatiotemporalFilter
    PRIVATE /std:c++17)

target_include_directories(D3D11SpatiotemporalFilter
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(D3D11SpatiotemporalFilter
    d3d11
    dxgi
    dxguid
    d3dcompiler
    DirectXTK
    D3D11Backend
    D3D11Utils
    GraphicsBackendLoader
)