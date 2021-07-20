set (Sources
    D3D11GpuProfiler.cpp
)

set (Includes
    D3D11GpuProfiler.h
    DebugUtils.h
)

add_library(D3D11Utils STATIC
    ${Sources}
    ${Includes})

target_include_directories(D3D11Utils
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(D3D11Utils
    d3d11
    dxgi
    d3dcompiler)