add_library(D3D11Backend

    D3D11Backend.cpp
    D3D11Geometry.cpp
    D3D11ResourceFactory.cpp

    CBStructures.h
    D3D11Backend.h
    D3D11Geometry.h
    D3D11ResourceFactory.h
)

target_compile_options(D3D11Backend
    PRIVATE /std:c++17)

target_link_libraries(D3D11Backend
    d3d11
    dxgi
    d3dcompiler
    D3D11Utils
    GraphicsBackendLoader
    WindowFactory
)